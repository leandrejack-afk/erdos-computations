# Erdős Problem 382: the first maximal bad intervals of widths 9 to 13 and 15

Problem: https://www.erdosproblems.com/382. Let u <= v be such that the largest prime dividing
u(u+1)...v appears with exponent at least 2. Is it true that v - u = v^{o(1)}? Can v - u be
arbitrarily large?

Erdős and Graham called such an interval [u, v] bad. OEIS A388654 lists the integers that lie in
at least one bad interval, and OEIS A388850 gives a(n), the first term of the first maximal bad
interval of width n, that is, the start of the first run of exactly n + 1 consecutive members of
A388654.

## What was already known

A388850 (Pontus von Brömssen, Sep 2025) has nine terms, a(0) to a(8):
4, 8, 48, 1680, 76725, 332925, 7474752, 4541154, 75047565. Its comments give upper bounds for the
next five: a(9) <= 4928180396, a(10) <= 40533366231, a(11) <= 111460496439 and
a(12) <= 436502026483 (Touch Sungkawichai, 3 Jan 2026), and a(13) <= 17952448643078 (StijnC, on the
problem's forum thread). The entry was last edited on 7 Jan 2026 and had no pending edits on
11 Oct 2026.

## Result

Each of the five upper bounds is the exact value:

| n | a(n) | the run (n + 1 integers) | largest prime p | where p^2 sits |
|---|------|--------------------------|-----------------|----------------|
| 9 | 4928180396 | 4928180396 to 4928180405 | 70201 | 70201^2 = 4928180401 |
| 10 | 40533366231 | 40533366231 to 40533366241 | 201329 | 201329^2 = 40533366241 |
| 11 | 111460496439 | 111460496439 to 111460496450 | 333857 | 333857^2 = 111460496449 |
| 12 | 436502026483 | 436502026483 to 436502026495 | 660683 | 660683^2 = 436502026489 |
| 13 | 17952448643078 | 17952448643078 to 17952448643091 | 4237033 | 4237033^2 = 17952448643089 |

So A388850 continues 4928180396, 40533366231, 111460496439, 436502026483, 17952448643078.

The search was then continued to 2 x 10^14. No run of exactly 15 consecutive members of A388654
starts at or below 2 x 10^14, so a(14) = 0 or a(14) > 2 x 10^14 (the entry uses 0 for a width that
never occurs, and whether arbitrarily wide bad intervals exist is the second question of the
problem). There is, however, a run of 16, and it is the first one:

| n | a(n) | the run (n + 1 integers) | largest prime p | where p^2 sits |
|---|------|--------------------------|-----------------|----------------|
| 15 | 52629641927627 | 52629641927627 to 52629641927642 | 7254629 | 7254629^2 = 52629641927641 |

so a(15) = 52629641927627. No run of 17 or more starts at or below 2 x 10^14.

Below 2 x 10^14 there are 616 maximal runs of length at least 10: 408 of length 10, 137 of length 11,
48 of length 12, 15 of length 13, 7 of length 14, none of length 15 and 1 of length 16 (all listed in
`logs/cand_2e14_min10.txt`; the 182 below 2 x 10^13 are also in `logs/cand_2e13_min10.txt`). In
each of the six first runs above, the square of the largest prime is itself a member of the run and
every other member has all its prime factors below p.

## Why the search is exhaustive

Write P(k) for the largest prime factor of k (P(1) = 1). Call m > 1 a square point if P(m)^2
divides m, and let W(m) be the longest interval of integers containing m on which every k has
P(k) <= P(m).

1. Every bad interval [u, v] contains a square point m with P(m) equal to the largest prime p of
   the product. (This is the start of Section 6 and Lemma 6.1 of Terence Tao, "Products of
   consecutive integers with unusual anatomy", arXiv:2603.27990, the paper cited in A388654; the
   short argument is repeated here.) Otherwise p divides two different members, so v - u >= p and every member has all
   its prime factors at most p <= v - u. If u > v - u + 1, Sylvester's theorem (a product of k
   consecutive integers, all greater than k, has a prime factor greater than k), applied with
   k = v - u + 1, gives a prime factor larger than p, which is impossible. If u <= v - u + 1, then
   v >= 2u - 1, and (for v >= 2; the interval [1, 1] has no prime factor at all) Bertrand's
   postulate puts a prime q with v/2 < q <= v inside [u, v]; then p >= q > v/2, so p divides
   exactly one member and only once, and the interval is not bad.
