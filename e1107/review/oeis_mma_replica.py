"""Replica of the OEIS Mathematica programs (A392342, A392343): t = {0, 1} u {r-full k <= n, k >= 2}
with r-fullness from a direct factorisation of every k (FactorInteger analogue, trial division),
then Complement[Range[n], Flatten[Outer[Plus, t, ..., t]]] with r + 1 copies of t, done as iterated
Python set sums capped at n. Independent of every C program and of the review DP.
usage: python3 -I oeis_mma_replica.py r n"""
import sys

r, n = int(sys.argv[1]), int(sys.argv[2])


def min_exponent(k):
    m, e_min, p = k, None, 2
    while p * p <= m:
        if m % p == 0:
            e = 0
            while m % p == 0:
                m //= p
                e += 1
            e_min = e if e_min is None else min(e_min, e)
        p += 1
    if m > 1:
        e_min = 1 if e_min is None else min(e_min, 1)
    return e_min


t = [0, 1] + [k for k in range(2, n + 1) if min_exponent(k) > r - 1]
S = {0}
for _ in range(r + 1):
    S = {s + a for s in S for a in t if s + a <= n}
print("\n".join(str(k) for k in range(1, n + 1) if k not in S))
