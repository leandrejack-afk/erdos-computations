# Erdős Problem 1204: the minimal sum of an admissible k-tuple, S(k) for k <= 147

Problem: https://www.erdosproblems.com/1204. A sequence of integers 0 <= a_1 < ... < a_k is
admissible if it misses at least one congruence class modulo every prime p. Let A(k) = min a_k.
Estimate A(k); in particular, is A(k) ~ k log k? Estimate B(k) = min (a_1 + ... + a_k)/k. Erdős
[Er80, p. 108] attributes the question to Elliott.

Write S(k) for the least a_1 + ... + a_k over admissible k-tuples, so B(k) = S(k)/k. This folder
computes S(k) exactly, with the complete set of optimal tuples, for k <= 147.

## What was already known

The problem page's commentary records (1/2 + o(1)) k log k <= A(k) <= (1 + o(1)) k log k, notes that
B(k) < A(k) trivially, gets B(k) <= (1/2 + o(1)) k log k from the first k primes above k and
B(k) >= (1/k) sum_{j <= k} A(j) from a_j >= A(j), and expects B(k) ~ (1/2 + o(1)) k log k. The page
(last edited 07 April 2026) had no comments, and links A008407 (A(k), the minimal diameter, with
terms confirmed minimal for k <= 342 on the OEIS), A023193 and A135311 (the greedy admissible
sequence, 10000-term b-file). No tabulation of B(k) or S(k) was on the page, and OEIS term searches
for S(k) (0, 2, 8, 16, 28, 46, 66, 92, 122, 154, 190, 232, 280, 330) and for S(k)/2 return no match,
while no sequence citing A135311 is its partial sums. An OEIS entry for S(k) has been drafted
(`oeis-draft.md`) but not submitted.

## Result

S(k) for k = 1..147 is in `b-file-S.txt` (format "k S(k)"); the first terms are

    0, 2, 8, 16, 28, 46, 66, 92, 122, 154, 190, 232, 280, 330, 386, 448, 516, 588, 666, 752, ...

and S(147) = 61190. Each of these 147 values was produced by two independent exact searches that
share no code and use no external data (`smin.c` and `rvs.c`), agreeing on the value and on the
complete set of optimal tuples; a MaxSAT model of the definition agrees for k <= 38, and a run of
`smin.c` with elementwise bounds from A008407 agrees for k <= 144.

`S-all-runs.txt` holds every k that any run reached (k <= 170 in the copy here). Its values for
k = 148 onward come from `rvs.c` alone: an exact search, but a single implementation with no second
program confirming it, so they are not in the b-file and should be treated as unconfirmed. The raw
`rvs.c` log `runs/rvs-400.txt` reaches k = 174.

Facts from the computed range:

- The greedy admissible sequence A135311 is sum-optimal for every k <= 92 and for no k from 93 to
  147: S(93) = 22474 against the greedy sum 22480 (drop 282 and 530 from the greedy 93-tuple and add
  366 and 440).
- The optimal tuple is unique for every k <= 147 except k = 109, where two tuples attain
  S(109) = 31862.
- B(k)/(k log k) decreases slowly: 0.628 (k = 20), 0.594 (40), 0.572 (100), 0.567 (147), still well
  above the expected 1/2 + o(1). B(k)/A(k) stays near 0.48 for 40 <= k <= 147.

## Method

Reductions, each one line: translating a tuple down by a_1 keeps it admissible and lowers the sum,
so a_1 = 0 at the optimum; the prime 2 then forces every a_i to be even, a_i = 2 b_i, and (b_i) must
miss a class modulo every odd prime p <= k (a k-tuple cannot cover all classes of a prime p > k), so
S(k) = 2 min sum b_i; with an incumbent I, any tuple with sum <= I has b_k <= I - S(k-1)/2 because its
first k - 1 elements form an admissible (k-1)-tuple.

