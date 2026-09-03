#!/usr/bin/env python3
"""Compact exact program for the OEIS PROG field (same method as rvs.c): a(n) = S(n) for Erdős 1204.
Usage: smin_short.py NMAX   prints a(1..NMAX).
"""


def a(n):
    P = [p for p in range(3, n + 1, 2) if all(p % d for d in range(3, int(p ** 0.5) + 1, 2))]
    # halved space: a_i = 2*b_i, and b must miss a class mod every odd prime p <= n
    b, cov = [0], {p: {0} for p in P}
    while len(b) < n:                                   # greedy tuple gives the incumbent
        x = b[-1] + 1
        while any(len(cov[p]) == p - 1 and x % p not in cov[p] for p in P):
            x += 1
        b.append(x)
        for p in P:
            cov[p].add(x % p)
    best = [sum(b)]
    xlim = best[0] - (a(n - 1) // 2 if n > 1 else 0)   # b_n <= best - S(n-1)/2
    dead = [0] * (xlim + 1)

    def ksum():                                        # sum of the n smallest survivors
        s, c = 0, 0
        for x in range(xlim + 1):
            if not dead[x]:
                s += x
                c += 1
                if c == n:
                    return s
        return None

    def rec(j):
        s = ksum()
        if s is None or s > best[0]:
            return
        if j == len(P):
            best[0] = s
            return
        p = P[j]
        for r in range(1, p):
            if not any(not dead[x] for x in range(r, xlim + 1, p)):
                rec(j + 1)                             # a class with no survivor costs nothing
                return
        for r in range(1, p):
            for x in range(r, xlim + 1, p):
                dead[x] += 1
            rec(j + 1)
            for x in range(r, xlim + 1, p):
                dead[x] -= 1

    rec(0)
    return 2 * best[0]


if __name__ == "__main__":
    import sys
    print(", ".join(str(a(n)) for n in range(1, int(sys.argv[1]) + 1)))
