"""Independent check below N (numpy): r-full numbers from a smallest-prime-factor sieve, then the
sumset of r+1 copies of {0} u F by shifted ORs on a boolean array. usage: check_np.py N out [r]"""
import sys
import numpy as np

N = int(sys.argv[1])
R = int(sys.argv[3]) if len(sys.argv) > 3 else 4
spf = np.zeros(N + 1, dtype=np.int32)
for p in range(2, int(N ** 0.5) + 1):
    if spf[p] == 0:
        blk = spf[p * p::p]
        blk[blk == 0] = p
r = np.arange(N + 1, dtype=np.int64)
spf[spf == 0] = r[spf == 0].astype(np.int32)  # primes (and 0, 1)


def r_full(n):
    if n == 1:
        return True
    while n > 1:
        p = int(spf[n]); e = 0
        while n % p == 0:
            n //= p; e += 1
        if e < R:
            return False
    return True


F = [n for n in range(1, N + 1) if n == 1 or (int(spf[n]) ** R <= n and r_full(n))]
S = np.zeros(N + 1, dtype=bool); S[0] = True
for _ in range(R + 1):
    T = S.copy()
    for a in F:
        T[a:] |= S[:N + 1 - a]
    S = T
ex = np.nonzero(~S[1:])[0] + 1
print(len(F), f"{R}-full;", len(ex), "exceptions; largest", ex[-1] if len(ex) else None)
np.savetxt(sys.argv[2], ex, fmt="%d")
