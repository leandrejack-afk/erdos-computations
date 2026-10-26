"""c(n) = size of a largest Sidon subset of the first n cubes that contains n^3 (the variant an OEIS
editor asked about), by integer programming: the least number of removals from {1, ..., n - 1}
that meet every conflict {a, b, c, d} with a^3 + b^3 = c^3 + d^3 and max <= n, with n itself
fixed as kept; solved with HiGHS (scipy.optimize.milp, relative gap 0) at every n in [lo, hi].
Conflicts are found by sorting all pair sums, as in sidon_ilp.py.
usage: contain_ilp.py lo hi > c.txt  (prints "n c(n)"; the removal sets go to stderr)"""
import sys
import numpy as np
from scipy.optimize import milp, LinearConstraint, Bounds
from scipy.sparse import csr_matrix

lo, hi = int(sys.argv[1]), int(sys.argv[2])
a = np.arange(1, hi + 1, dtype=np.int64)
A, B = np.triu_indices(hi, k=1)
S = (a[A] ** 3 + a[B] ** 3)
order = np.argsort(S, kind="stable")
S, A, B = S[order], A[order] + 1, B[order] + 1
edges = []
i = 0
while i < len(S):
    j = i
    while j < len(S) and S[j] == S[i]:
        j += 1
    for p in range(i, j):
        for q in range(p + 1, j):
            edges.append((int(A[p]), int(B[p]), int(A[q]), int(B[q])))
    i = j
for n in range(lo, hi + 1):
    E = [e for e in edges if max(e) <= n]
    if not E:
        print(n, n, flush=True); print(n, 0, [], file=sys.stderr, flush=True); continue
    rows, cols = [], []
    for r, e in enumerate(E):
        for v in e:
            rows.append(r); cols.append(v - 1)
    M = csr_matrix((np.ones(len(rows)), (rows, cols)), shape=(len(E), n))
    ub = np.ones(n); ub[n - 1] = 0
    res = milp(c=np.ones(n), constraints=LinearConstraint(M, lb=1, ub=np.inf),
               integrality=np.ones(n), bounds=Bounds(0, ub), options={"mip_rel_gap": 0})
    if not res.success:
        print(n, "FAILED", res.message, flush=True); sys.exit(2)
    tau = int(round(res.fun))
    D = [v + 1 for v in range(n) if res.x[v] > 0.5]
    print(n, n - tau, flush=True)
    print(n, tau, D, file=sys.stderr, flush=True)
