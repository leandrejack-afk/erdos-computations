"""Reviewer check of the lower bound a(n) >= b-file value, straight from the definition.

For every logged deletion set D (sidon_cpsat.py stderr, "n t [..]"), take A = {1..n} minus D and
test that ALL sums x^3 + y^3 with x <= y in A (x = y included) are distinct, with exact integers.
Also check that every n in 1..N appears exactly once, that t = |D|, and that |A| equals the
b-file value. No reduction to 4-sets is used.
usage: check_witnesses.py log.err b-file
"""
import ast
import sys

logged = {}
for line in open(sys.argv[1]):
    if not line[:1].isdigit():
        continue
    n, t, rest = line.split(" ", 2)
    n, t = int(n), int(t)
    if n in logged:
        print("duplicate row for n =", n)
    logged[n] = (t, ast.literal_eval(rest.strip()))
b = {}
for line in open(sys.argv[2]):
    n, v = map(int, line.split())
    b[n] = v
N = max(b)
problems = 0
if sorted(logged) != list(range(1, N + 1)) or sorted(b) != list(range(1, N + 1)):
    problems += 1
    print("n ranges differ:", min(logged), max(logged), len(logged), "vs", min(b), max(b), len(b))
for n in range(1, N + 1):
    if n not in logged:
        problems += 1
        print("no deletion set logged for n =", n)
        continue
    t, D = logged[n]
    D = set(D)
    A = [x for x in range(1, n + 1) if x not in D]
    sums = set()
    ok = True
    for i, x in enumerate(A):
        for y in A[i:]:
            s = x ** 3 + y ** 3
            if s in sums:
                ok = False
                break
            sums.add(s)
        if not ok:
            break
    if not ok or len(D) != t or any(not 1 <= v <= n for v in D) or len(A) != b[n] or n - t != b[n]:
        problems += 1
        print("problem at n =", n, "t =", t, "|A| =", len(A), "b =", b[n], "sidon =", ok)
print(f"checked n = 1..{N}: every complement of a logged deletion set is Sidon by direct pair sums "
      f"and has size b(n): problems = {problems}")