2. Each W(m) is bad (its largest prime is P(m), and P(m)^2 divides m), and every bad interval lies
   inside W(m) for the square point m from step 1. So A388654 is exactly the union of the W(m).
3. If W(m1) and W(m2) overlap or touch end to end, and P(m1) <= P(m2), then W(m1) lies inside
   W(m2): their union is an interval containing m2 on which P stays at most P(m2). So the windows
   that sit inside no other window are separated by at least one non-member, and they are exactly
   the maximal runs of A388654.

Hence a(n) is the smallest start of a window W(m) of length n + 1 that lies inside no other window.
Listing every square point m <= X therefore finds every maximal run that ends at or below X, and in
particular every run that could come before any of the runs above. The run containing
2 x 10^13 = 2^14 * 5^13 is that number alone, and so is the run containing 2 x 10^14 = 2^15 * 5^14,
so no run crosses either bound, and "ends at or below" and "starts at or below" mean the same
there.

## Method

Three programs, each run over the whole range it reports on:

- `cand.c` lists every square point m = p^2 s <= X (p prime, all prime factors of s at most p) by a
  depth-first enumeration of the s for each p, computes W(m) by testing the neighbours of m one at a
  time (is every prime factor at most p? trial division by the primes up to min(p, 2048), then a
  deterministic Miller-Rabin test and Pollard-Brent factoring of what is left), and keeps the
  windows that lie inside no other window.
- `cand2.c` was written separately as a check on `cand.c`. It takes the s with X/p^2 < 2^24 from a
  table of largest prime factors and enumerates the rest with an explicit stack, largest prime
  first; it tests neighbours by trial division up to min(p, 1000), a Baillie-PSW test (no composite
  below 2^64 passes it) and Pollard's rho with Floyd's cycle test; it reads the runs off as the
  connected pieces of the union of the windows, and it reports every kept window (length at least
  MINLEN) that overlaps the piece built so far without lying inside it, or that touches it end to
  end (step 3 says this never happens; it reported none).
- `sieve.c` uses no primality test and no factoring routine: a segmented sieve divides every
  integer in the range by every prime up to the square root of the range end, which gives P(k) for
  every k directly; it marks W(m) around every square point and reads off the runs. It stops with an
  error if any window or run reaches its 64-integer margin, so a completed run also proves that no
  window is that long.

These three all rest on the reduction above, so a mistake in it could make them agree on a wrong
answer. Two further checks do not use it:

