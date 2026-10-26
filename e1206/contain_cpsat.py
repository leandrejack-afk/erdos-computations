"""Second route to c(n) = size of a largest Sidon subset of the first n cubes that contains n^3,
with OR-tools CP-SAT, each n from scratch: minimize the removals from {1, ..., n - 1} that meet
every conflict {a, b, c, d} (a^3 + b^3 = c^3 + d^3, max <= n), n fixed as kept. Conflicts are
found by dictionary lookup of pair sums, as in sidon_cpsat.py; no hints from other solvers.
usage: contain_cpsat.py N [workers] > c.txt  (prints "n c(n)")"""
import sys
from ortools.sat.python import cp_model

N = int(sys.argv[1]); W = int(sys.argv[2]) if len(sys.argv) > 2 else 8
cubes = [i ** 3 for i in range(N + 1)]
first = {}; edges = []
for d in range(2, N + 1):
    for c in range(1, d):
        s = cubes[c] + cubes[d]
        for (a, b) in first.get(s, []):
            edges.append((a, b, c, d))
        first.setdefault(s, []).append((c, d))
for n in range(1, N + 1):
    E = [e for e in edges if max(e) <= n]
    if not E:
        print(n, n, flush=True); continue
    vs = sorted(set(v for e in E for v in e) - {n})
    m = cp_model.CpModel()
    x = {v: m.NewBoolVar(f"x{v}") for v in vs}
    for e in E:
        m.AddBoolOr([x[v] for v in e if v != n])
    m.Minimize(sum(x.values()))
    sol = cp_model.CpSolver()
    sol.parameters.num_workers = W
    st = sol.Solve(m)
    if st != cp_model.OPTIMAL:
        print(f"n={n}: solver status {sol.StatusName(st)}", file=sys.stderr); sys.exit(2)
    print(n, n - round(sol.ObjectiveValue()), flush=True)
