# OEIS draft: minimal sum of an admissible n-tuple (Erdős Problem #1204)

Status: draft, not yet submitted. The b-file holds the terms confirmed by two independent exact
searches (n <= 147); `crosscheck.py --bfile-max 147` regenerates it and the agreement record.

## NAME

Minimal sum of an admissible n-tuple: a(n) is the least a_1 + ... + a_n over integers 0 <= a_1 < ... < a_n that omit at least one residue class modulo every prime.

## DATA

0, 2, 8, 16, 28, 46, 66, 92, 122, 154, 190, 232, 280, 330, 386, 448, 516, 588, 666, 752, 842, 938, 1036, 1138, 1248, 1364, 1484, 1612, 1744, 1882, 2022, 2168, 2320, 2476, 2634, 2796, 2964, 3140, 3322, 3508, 3696, 3894, 4094, 4304, 4516, 4732, 4962, 5202, 5444

## OFFSET

1,2

## COMMENTS

Erdős Problem #1204 (Erdős [Er80, p. 108], who attributes the question to Elliott) asks to estimate B(n) = a(n)/n, the minimal average of an admissible n-tuple; A008407(n) is the minimal diameter of the same tuples.

Translating a tuple down by a_1 keeps it admissible and lowers the sum, so the minimum is attained with a_1 = 0. The prime 2 then forces every element to be even, so a(n) is even and a(n)/2 is the minimal sum of an n-tuple of integers 0 = b_1 < ... < b_n that omits a class modulo every odd prime p <= n. Primes p > n need no check, since n residues cannot cover p classes.

a(n) >= Sum_{j=1..n} A008407(j), since the j-th element of an admissible n-tuple is at least A008407(j).

a(n) = Sum_{j=1..n} A135311(j), the sum of the first n terms of the greedy admissible sequence, for n <= 92; a(93) = 22474 while the greedy sum is 22480 (the optimal 93-tuple is the greedy one with 282 and 530 removed and 366 and 440 added), and a(n) is below the greedy sum for every n from 93 to 147.

The optimal n-tuple is unique for every n <= 147 except n = 109, where exactly two tuples attain a(109) = 31862: one begins 0, 2, 6, 8, 12, 18, 20, 26, ... and the other 0, 4, 6, 10, 16, 18, 24, 28, ...

B(n)/(n log n) decreases slowly: 0.594 at n = 40, 0.572 at n = 100, 0.567 at n = 147; the commentary on the problem page expects B(n) ~ (1/2 + o(1)) n log n. The ratio B(n)/A008407(n) stays between 0.45 and 0.48 for 40 <= n <= 147.

All terms were computed by two different exact searches (branch-and-bound over increasing tuples; search over the missed residue class of each prime, taking the n smallest survivors of the sieve) that agree on the values and on the optimal tuples for n <= 147; a MaxSAT model of the definition (python-sat, RC2) confirms values and uniqueness for n <= 38.

## LINKS

Leandre Jack, Table of n, a(n) for n = 1..147 (b-file-S.txt).
Thomas Bloom, Erdős Problem #1204, https://www.erdosproblems.com/1204
P. Erdős, A survey of problems in combinatorial number theory, Ann. Discrete Math. 6 (1980), 89-115 (MR 593525); the problem is on p. 108 and is cited as [Er80] on erdosproblems.com.
Leandre Jack, C and Python programs, https://github.com/... (repository path targets/e1204: smin.c, rvs.c, verify_sat.py, smin_short.py)

## FORMULA

a(n) >= Sum_{j=1..n} A008407(j).
a(n) = Sum_{j=1..n} A135311(j) for n <= 92.
a(n) < n*A008407(n) for n >= 2 (B(n) < A(n), noted on the problem page).

## EXAMPLE

a(5) = 28: the tuple (0, 2, 6, 8, 12) misses 1 mod 2, 1 mod 3 and 4 mod 5, and no admissible 5-tuple of nonnegative integers has a smaller sum.
a(6) = 46 from (0, 2, 6, 8, 12, 18); the diameter-minimal 6-tuple (0, 4, 6, 10, 12, 16) of A008407 has the larger sum 48.

## PROG

(Python)
```python
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

print([a(n) for n in range(1, 41)])
```
(The C programs smin.c and rvs.c in the repository are the two searches behind the b-file.)

## CROSSREFS

Cf. A008407 (minimal diameter of an admissible n-tuple), A135311 (greedy admissible sequence; partial sums give a(n) for n <= 92), A388999 (n with A135311(n) = A008407(n)), A023193, A020497.

## KEYWORD

nonn,hard

## AUTHOR

Leandre Jack, Sep 03 2026
