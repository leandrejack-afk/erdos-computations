"""a(n) = size of a largest Sidon subset of {1^3, ..., n^3}, n = 1..N, by incremental SAT.

The only conflicts are 4-sets {a, b, c, d} with a^3 + b^3 = c^3 + d^3 (x^3 + y^3 = 2z^3 has only
trivial solutions). a(n) = n - tau(n), tau(n) = minimum number of numbers to delete so that every
conflict with all elements <= n loses an element. Since H_n adds only conflicts containing n,
tau(n-1) <= tau(n) <= tau(n-1) + 1, so each n needs one SAT call: can tau(n-1) deletions hit all
conflicts? (At-most-k by an incremental totalizer, k passed as an assumption.)
usage: sidon_sat.py N > a.txt
"""
import sys
from collections import defaultdict
from pysat.card import ITotalizer
from pysat.solvers import Solver

N = int(sys.argv[1])
sums = defaultdict(list)
for a in range(1, N + 1):
    a3 = a ** 3
    for b in range(a + 1, N + 1):
        sums[a3 + b ** 3].append((a, b))
edges_at = defaultdict(list)
for reps in sums.values():
    if len(reps) > 1:
        for i in range(len(reps)):
            for j in range(i + 1, len(reps)):
                e = reps[i] + reps[j]
                edges_at[max(e)].append(e)
del sums
var = {}
def x(v):
    if v not in var:
        var[v] = len(var) + 1
    return var[v]
S = Solver(name="cadical153")
tot = None
tau = 0
top = [0]
for n in range(1, N + 1):
    new = edges_at.get(n, [])
    if new:
        fresh = []
        for e in new:
            for v in e:
                if v not in var:
                    fresh.append(x(v))
        # totalizer over all deletion variables, extended with the new ones
        if tot is None:
            tot = ITotalizer(lits=[x(v) for v in sorted(var)], ubound=tau + 1, top_id=10 ** 6)
            for cl in tot.cnf.clauses:
                S.add_clause(cl)
        elif fresh:
            ncl = len(tot.cnf.clauses)
            tot.extend(lits=fresh, ubound=tau + 1)
            for cl in tot.cnf.clauses[ncl:]:
                S.add_clause(cl)
        for e in new:
            S.add_clause([x(v) for v in e])
        if tau + 1 > tot.ubound:
            ncl = len(tot.cnf.clauses)
            tot.increase(ubound=tau + 1)
            for cl in tot.cnf.clauses[ncl:]:
                S.add_clause(cl)
        if not S.solve(assumptions=[-tot.rhs[tau]]):
            tau += 1
    print(n, n - tau, flush=True)
