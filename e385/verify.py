# Independent brute-force check of the Erdős 385 exception list for n <= LIMIT (written from the statement, not from fn.c).
import sys
from sympy import primefactors, isprime
LIMIT = int(sys.argv[1]) if len(sys.argv) > 1 else 300000
lpf = {}
best = 0
exceptions = []
for n in range(2, LIMIT + 1):
    m = n - 1
    if m >= 4 and not isprime(m):
        reach = m + min(primefactors(m))
        best = max(best, reach)
    if best <= n:
        exceptions.append(n)
print(len(exceptions), "exceptions up to", LIMIT)
print(exceptions)
