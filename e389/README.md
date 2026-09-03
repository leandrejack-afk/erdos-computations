# Erdős Problem 389: the least k for n = 28 and n = 29

Problem: https://www.erdosproblems.com/389. For n >= 1, is there always a k >= 1 with

    n (n+1) ... (n+k-1)  |  (n+k) (n+k+1) ... (n+2k-1) ?

OEIS A375071 records the least such k for each n (offset 0, so index i holds the value for
n = i + 1). Whether such a k exists for every n is open; this computation only settles the least k
for two more values of n, and records upper bounds and a correction for six more.

## What was already known

A375071 was published through a(26), that is n = 1..27: a(0)..a(9) (n = 1..10) from the original
entry and Bhavik Mehta; a(10)..a(18) (n = 11..19) from Bhavik Mehta, Aug 16 2024; a(19)..a(26)
(n = 20..27) from Sharvil Kesarwani, Mar 30 2026. (The review record notes that the entry's credit
lines look shifted by one relative to its data field; see REVIEW.md, item E5.)

On the problem's forum thread, KentaKitamura (01 Aug 2026) posted witnesses for n = 28..33, stated
as upper bounds only ("These are upper-bound witnesses only"), and Sharvil Kesarwani (25 Aug 2026)
added values for n = 34, 35 and pointed to his solver at https://github.com/sharky564/ErdosProblems.
None of these had been shown minimal.

## Result

The upper bounds for n = 28 and n = 29 are the exact least k:

| n  | least k         | N = n + k - 1   | OEIS  |
|---:|----------------:|----------------:|:------|
| 28 | 18253129921815  | 18253129921842  | a(27) |
| 29 | 18253129921814  | 18253129921842  | a(28) |

The two share one N because n = 28 and n = 29 are the pair (2K+2, 2K+3) with K = 13 and the
divisibility depends only on N and the block length, so one scan settles both. For n = 28 the
statement reads: 28 * 29 * ... * 18253129921842 divides 18253129921843 * ... * 36506259843657, that
is C(2N, 27) | C(2N, N) with N = 18253129921842, and no smaller k works.

Minimality rests on one completed exhaustive run: Sharvil Kesarwani's published solver scanned
N over [1, 18253129921842] and returned that N as the least witness for both n. The C sieve in this
folder is a second implementation of the same governor-sieve method, not an independent method and
not written from scratch; it reproduced a(1)..a(24) and agreed with the published solver on a
2^36-wide window near N = 10^13, but it did not complete its own scan of the n = 28 range.

Values that remain upper bounds only, each verified to divide by the prime-valuation check:

| n  | witness k (upper bound) | N                   |
|---:|------------------------:|--------------------:|
| 30 | 359503904702161         | 359503904702190     |
| 31 | 359503904702160         | 359503904702190     |
| 32 | 2394789405254690        | 2394789405254721    |
| 33 | 2394789405254689        | 2394789405254721    |
| 34 | 20021368432952066       | 20021368432952099   |
| 35 | 20021368432952065       | 20021368432952099   |

Exhaustive scans for these would run over N up to about 3.6 x 10^14 (n = 30, 31), 2.4 x 10^15
(n = 32, 33) and 2.0 x 10^16 (n = 34, 35), out of reach on one machine. One correction: the n = 34, 35
values Sharvil Kesarwani posted on the forum, k = 20021368432952132 and 20021368432952131, are not
witnesses; both fail at p = 67. They equal N + n - 1 instead of N - n + 1 for
N = 20021368432952099, which is a(16) of A375077 (Justin Dehorty, Jul 28 2026). The corrected values
in the table, k = 20021368432952066 (n = 34) and 20021368432952065 (n = 35), do divide. Their
minimality is unproven.

A side result, prompted by a question on the thread from old-bielefelder about the second-smallest
k: for n = 22 the next k after 17609764973 is 34716149191, and for n = 23 the next after 17609764972
is 34716149190 (both with N = 34716149212). Both divide and the value one below each fails at
p = 807352307 (valuation check, re-run at packaging time). The claim that nothing lies between the
first and second witnesses comes from a resumed scan described in the private results record
(coverage [17609764995, 18724421632) by a gap scan and [18724421632, 34716149212] by the resumed
scan); no log of that scan is among the files here.

## Method

