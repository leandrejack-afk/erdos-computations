"""Check the lower bound c(n) >= value for the variant that must contain n^3, from the definition.

For every logged removal set D (contain_ilp.py stderr, "n t [..]"), take A = {1..n} minus D and
test that n is in A, that ALL sums x^3 + y^3 with x <= y in A (x = y included) are distinct, with
exact integers, and that |A| equals the c-file value. Then compare with the b-file of a(n) and
list every n where c(n) < a(n).
usage: check_contain.py log.err c-file b-file
"""
import ast
import sys

logged = {}
for line in open(sys.argv[1]):
    if not line[:1].isdigit():
        continue
    n, t, rest = line.split(" ", 2)
    logged[int(n)] = (int(t), ast.literal_eval(rest.strip()))
c = {int(l.split()[0]): int(l.split()[1]) for l in open(sys.argv[2])}
a = {int(l.split()[0]): int(l.split()[1]) for l in open(sys.argv[3])}
N = max(c)
problems = 0
if sorted(logged) != list(range(1, N + 1)) or sorted(c) != list(range(1, N + 1)):
    problems += 1
    print("n ranges differ")
for n in range(1, N + 1):
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
    if not ok or n in D or len(D) != t or len(A) != c[n] or c[n] > a[n]:
        problems += 1
        print("problem at n =", n, "t =", t, "|A| =", len(A), "c =", c[n], "a =", a[n], "sidon =", ok, "n kept =", n not in D)
low = [n for n in range(1, N + 1) if c[n] < a[n]]
print(f"checked n = 1..{N}: every complement of a logged removal set contains n, is Sidon by direct "
      f"pair sums and has size c(n): problems = {problems}")
print("c(n) < a(n) at n =", low, "with a(n) - c(n) =", [a[n] - c[n] for n in low])