- `smin.c` (M1): depth-first over increasing b-tuples, keeping the covered residues per prime;
  a prime with p - 1 covered classes forbids its last class. Bounds: the sum of the m smallest
  non-forbidden integers above the last element, and m times the next free integer plus S(m)/2 from
  the same run. Ties are kept, so every optimal tuple is listed. With `-A` it also uses elementwise
  bounds from A008407 (external data; M1').
- `rvs.c` (M2): fix for every odd prime p <= k a class r_p to miss; any k survivors of that sieve
  form an admissible tuple, every admissible tuple survives some choice, and an optimal tuple is the
  k smallest survivors of its own sieve. Depth-first over the primes in increasing order, with the
  sum of the k smallest survivors as a lower bound that only grows as more primes are sieved. Ties
  kept.
- `verify_sat.py` (M3): weighted MaxSAT (python-sat, RC2) written from the definition only, with all
  primes including 2, a selector per (prime, class), a cardinality constraint, and weight n on x_n;
  the window is inductive from its own S(k-1), or self-contained with `--trivial`.
- `smin_short.py` (M4): a 40-line Python version of the `rvs.c` method for the OEIS PROG field;
  `greedy.py`: the greedy sequence; `crosscheck.py`: re-parses every run in `runs/`, re-checks
  every listed tuple (length, first element 0, sum, admissibility for every prime up to k), compares
  every pair of runs on the k they share, prints the table and writes `b-file-S.txt`.

## Verification

Full detail is in `verify.md`; the cross-check output at the end of the session is
`runs/crosscheck.txt`. Every tuple in every run file was re-verified there (0 bad tuples), and the
pairwise comparisons show no value conflict at any k:

| pair | k range shared | values | optimal tuple sets |
|---|---|---|---|
| M1 vs M2 (`runs/r500-s0.txt` vs `runs/rvs-400.txt`) | 1..147 | agree | agree |
| M1' vs M2 (`runs/r342-A.txt` vs `runs/rvs-400.txt`) | 1..144 | agree | agree |
| M4 vs M2 (`runs/short-run.txt` vs `runs/rvs-400.txt`) | 1..102 | agree | values only |
| M3 vs M1, M2 (`runs/sat-1-60.txt`) | 1..38 | agree | agree, one tuple each |
| M3 self-contained window vs M1 (`runs/sat-1-12.txt`) | 1..12 | agree | agree |
| M1 pruning variants (`-s 0`, `-s 3`, `-A`, `-slack 20`) | 1..40 | agree | agree |

Coverage: for k <= 38 three methods agree on the value and the unique optimal tuple; for
39 <= k <= 147 the two data-free searches M1 and M2 agree on value and tuple set, with M1' agreeing
as well for k <= 144; for k >= 148 only M2 has run. Other checks: `greedy.py 2000` equals the
A135311 b-file for n = 1..2000; `smin_short.py 40` prints the same 40 terms as the C runs.

The adversarial review of this folder was still in progress when it was packaged; `REVIEW.md` is a
copy of the review record as it stood at that time and may need refreshing.

## Files

- `smin.c`, `rvs.c`: the two exact searches (compile with `cc -O2`); `verify_sat.py`: the MaxSAT
  check (needs python-sat); `smin_short.py`, `greedy.py`, `crosscheck.py` as above.
- `b-file-S.txt`: S(k) for k <= 147. `S-all-runs.txt`: every k any run reached, single-implementation
  above k = 147. `oeis-draft.md`: the unsubmitted OEIS draft.
- `runs/`: every run log (`r500-s0.txt` is M1, `r342-A.txt` is M1', `rvs-400.txt` is M2, the
  `sat-*.txt` files are M3, `short-run.txt` is M4, the `r40-*.txt` files are the pruning variants),
  `crosscheck.txt`, `table-full.txt`, and the two helper scripts `monitor.sh` and `short-run.py`
  that drove the refresh loop and the Python run.
- `verify.md`: the verification record. `REVIEW.md`: the review record (in progress).

## Reproduce

    cc -O2 -Wall -Wextra -o smin smin.c && ./smin 147 -s 0 > runs/r500-s0.txt   # M1, hours at the top end
    ./smin 342 -A b008407.txt > runs/r342-A.txt                                 # M1'
    cc -O2 -Wall -Wextra -o rvs rvs.c && ./rvs 400 > runs/rvs-400.txt           # M2
    python3 verify_sat.py 1 38 --all > runs/sat-1-60.txt                        # M3 with uniqueness
    python3 verify_sat.py 1 12 --trivial > runs/sat-1-12.txt                    # M3 self-contained window
    python3 greedy.py 2000 > runs/greedy-2000.txt                               # compare with the A135311 b-file
    python3 crosscheck.py --table-every 10 --bfile-max 147 > runs/crosscheck.txt
    ./smin 60 -s 0 -q ; ./rvs 60 -q                                             # quick check, identical S values

`b008407.txt` is the OEIS b-file of A008407 (https://oeis.org/A008407/b008407.txt). `crosscheck.py`
reads it from `../../scratch/b008407.txt`, the layout of the private working tree; point `A_FILE`
at your copy.

Computed 2026-09-03 on an Apple M5 laptop. Programs written and run with Claude Code (Claude Fable
5.1) assistance; every claim above comes from the executed runs.
