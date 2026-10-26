"""Independent check: tau(n) by MaxSAT (RC2), minimum hitting set of the conflicts with max <= n,
for the n given on the command line; prints n and a(n) = n - tau(n).
Conflicts are found by a different route: for each pair c < d, look up c^3 + d^3 in a dict of
all smaller pair sums."""
import sys
from pysat.examples.rc2 import RC2
from pysat.formula import WCNF

ns = [int(v) for v in sys.argv[1:]]
M = max(ns)
cubes = [i ** 3 for i in range(M + 1)]
first = {}  # sum -> list of pairs
edges = []
for d in range(2, M + 1):
    for c in range(1, d):
        s = cubes[c] + cubes[d]
        if s in first:
            for (a, b) in first[s]:
                edges.append(((a, b, c, d), d))
            first[s].append((c, d))
        else:
            first[s] = [(c, d)]
for n in ns:
    w = WCNF()
    vs = set()
    for e, mx in edges:
        if max(e) <= n:
            w.append(list(e))
            vs.update(e)
    for v in vs:
        w.append([-v], weight=1)
    with RC2(w) as r:
        r.compute()
        print(n, n - r.cost, flush=True)