- `direct.c`, written during the independent review, tests every interval of up to 128 consecutive
  integers against the definition itself (it tracks the largest prime of the product and that
  prime's exponent as the interval grows), with P(k) from its own segmented sieve. It does not
  enumerate square points or build windows; it only looks ahead for a k with P(k)^2 | k to decide
  when no longer interval from the same start can be bad.
- `check_definition.gp` does the same in PARI/GP, using `factor()`, around each of the five runs
  and around 2 x 10^13; `check_definition_2e14.gp` around the run of 16, the seven runs of 14 below
  2 x 10^14 and around 2 x 10^14; and `check_all_runs.gp` around every one of the 616 runs of
  length at least 10 that the enumerations list below 2 x 10^14.

`check_runs.gp` is a local PARI/GP check of the windows around each of the five runs.

## Verification

- `cand.c` and `cand2.c` give the same first run and the same count of maximal runs for every length
  from 1 to 10 below 4928180500, which reproduces a(0) to a(8) and gives a(9); `sieve.c` gives the
  same first runs for lengths 1 to 13 below 5 x 10^11 (`logs/sieve_5e11.txt`). The members that
  `sieve.c` and `cand2.c` produce agree term for term with the 10,000-term b-file of A388654, up to
  1014049 (`logs/small_checks.txt`).
- Up to 5 x 10^11, the three programs list the same 25 runs of length at least 10 and count the same
  112,277,150 square points (`logs/cand_5e11_min10.txt`, `logs/cand2_5e11_min10.txt`,
  `logs/sieve_5e11.txt`).
- Up to 2 x 10^13, `cand.c` and `cand2.c` list the same 182 runs of length at least 10 and count the
  same 1,773,455,323 square points (`logs/cand_2e13_min10.txt`, `logs/cand2_2e13_min10.txt`).
- Up to 2 x 10^14, `cand.c` and `cand2.c` list the same 616 runs of length at least 10 and count the
  same 10,116,434,980 square points (`logs/cand_2e14_min10.txt`, `logs/cand2_2e14_min10.txt`).
- `sieve.c` is running over (5 x 10^11, 2 x 10^13] in 78 pieces (`logs/sieve-chunks/`,
  `merge_sieve.py` compares them with the enumerations); every finished piece lists exactly the runs
  of the two enumerations. This check is not finished and nothing above relies on it.
- `direct.c` finds exactly the same runs as `cand.c` below 4928180500 (a(9)), below 40533366300
  (a(10)) and below 436502026500 (a(11) and a(12)), with the same first run and count for every run
  length and the same number of square points, 101,506,569 at the last bound, and no flags
  (`logs/direct_4928180500.txt`, `logs/direct_40533366300.txt`, `logs/direct_436502026500.txt`; the
  matching `cand.c` runs are in `logs/small_checks.txt` and `logs/cand_436502026500_min1.txt`). So
  a(9) to a(12) are also confirmed without the reduction.
- `check_definition.gp` confirms from the definition that each of the five runs is a maximal run of
  the stated length and that 2 x 10^13 is a run by itself (`logs/check_definition.txt`);
  `check_runs.gp` agrees (`logs/check_runs.txt`). `check_definition_2e14.gp` does the same for the
  run of 16, for the seven runs of 14 below 2 x 10^14, and for 2 x 10^14
  (`logs/check_definition_2e14.txt`), and `check_all_runs.gp` confirms that each of the 616 listed
  runs is a maximal run of exactly its listed length, 408, 137, 48, 15, 7, 0 and 1 of lengths 10
  to 16 (`logs/check_all_runs.txt`). So every run the enumerations report is real; that none is
  missing rests on the two enumerations, which agree.
- `test_cand2.c` checks the arithmetic of `cand2.c` on its own: its Baillie-PSW test against a sieve
  to 3 x 10^7 and against a 12-base Miller-Rabin test on 832,745 random integers below 2^62, its
  rejection of composites that fool weaker tests (strong pseudoprimes to base 2, strong Lucas
  pseudoprimes, Carmichael numbers), its factoring on 200,000 semiprimes, and its smoothness test
  against plain trial division on 400,000 cases (`logs/test_cand2.txt`).

All of these programs were written with the help of the same language model within two days; the
independence is in code, algorithms and method (the sieve shares no arithmetic with the
enumerations, and `direct.c` does not use the reduction), not in authorship.

## Files

- `cand.c`, `cand2.c`, `sieve.c`: the three programs. `direct.c`: the check from the definition.
  `check_definition.gp`, `check_definition_2e14.gp`, `check_all_runs.gp` (with `runs_2e14.gp`, the
  616 runs taken from `logs/cand_2e14_min10.txt`), `check_runs.gp`: the PARI/GP checks. `test_cand2.c`: unit checks for
  `cand2.c`. `check_small.sh`: the small-range checks. `run_sieve_chunks.sh`, `merge_sieve.py`: the
  restartable driver for the sieve pieces and the comparison.
- `logs/`: every run quoted above; the `.time` files hold the timings where they were recorded.
- `oeis/b388654.txt`: the A388654 b-file as fetched on 11 Oct 2026.
- `REVIEW.md`: the independent review and how each of its findings was handled.

## Reproduce

    cc -O3 -o cand cand.c -lpthread
    cc -O3 -o cand2 cand2.c -lpthread
    cc -O3 -o sieve sieve.c -lpthread
    ./cand 20000000000000 10 8        # about 7 minutes on 8 threads
    ./cand2 20000000000000 10 8
    ./cand 200000000000000 10 8       # 2.5 hours on 8 threads shared with other jobs
    ./cand2 200000000000000 10 8
    ./sieve 1 500000000001 8          # about 25 minutes; then ./run_sieve_chunks.sh for the rest
    cc -O3 -o direct direct.c -lpthread
    ./direct 40533366300 2            # about 20 minutes on 2 threads
    ./direct 436502026500 3           # 2.4 hours on 3 threads shared with other jobs
    gp -q check_definition.gp
    gp -q check_definition_2e14.gp
    awk 'BEGIN{printf "R = ["} $1=="RUN"{if(n++)printf ","; printf "[%s,%s,%s]",$2,$3,$5} END{print "];"}' \
        logs/cand_2e14_min10.txt > runs_2e14.gp
    gp -q check_all_runs.gp
    gp -q check_runs.gp
    ./check_small.sh

Runs made 11 and 12 Oct 2026 on an Apple M5 laptop. Programs written and run with the assistance of
a large language model; every number above comes from the executed runs.
