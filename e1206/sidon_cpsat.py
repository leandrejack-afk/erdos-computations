"""a(n) = size of a largest Sidon subset of {1^3, ..., n^3}, n = 1..N, with OR-tools CP-SAT.

Conflicts: 4-sets {a, b, c, d} with a^3 + b^3 = c^3 + d^3. tau(n) = minimum number of deletions
hitting every conflict with max <= n, a(n) = n - tau(n). tau(n-1) <= tau(n) <= tau(n-1) + 1, so
for each n that brings new conflicts we ask CP-SAT whether tau(n-1) deletions still suffice
(hinted with the previous optimal deletion set); if not, tau(n) = tau(n-1) + 1.
usage: sidon_cpsat.py N [workers] > a.txt  (prints "n a(n)"; the deletion sets go to stderr)
"""
import sys
from ortools.sat.python import cp_model

N = int(sys.argv[1]); W = int(sys.argv[2]) if len(sys.argv) > 2 else 4
cubes = [i ** 3 for i in range(N + 1)]
first = {}; edges_at = {}
for d in range(2, N + 1):
    for c in range(1, d):
        s = cubes[c] + cubes[d]
        if s in first:
            for (a, b) in first[s]:
                edges_at.setdefault(d, []).append((a, b, c, d))
            first[s].append((c, d))
        else:
            first[s] = [(c, d)]
del first
edges = []
tau = 0
best = set()  # an optimal deletion set for the current n
for n in range(1, N + 1):
    new = edges_at.get(n, [])
    if new:
        edges.extend(new)
        if not all(any(v in best for v in e) for e in edges):
            # does a deletion set of size tau exist?
            vs = sorted(set(v for e in edges for v in e))
            m = cp_model.CpModel()
            x = {v: m.NewBoolVar(f"x{v}") for v in vs}
            for e in edges:
                m.AddBoolOr([x[v] for v in e])
            m.Add(sum(x.values()) <= tau)
            for v in vs:
                m.AddHint(x[v], v in best)
            sol = cp_model.CpSolver()
            sol.parameters.num_workers = W
            st = sol.Solve(m)
            if st in (cp_model.OPTIMAL, cp_model.FEASIBLE):
                best = {v for v in vs if sol.Value(x[v])}
            elif st == cp_model.INFEASIBLE:
                tau += 1
                best = best | {n}  # deleting n hits every new conflict
            else:
                print(f"n={n}: solver status {sol.StatusName(st)}", file=sys.stderr); sys.exit(2)
    print(n, n - tau, flush=True)
    print(n, tau, sorted(best), file=sys.stderr, flush=True)
