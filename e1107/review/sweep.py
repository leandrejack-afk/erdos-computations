"""Boundary sweep (review): run fresh builds of the implementer's sumfull.c / sum4full.c at many N and
compare each output with the exceptions <= N from the review DP (out/dp_r_1e7.txt, complete below 1e7
because the DP run at N = 1e7 sees every coin <= 1e7). N values: every N in 1..2100, N = 64k-1, 64k,
64k+1 over a range of k, N = a-1, a, a+1 for every r-full a <= 2e6, and some random N <= 1e7.
usage: python3 -I sweep.py <review dir>"""
import bisect
import random
import subprocess
import sys

D = sys.argv[1]
exc = {r: [int(x) for x in open(f"{D}/out/dp_{r}_1e7.txt")] for r in (4, 5, 6)}
full = {r: [int(x) for x in open(f"{D}/out/rfull_{r}_1e9.txt")] for r in (4, 5, 6)}


def expected(r, N):
    v = exc[r]
    return v[: bisect.bisect_right(v, N)]


def run(cmd):
    p = subprocess.run(cmd, capture_output=True, text=True, check=True)
    return [int(x) for x in p.stdout.split()]


random.seed(1107)
Ns = set(range(1, 2101))
for k in list(range(1, 400)) + [1000, 4096, 15625, 65536, 100000, 150000]:
    Ns.update({64 * k - 1, 64 * k, 64 * k + 1})
Ns.update(random.randrange(1, 10**7) for _ in range(40))
Ns.update({150270, 150271, 150272, 449559, 449560, 449561, 4034494, 4034495, 4034496})
runs = bad = 0
for r in (4, 5, 6):
    Nr = set(Ns)
    for a in full[r]:
        if a <= 2 * 10**6:
            Nr.update({a - 1, a, a + 1})
    Nr.discard(0)
    progs = [[f"{D}/bin/sumfull_fresh", str(r)]]
    if r == 4:
        progs.append([f"{D}/bin/sum4full_fresh"])
    for N in sorted(Nr):
        e = expected(r, N)
        for pg in progs:
            got = run(pg + [str(N)])
            runs += 1
            if got != e:
                bad += 1
                print("MISMATCH", pg[0].split('/')[-1], r, N, len(got), len(e))
    print(f"r={r}: {len(Nr)} values of N, max {max(Nr)}", flush=True)
print(f"total runs {runs}, mismatches {bad}")
