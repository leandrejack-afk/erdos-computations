"""Independent check of a(n) (largest Sidon subset of the first n cubes) by integer programming:
tau(n) = min sum x_v subject to x_a + x_b + x_c + x_d >= 1 for every conflict with max <= n,
solved with HiGHS (scipy.optimize.milp, relative gap 0) at every n in [lo, hi]; prints "n a(n)".
Conflicts are found by sorting all pair sums (a third route, independent of the dict lookups)."""
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
edges.sort(key=max)
prev = None
for n in range(lo, hi + 1):
    E = [e for e in edges if max(e) <= n]
    if not E:
        print(n, n, flush=True); continue
    rows, cols = [], []
    for r, e in enumerate(E):
        for v in e:
            rows.append(r); cols.append(v - 1)
    M = csr_matrix((np.ones(len(rows)), (rows, cols)), shape=(len(E), n))
    res = milp(c=np.ones(n), constraints=LinearConstraint(M, lb=1, ub=np.inf),
               integrality=np.ones(n), bounds=Bounds(0, 1), options={"mip_rel_gap": 0})
    if not res.success:
        print(n, "FAILED", res.message, flush=True); sys.exit(2)
    tau = int(round(res.fun))
    print(n, n - tau, flush=True)
