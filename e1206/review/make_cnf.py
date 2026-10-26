"""Write a DIMACS CNF that is satisfiable iff some A in {1..n} with |A| >= K has Sidon cubes.

Variables 1..n: x_i true iff i in A. Clauses come straight from the definition: list ALL pair
sums x^3 + y^3 with x <= y <= n (x = y included); for every two different representations r, r'
of one value, forbid having both inside A (one clause: not all of the elements of r and r').
Cardinality: at most n - K of the literals -x_1..-x_n are true (Sinz sequential counter).
usage: make_cnf.py n K out.cnf
"""
import sys
from collections import defaultdict

n, K, out = int(sys.argv[1]), int(sys.argv[2]), sys.argv[3]
reps = defaultdict(list)
for x in range(1, n + 1):
    for y in range(x, n + 1):
        reps[x ** 3 + y ** 3].append((x, y))
clauses = []
for r in reps.values():
    for i in range(len(r)):
        for j in range(i + 1, len(r)):
            els = sorted(set(r[i]) | set(r[j]))
            clauses.append([-e for e in els])

# at most k of lits true, sequential counter (Sinz 2005)
k = n - K
lits = [-i for i in range(1, n + 1)]
top = n
if k < 0:
    clauses.append([])  # impossible: K > n
elif k == 0:
    clauses += [[-l] for l in lits]
else:
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
with open(out, "w") as f:
    f.write(f"c Sidon cubes: n={n} K={K}\np cnf {top} {len(clauses)}\n")
    for c in clauses:
        f.write(" ".join(map(str, c)) + " 0\n")
