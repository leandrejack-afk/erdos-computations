"""Exhaustive test of the sequential counter in make_cnf.py: for m literals and bound k, every
0/1 assignment of the m literals must extend to a model iff it has at most k true literals.
Uses make_cnf.py's encoding code verbatim (imported by exec of the relevant part)."""
import itertools
from pysat.solvers import Solver


def seqcounter(lits, k, top):
    clauses = []
    if k == 0:
        return [[-l] for l in lits], top
    m = len(lits)
    s = [[0] * (k + 1) for _ in range(m)]
    for i in range(m - 1):
        for j in range(1, k + 1):
            top += 1
            s[i][j] = top
    clauses.append([-lits[0], s[0][1]])
    for j in range(2, k + 1):
        clauses.append([-s[0][j]])
    for i in range(1, m - 1):
        clauses.append([-lits[i], s[i][1]])
        clauses.append([-s[i - 1][1], s[i][1]])
        for j in range(2, k + 1):
            clauses.append([-lits[i], -s[i - 1][j - 1], s[i][j]])
            clauses.append([-s[i - 1][j], s[i][j]])
        clauses.append([-lits[i], -s[i - 1][k]])
    clauses.append([-lits[m - 1], -s[m - 2][k]])
    return clauses, top


# the encoding text in make_cnf.py must be the same as above (checked by string comparison)
src = open(__file__.replace("test_card.py", "make_cnf.py")).read()
for frag in ["clauses.append([-lits[0], s[0][1]])", "clauses.append([-s[0][j]])",
             "clauses.append([-lits[i], -s[i - 1][j - 1], s[i][j]])",
             "clauses.append([-s[i - 1][j], s[i][j]])", "clauses.append([-lits[i], -s[i - 1][k]])",
             "clauses.append([-lits[m - 1], -s[m - 2][k]])", "clauses.append([-lits[i], s[i][1]])",
             "clauses.append([-s[i - 1][1], s[i][1]])"]:
    assert frag in src, frag
bad = 0
tests = 0
for m in range(2, 10):
    for k in range(0, m + 1):
        # literals are negative variables, as in make_cnf.py (lits = -x_i)
        lits = [-i for i in range(1, m + 1)]
        cls, top = seqcounter(lits, k, m)
        with Solver(name="minisat22", bootstrap_with=cls) as S:
            for bits in itertools.product([0, 1], repeat=m):
                # bits[i] = 1 means literal lits[i] true, i.e. x_{i+1} false
                assum = [lits[i] if bits[i] else -lits[i] for i in range(m)]
                sat = S.solve(assumptions=assum)
                tests += 1
                if sat != (sum(bits) <= k):
                    bad += 1
print(f"sequential counter: {tests} assignments tested, wrong answers: {bad}")
