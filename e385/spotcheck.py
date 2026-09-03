# Independent spot check of F(n) > n for random n in [1e9, 1e11], from the definition:
# F(n) > n iff there is a composite m < n with lpf(m) > n - m. Only m >= n - sqrt(n) - 1 can qualify.
import random, math, sys
from sympy import isprime, primerange
random.seed(20260903)
def good(n):
    lim = int(math.isqrt(n)) + 2
    primes = list(primerange(2, lim + 1))
    for d in range(1, lim + 1):
        m = n - d
        if m < 4 or isprime(m):
            continue
        # m composite; need lpf(m) > d
        ok = True
        for q in primes:
            if q > d:
                break
            if m % q == 0:
                ok = False
                break
        if ok:
            return True, m, d
    return False, None, None
res = []
for _ in range(int(sys.argv[1]) if len(sys.argv) > 1 else 25):
    n = random.randint(10**9, 10**11)
    g, m, d = good(n)
    res.append((n, g, m, d))
    print(n, "F(n)>n" if g else "EXCEPTION", m, d, flush=True)
print("all good:", all(r[1] for r in res))
