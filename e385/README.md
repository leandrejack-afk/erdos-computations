# Erdős Problem 385 (and 430): exceptions to F(n) > n below 10^11

Problem: https://www.erdosproblems.com/385. Let F(n) = max over composite m < n of m + p(m),
where p(m) is the least prime factor of m. Erdős, Eggleton and Selfridge asked whether F(n) > n
for all sufficiently large n. Problem 430 (https://www.erdosproblems.com/430) is equivalent to this
question (observation of Sarosh Adenwalla recorded on both pages): the sequence defined there for n
consists entirely of primes exactly when F(n) <= n.

## Result

F(n) <= n holds for exactly 103 values of n <= 10^11. Three of them (n = 2, 3, 4) are trivial, since
no composite m < n exists. The other 100 are

6, 8, 12, 14, 18, 20, 24, 30, 32, 42, 44, 48, 60, 62, 72, 74, 84, 90, 102, 104, 108, 110, 114, 132, 140,
168, 182, 198, 200, 234, 240, 242, 270, 272, 282, 284, 312, 314, 318, 354, 360, 390, 420, 422, 434, 462,
464, 468, 510, 572, 648, 660, 662, 762, 840, 884, 888, 942, 1064, 1110, 1302, 1304, 1308, 1430, 1434,
1440, 1452, 1454, 1488, 1490, 1494, 1500, 1572, 2004, 2114, 2352, 2394, 2400, 2622, 2688, 2690, 2694,
2700, 2862, 2970, 2972, 3042, 3540, 3542, 4290, 4974, 5418, 5420, 5852, 5862, 5880, 5882, 8742,
267672, 267680.

So F(n) > n for every n with 267680 < n <= 10^11. Every exception is of the form q + 1 with q prime,
as predicted by the reduction posted by the commenter CKS on Terence Tao's blog post about this
problem (August 2024), where the values 8742, 267672 and 267680 were first reported.

## Method and verification

- `fn.c`: sieve of least prime factors up to N, running maximum of m + lpf(m) over composite m,
  reports every n with F(n) <= n. Run with N = 10^9 (6 seconds).
- `fn_seg.c`: segmented version for an interval [L, R]. Only composites m >= n - sqrt(n) - 1 can
  satisfy m + p(m) >= n (because p(m) <= sqrt(m)), so each interval is sieved from L - sqrt(R) - 1000.
  Ten instances covered [10^9, 10^11] in about one minute each and found no exception.
- `verify.py`: independent brute force from the definition using sympy (prime factorisations, no
  sieve), reproduces the full list of 103 values for n <= 300000.
- `spotcheck.py`: independent check of the definition at 30 pseudo-random n in [10^9, 10^11]
  (seed 20260903): for each n it exhibits a composite m < n with p(m) > n - m. All 30 passed.
- The segmented program was also checked against the plain sieve on [200000, 300000] (finds exactly
  267672 and 267680) and on [10^9, 1.2 x 10^9] (no exceptions), agreeing with `fn.c`.

Computed 2026-09-03 on an Apple M5 laptop. Programs written and run with Claude Code (Claude Fable
5.1) assistance; every claim above comes from the executed runs.
