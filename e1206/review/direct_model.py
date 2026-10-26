"""a(n) by a different formulation: maximize |A| directly with one constraint per sum value.

x_i = 1 iff i in A. For every pair i <= j (i = j included) whose sum i^3 + j^3 has two or more
representations, p_ij >= x_i + x_j - 1 (p_ii = x_i), and for every such sum value the p's of its
representations add up to at most 1 (the Sidon condition itself). Maximize sum x_i.
Backends: CP-SAT (1 worker) and HiGHS MIP through scipy (mip_rel_gap = 0). The optimal A is
re-checked against the definition. usage: direct_model.py cpsat|highs n1 n2 ..."""
import sys
import time
from collections import defaultdict

backend = sys.argv[1]
b = {int(l.split()[0]): int(l.split()[1]) for l in open(__file__.rsplit("/", 1)[0] + "/../b1206.txt")}


def is_sidon(A):
    seen = set()
    A = sorted(A)
    for i, x in enumerate(A):
        for y in A[i:]:
            s = x ** 3 + y ** 3
            if s in seen:
                return False
            seen.add(s)
    return True


for n in map(int, sys.argv[2:]):
    reps = defaultdict(list)
    for i in range(1, n + 1):
        for j in range(i, n + 1):
            reps[i ** 3 + j ** 3].append((i, j))
    groups = [r for r in reps.values() if len(r) >= 2]
    pairs = sorted(set(p for r in groups for p in r))
    t0 = time.time()
    if backend == "cpsat":
        from ortools.sat.python import cp_model
        m = cp_model.CpModel()
        x = {i: m.NewBoolVar(f"x{i}") for i in range(1, n + 1)}
        p = {}
        for (i, j) in pairs:
            if i == j:
                p[(i, j)] = x[i]
            else:
                p[(i, j)] = m.NewBoolVar(f"p{i}_{j}")
                m.Add(p[(i, j)] >= x[i] + x[j] - 1)
        for r in groups:
            m.Add(sum(p[q] for q in r) <= 1)
        m.Maximize(sum(x.values()))
        s = cp_model.CpSolver()
        s.parameters.num_workers = 1
        st = s.Solve(m)
        A = [i for i in range(1, n + 1) if s.Value(x[i])]
        status, val, bound = s.StatusName(st), int(s.ObjectiveValue()), s.BestObjectiveBound()
    else:
        import numpy as np
        from scipy.optimize import milp, LinearConstraint, Bounds
        from scipy.sparse import lil_matrix
        nx = n
        pid = {}
        for q in pairs:
            if q[0] != q[1]:
                pid[q] = nx; nx += 1
        col = lambda q: q[0] - 1 if q[0] == q[1] else pid[q]
        rows = len(pid) + len(groups)
        M = lil_matrix((rows, nx)); lb = np.full(rows, -np.inf); ub = np.full(rows, np.inf)
        r = 0
        for (i, j), c in pid.items():          # p_ij - x_i - x_j >= -1
            M[r, c] = 1; M[r, i - 1] = -1; M[r, j - 1] = -1; lb[r] = -1; r += 1
        for g in groups:                       # sum of p over the representations <= 1
            for q in g:
                M[r, col(q)] += 1
            ub[r] = 1; r += 1
        cost = np.zeros(nx); cost[:n] = -1
        res = milp(cost, constraints=LinearConstraint(M.tocsr(), lb, ub), integrality=np.ones(nx),
                   bounds=Bounds(0, 1), options={"mip_rel_gap": 0})
        A = [i for i in range(1, n + 1) if res.x[i - 1] > 0.5]
        status, val, bound = res.message.split(".")[0], int(round(-res.fun)), getattr(res, "mip_dual_bound", None)
        bound = -bound if bound is not None else None
    dt = time.time() - t0
    ok = is_sidon(A) and len(A) == val
    print(f"{backend} n={n} a={val} b-file={b[n]} {'AGREE' if val == b[n] else 'DIFFER'} status={status} "
          f"bound={bound} witness_sidon={ok} groups={len(groups)} time={dt:.1f}s", flush=True)
