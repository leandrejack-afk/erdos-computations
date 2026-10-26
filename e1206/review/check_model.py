"""Read a SAT solver's 'v' lines for make_cnf.py's CNF and check the model from the definition:
A = {i <= n : x_i true} must have |A| >= K and all sums x^3 + y^3 (x <= y in A) distinct.
usage: check_model.py n K solver_output"""
import sys

n, K = int(sys.argv[1]), int(sys.argv[2])
vals = []
for line in open(sys.argv[3]):
    if line.startswith("v "):
        vals += [int(t) for t in line.split()[1:]]
A = sorted(v for v in vals if 0 < v <= n)
sums = {}
bad = None
for i, x in enumerate(A):
    for y in A[i:]:
        s = x ** 3 + y ** 3
        if s in sums:
            bad = (sums[s], (x, y))
            break
        sums[s] = (x, y)
    if bad:
        break
print(f"n={n} K={K}: |A|={len(A)} sidon={'no ' + str(bad) if bad else 'yes'} "
      f"{'OK' if not bad and len(A) >= K else 'FAIL'}")
