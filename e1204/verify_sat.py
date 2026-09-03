#!/usr/bin/env python3
"""Independent check of S(k) for Erdős problem 1204 by weighted MaxSAT (python-sat, RC2).

Written from the definition only: it does not use the even-number reduction of smin.c, nor A008407,
nor any value produced by smin.c.

Variables x_n (0 <= n <= N): n belongs to the tuple.
Hard clauses: x_0 (translating a tuple down by a_1 keeps it admissible and lowers the sum, so
a_1 = 0 is optimal); exactly k of the x_n are true; for every prime p <= k at least one selector
y_{p,r} is true, and y_{p,r} forbids every x_n with n = r (mod p). Primes p > k cannot be covered
by k residues, so they need no clauses.
Soft clauses: not x_n with weight n, so the minimal cost is the minimal sum S(k).
Window: with G(k) the sum of the greedy admissible k-tuple (an upper bound on S(k)), a tuple with
sum <= G(k) has a_k <= G(k) - L(k-1) whenever its first k-1 elements sum to at least L(k-1).
  --trivial : L(k-1) = 0 + 1 + ... + (k-2), since the elements are distinct non-negative integers.
  default   : L(k-1) = S(k-1) as established by this same script at the previous k (the first k-1
              elements form an admissible (k-1)-tuple with a_1 = 0), so the run is inductive from k = 1.
By default one optimal tuple is reported; with --all the solver enumerates every optimal tuple in the
window (slow: after blocking a tuple RC2 must prove that no other tuple reaches the same cost).

Usage: verify_sat.py KMIN KMAX [--trivial] [--all]   (default window requires KMIN = 1)
Prints one line per k: k, S(k), window, optimal tuple(s).
"""
import sys
import time

from pysat.card import CardEnc, EncType
from pysat.examples.rc2 import RC2
from pysat.formula import WCNF


def primes_upto(n):
    return [p for p in range(2, n + 1) if all(p % d for d in range(2, int(p ** 0.5) + 1))]


def admissible(t):
    return all(len({x % p for x in t}) < p for p in primes_upto(len(t)))


def greedy(k):
    seq = [0]
    x = 0
    while len(seq) < k:
        x += 1
        if admissible(seq + [x]):
            seq.append(x)
    return seq


def solve(k, prev_S, enumerate_all):
    G = sum(greedy(k))
    lower_prefix = (k - 1) * (k - 2) // 2 if prev_S is None else prev_S
    N = G - lower_prefix
    xv = lambda n: n + 1
    top = N + 1
    wcnf = WCNF()
    wcnf.append([xv(0)])
    for p in primes_upto(k):
        selectors = []
        for r in range(p):
            top += 1
            selectors.append(top)
            for n in range(r, N + 1, p):
                wcnf.append([-top, -xv(n)])
        wcnf.append(selectors)
    card = CardEnc.equals(lits=[xv(n) for n in range(N + 1)], bound=k, top_id=top, encoding=EncType.seqcounter)
    wcnf.extend(card.clauses)
    for n in range(1, N + 1):
        wcnf.append([-xv(n)], weight=n)
    opt = None
    tuples = []
    with RC2(wcnf) as rc2:
        while True:
            model = rc2.compute()
            if model is None:
                break
            if opt is None:
                opt = rc2.cost
            if rc2.cost > opt:
                break
            t = [n for n in range(N + 1) if model[n] > 0]
            if not (len(t) == k and sum(t) == opt and admissible(t)):
                raise RuntimeError(f"solver returned a bad tuple for k={k}: {t}")
            tuples.append(t)
            if not enumerate_all:
                break
            # block this tuple (on the x variables only, so selector choices do not repeat it)
            rc2.add_clause([-xv(n) for n in t if n > 0])
    return opt, tuples, N, G


def main():
    kmin, kmax = int(sys.argv[1]), int(sys.argv[2])
    trivial = "--trivial" in sys.argv[3:]
    enumerate_all = "--all" in sys.argv[3:]
    if not trivial and kmin != 1:
        raise SystemExit("inductive window needs KMIN = 1 (or pass --trivial)")
    prev_S = None
    for k in range(kmin, kmax + 1):
        t0 = time.time()
        opt, tuples, N, G = solve(k, None if trivial else prev_S, enumerate_all)
        prev_S = opt
        print(f"k={k} S={opt} greedy={G} window={N} nopt={len(tuples) if enumerate_all else 'n/a'} secs={time.time() - t0:.1f}")
        for t in tuples:
            print("  tuple: " + " ".join(map(str, t)))
        sys.stdout.flush()


if __name__ == "__main__":
    main()
