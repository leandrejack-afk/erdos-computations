# Erdős Problem 1107: numbers that are not a sum of at most r + 1 r-full numbers

Problem: https://www.erdosproblems.com/1107. A positive integer n is r-full if p^r divides n for
every prime p dividing n (1 is r-full). Erdős and Ivić conjectured that every sufficiently large
integer is a sum of at most r + 1 r-full numbers. Heath-Brown proved it for r = 2.

The exceptions are OEIS A056828 (r = 2: 7, 15, 23, 87, 111, 119; Henry Bottomley, 2000), A392342
(r = 3: 45 terms, the last 2039, no others below 10^9 by David Cleaver) and A392343 (r = 4: 70
terms listed; the Mathematica program in the entry searches up to 31000). A392342 and A392343
were added by Elijah Beregovsky in January 2026; A392343 had no pending edits on 26 Oct 2026.

## Results

| r | exceptions found | largest | searched up to | OEIS |
|---|---|---|---|---|
| 4 | 1318 | 150271 | 10^10 | A392343 (b-file n = 1..1318) |
| 5 | 6018 | 449560 | 10^10 | new |
| 6 | 37036 | 4034495 | 10^10 | new |

So every integer from 150272 to 10^10 is a sum of at most five 4-full numbers, and the 1318
numbers in `b392343.txt` are all the exceptions up to 10^10. Whether any exception lies beyond
10^10 is open (the conjecture says there are finitely many). Likewise for r = 5 (6018 exceptions, the last
449560) and r = 6 (37036 exceptions, the last 4034495), each searched to 10^10. The lists found
with N = 10^9 and with N = 10^10 are identical.

The counts by decade show how quickly the exceptions stop:

| r | 1..9 | 10..99 | 10^2.. | 10^3.. | 10^4.. | 10^5.. | 10^6.. | 10^7.. and up |
|---|---|---|---|---|---|---|---|---|
| 4 | 4 | 61 | 401 | 500 | 350 | 2 | 0 | 0 |
| 5 | 3 | 74 | 591 | 2390 | 2802 | 158 | 0 | 0 |
| 6 | 2 | 83 | 789 | 5701 | 19490 | 10107 | 864 | 0 |

## Method

The r-full numbers up to N are listed, and the set S_j of sums of at most j of them is built as a
bit array over [0, N]: S_0 = {0} and S_{j+1} = S_j ∪ (S_j + a) over all r-full a, by shifted ORs.
The exceptions are the n in [1, N] outside S_{r+1}. This is exhaustive; nothing is sampled.

## Checks

1. `sumfull.c` reproduces A056828 (r = 2) and all 45 terms of A392342 (r = 3), searched to 10^6
   (`r2_1e6.*`, `r3_1e6.*`), and the first 70 terms of A392343.
2. Two listings of the 4-full numbers (`sum4full.c`: every x^4 b^5 c^6 d^7, duplicates removed;
   `sumfull.c`: depth-first over primes and exponents >= r) give identical r = 4 lists up to 10^9.
   The two programs share the same bitset sumset code.
3. `check_np.py` (numpy; r-full numbers from a smallest-prime-factor sieve, its own shifted ORs)
   gives identical lists for r = 4 up to 10^8 and for r = 5 and r = 6 up to 2 x 10^7.
4. A separate review wrote a check with a different algorithm, a dynamic-programming count of
   representations (`review/dpcheck.c`), which gives the same three lists up to 10^9, and a tail
   check that finds no exception in (10^9, 10^10] for any of the three r (`review/out/*.log`). Its
   r-full numbers up to 10^10 agree with a sieve and with the OEIS b-files of A036967, A069492 and
   A069493.
5. The runs to 10^10 were made on a second machine (`server/MACHINE.txt`: x86 Linux, gcc) and give
   the same lists as the runs to 10^9 on the first (ARM macOS, clang).

`checks.log` records the comparisons.

## Files

`sumfull.c`, `sum4full.c`, `check_np.py`, the b-files (`b392343.txt`, `b_r5.txt`, `b_r6.txt`, in
the OEIS format "n a(n)"), the logs, `server/` (the runs to 10^10) and `review/`.

These programs were written with the assistance of a large language model; the results rest on
the checks listed above.