Write a = n - 1 and N = n + k - 1. Then left | right is equivalent to C(2N, a) | C(2N, N). Prime by
prime (Kummer's theorem), for a prime p > a + 1 the condition reduces to a "governor" condition on
the h = ceil(a/2) numbers N, N-1, ..., N-h+1: each such m must satisfy v_p(m) <= (number of carries
when m / p^{v_p(m)} is added to itself in base p). A segmented sieve accumulates floor(4 log2 p) per
prime-power factor of every m in a chunk, withholds the mass of primes that a divisor-size rule or a
base-p digit rule proves cannot be governors, and keeps only the m whose surviving mass clears a
threshold that every fully accounted m provably meets. A run of h consecutive kept m ending at N is a
candidate, and candidates go to an exact Legendre-valuation oracle. The threshold is sound for
N < 2^49 with the u8 accumulator used (argument in the header of `e389_sieve.c`). Both n (even) and
n + 1 share the same h, so one scan serves the pair. The published solver uses the same design;
`e389_sieve.c` was written in C with that source at hand.

## Verification

Full detail with quoted output is in `verify.md`; the adversarial review, which re-derived every step
with its own code, is `REVIEW.md` (verdict: safe with wording edits; this README follows its wording requirements).

- The witnesses divide, by prime valuations (`e389_check.py pair`, Legendre's formula at every
  prime factor of 2N, 2N-1, ..., 2N-a+1). The big-integer product and the central-binomial ratio
  are gated to k <= 2 x 10^7 and were used only at small k, where all three paths agree on the full
  solution set for k <= 3000 (n = 4 and n = 6). N - 1 fails for both n at p = 84239.

      python3 e389_check.py pair 28 18253129921815   -> valuation: DIVIDES
      python3 e389_check.py pair 29 18253129921814   -> valuation: DIVIDES
      python3 e389_check.py pair 28 18253129921814   -> valuation: fails at p=84239

- No smaller k divides: `reference-sharvil-erdos_problem_389.cpp` (Sharvil Kesarwani's solver,
  byte-identical to the master file at https://github.com/sharky564/ErdosProblems, md5
  0c61bca7a1a83543e8c6bafc277cacc3) run in pair worker mode over N in [1, 18253129921842], about
  1 h 50 m of wall time on 10 threads. Its result line is `logs/evidence-sharvil-n28.txt`:

      PAIR=1 NT=10 ./sharvil389 28 1 18253129921842
      RESULT 28 18253129921842 18253129921842

  The two fields are the least N for n = 28 and for n = 29. The review points out that the original
  run kept no record of its command line. A rerun that logs the command line was started on
  2026-09-03 at 17:39 (`logs/rerun-sharvil-n28.log`); it was still running when this folder was
  packaged, so `logs/rerun-sharvil-n28.out` and `.err` are empty here.

- The C sieve reproduces every published term it was run against, each pair scanned from k = 1:
  a(1)..a(22) by `regress.sh` (output quoted in `verify.md`), and a(23), a(24) by a full scan to
  N = 1.07 x 10^12 in 1088 s (`logs/regress_24.out`, `logs/regress_24.err`):

      RESULT n=24 N=1070858041585 k=1070858041562
      RESULT n=25 N=1070858041585 k=1070858041561

  It also agrees with the published solver on a 2^36-wide window near N = 10^13 (both report no
  witness). Its own n = 28 scan was stopped below one percent of the range on the shared machine.

- The sieve's per-candidate verdicts were re-derived by the valuation method
  (`verify_survivors.py`): 14662 verdicts from the n = 20 run and a sample of 2994 from the n = 22
  run, all agreeing, with the blamed prime confirmed to fail each time. The survivor files are
  regenerated by `regress.sh`.

- Rejection sampling at the frontier: 40 random k below each witness, all rejected by the valuation
  check. Range splitting and checkpoint resume were tested at the n = 20 and n = 22 witnesses.

- The review (REVIEW.md) added its own valuation checker built from the definition, reproduced all
  27 published terms with k - 1 failing each time, found no k for n = 28 or 29 with N < 2^27 by its
  own brute force, rebuilt the published solver and confirmed it returns the witness on
  [18253129787624, 18253129921842] and nothing below 2^30, and re-derived the sieve's rejection
  rules and threshold arithmetic.

## Files

- `e389_sieve.c`: the C governor sieve (usage in its header). `e389_check.py`: the three
  divisibility checks plus `sample`, `agree` and `bruteforce` modes (needs gmpy2 and sympy).
  `verify_survivors.py`: re-derives sieve verdicts. `regress.sh`: reproduces a(1)..a(22).
- `reference-sharvil-erdos_problem_389.cpp`: Sharvil Kesarwani's published solver, copied unchanged
  from https://github.com/sharky564/ErdosProblems (md5 0c61bca7a1a83543e8c6bafc277cacc3).
- `logs/`: the regression output for n = 24, 25, the result line of the completed published-solver
  run, and the log of the rerun in progress.
- `oeis-extension.txt`: the note prepared for the A375071 editors. `verify.md`: the verification
  record. `REVIEW.md`: the adversarial review.

## Reproduce

    cc -O3 -mcpu=native -std=c11 -Wall -Wextra -pthread e389_sieve.c -o e389_sieve -lm
    ./regress.sh                                     # a(1)..a(22); writes surv_<n>.txt
    ./e389_sieve 24 24 2200000000000 -t 10           # a(23), a(24); about 18 minutes
    ./e389_sieve 28 28 18253129921843 -t 10          # the full n = 28, 29 scan; several hours
    python3 e389_check.py pair 28 18253129921815     # witness divides
    python3 e389_check.py sample 28 18253129921815 40
    python3 verify_survivors.py surv_20.txt 20       # after regress.sh
    g++ -O3 -march=native -std=c++23 reference-sharvil-erdos_problem_389.cpp -o sharvil389
    PAIR=1 NT=10 ./sharvil389 28 1 18253129921842    # the published solver; about 2 hours

Computed 2026-09-03 on an Apple M5 laptop. Programs written and run with Claude Code (Claude Fable
5.1) assistance; every claim above comes from the executed runs.
