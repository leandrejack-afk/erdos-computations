"""Certified lower bound tau(n) >= t: LP-based branch and bound whose every leaf is checked in
exact rational arithmetic, so HiGHS (used only to FIND the LP solutions) is not trusted.

Collisions C are found from ALL pair sums x <= y (x = y included). A node fixes some numbers as
removed (R) and some as kept (K). Alive collisions are those with no element in R; each must
still be broken by removing one of its free elements (not in R or K).
Leaf types:
  (a) an alive collision with no free element (nothing can break it), or
  (b) a fractional packing y >= 0 over alive collisions with sum_{C contains v} y_C <= 1 for every
      free v and sum y > budget = (t - 1) - |R|; by weak duality any set of free removals that
      breaks all alive collisions has size >= sum y > budget. y is rebuilt from the floating LP
      solution as exact Fractions and scaled down by the largest load, then checked exactly.
Internal nodes branch on a free number v: remove it (R + v) or keep it (K + v); collisions with
exactly one free element force that removal (the keep side is a type (a) leaf).
It also fixes a free number u as kept when some other free number v lies in every alive
collision that u lies in (any solution removing u stays a solution with v in place of u, so this
loses no solutions); this only shrinks the tree.
If the tree closes, no set of t - 1 removals leaves a Sidon set, so a(n) <= n - t.
usage: exact_bb.py n [t]   (t defaults to n - b(n), read from b1206.txt beside this file)"""
import os
import sys
import time
from collections import defaultdict
from fractions import Fraction
import numpy as np
import highspy

n = int(sys.argv[1])
b = {int(l.split()[0]): int(l.split()[1]) for l in open(os.path.join(os.path.dirname(os.path.abspath(__file__)), "b1206.txt"))}
t = int(sys.argv[2]) if len(sys.argv) > 2 else n - b[n]
reps = defaultdict(list)
for x in range(1, n + 1):
    for y in range(x, n + 1):
        reps[x ** 3 + y ** 3].append((x, y))
C = []
for r in reps.values():
    for i in range(len(r)):
        for j in range(i + 1, len(r)):
            C.append(tuple(sorted(set(r[i]) | set(r[j]))))
verts = sorted(set(v for c in C for v in c))
inc = defaultdict(list)
for k, c in enumerate(C):
    for v in c:
        inc[v].append(k)

# packing LP: max sum y_k, rows: for each vertex v, sum_{k in inc[v]} y_k <= 1
h = highspy.Highs()
h.setOptionValue("output_flag", False)
h.setOptionValue("threads", 1)
inf = highspy.kHighsInf
for k in range(len(C)):
    h.addVar(0, inf)
h.changeColsCost(len(C), np.arange(len(C), dtype=np.int32), -np.ones(len(C)))
row_of = {}
for v in verts:
    idx = np.array(inc[v], dtype=np.int32)
    h.addRow(-inf, 1.0, len(idx), idx, np.ones(len(idx)))
    row_of[v] = len(row_of)

stats = {"nodes": 0, "leaf_a": 0, "leaf_b": 0, "lp": 0, "maxdepth": 0}


def exact_packing_ok(yfloat, alive, free, budget):
    y = {k: max(Fraction(0), Fraction(yfloat[k]).limit_denominator(10 ** 6)) for k in alive}
    load = max([sum((y[k] for k in inc[v] if k in y), Fraction(0)) for v in free] + [Fraction(1)])
    total = sum(y.values(), Fraction(0)) / load
    return total > budget


def solve(R, K, depth):
    stats["nodes"] += 1
    stats["maxdepth"] = max(stats["maxdepth"], depth)
    budget = (t - 1) - len(R)
    # propagate: forced removals, then domination (u kept if another free v is in all of u's
    # alive collisions), repeated until nothing changes
    R, K = set(R), set(K)
    while True:
        alive = [k for k, c in enumerate(C) if not any(v in R for v in c)]
        fs = {k: [v for v in C[k] if v not in K] for k in alive}
        if any(not fs[k] for k in alive):
            stats["leaf_a"] += 1
            return True              # type (a) leaf: closed
        forced = {fs[k][0] for k in alive if len(fs[k]) == 1}
        if forced:
            R |= forced
            budget -= len(forced)
            continue
        occ = defaultdict(set)
        for k in alive:
            for v in fs[k]:
                occ[v].add(k)
        changed = False
        for u in sorted(occ, key=lambda w: (len(occ[w]), w)):
            ks = list(occ[u])
            cand = set(fs[ks[0]])
            for k in ks[1:]:
                cand &= set(fs[k])
            cand.discard(u)
            if cand:
                K.add(u)
                for k in ks:
                    fs[k].remove(u)
                changed = True
                if any(len(fs[k]) <= 1 for k in ks):
                    break
        if not changed:
            break
    if budget < 0:
        stats["leaf_a"] += 1         # forced removals exceed the budget (counting argument)
        return True
    if not alive:
        # a set of at most t - 1 removals breaks every collision: check it from the definition
        A = [x for x in range(1, n + 1) if x not in R]
        sums = set()
        for i, x in enumerate(A):
            for y in A[i:]:
                if x ** 3 + y ** 3 in sums:
                    raise SystemExit(f"BUG: OPEN witness at n={n} is not Sidon")
                sums.add(x ** 3 + y ** 3)
        print(f"OPEN witness: {len(R)} removals {sorted(R)}", flush=True)
        return False                 # a Sidon set within budget exists: tree does NOT close
    free = sorted(set(v for k in alive for v in C[k] if v not in K))
    # set up LP: dead collisions get y = 0, kept vertices lose their row constraint
    alive_set = set(alive)
    ub = np.array([inf if k in alive_set else 0.0 for k in range(len(C))])
    h.changeColsBounds(len(C), np.arange(len(C), dtype=np.int32), np.zeros(len(C)), ub)
    rup = np.array([inf if v in K else 1.0 for v in verts])
    h.changeRowsBounds(len(verts), np.arange(len(verts), dtype=np.int32), np.full(len(verts), -inf), rup)
    h.run()
    stats["lp"] += 1
    sol = h.getSolution()
    y = sol.col_value
    if exact_packing_ok(y, alive, free, budget):
        stats["leaf_b"] += 1
        return True
    # branch on the free vertex with the largest LP cover value (row dual), ties by degree
    duals = sol.row_dual
    deg = defaultdict(int)
    for k in alive:
        for v in C[k]:
            if v not in K:
                deg[v] += 1
    v = max(free, key=lambda u: (abs(duals[row_of[u]]), deg[u]))
    if not solve(R | {v}, K, depth + 1):
        return False
    return solve(R, K | {v}, depth + 1)


t0 = time.time()
closed = solve(frozenset(), frozenset(), 0)
print(f"n={n} t={t} collisions={len(C)}: {'CLOSED: no Sidon subset of size ' + str(n - t + 1) if closed else 'OPEN: found a Sidon subset with t-1 removals'} "
      f"nodes={stats['nodes']} leaves(a)={stats['leaf_a']} leaves(b)={stats['leaf_b']} lps={stats['lp']} "
      f"maxdepth={stats['maxdepth']} time={time.time() - t0:.1f}s", flush=True)
