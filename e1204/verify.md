# Verification record for S(k) = min sum of an admissible k-tuple (Erdős #1204, B(k) = S(k)/k)

All commands run from this folder on 2026-09-03. `crosscheck.py` re-parses every file in `runs/`,
re-checks every listed tuple (length k, first element 0, sum equal to the reported S(k), admissible
for every prime up to k) and compares every pair of runs on the k they share. Its full output at the
end of the session is in `runs/crosscheck.txt`; the summary below is copied from it.

## Methods

| method | file | what it is | data it depends on |
|---|---|---|---|
| M1 element branch-and-bound | `smin.c` (`-s 0`) | depth-first over increasing tuples, residue-coverage pruning, bounds LB1 (smallest non-forbidden integers) and LB2 (tail is an admissible m-tuple, uses its own S(m)) | none |
| M1' same with A008407 bounds | `smin.c -A b008407.txt` | M1 plus elementwise bounds a_j >= A(j) | A008407 (OEIS, terms confirmed minimal for n <= 342) |
| M1' single-k mode (in `smin.c`, not used for this record: the runs were stopped at the lead's request before any finished) | `smin K -k K -S table -A ... -inc tuple` | same search for one k with LB2 from a verified table | A008407 and the table |
| M4 Python sieve search (third implementation of M2's method, partial) | `smin_short.py` via `runs/short-run.py` | same algorithm as rvs.c written separately in Python | none |
| M2 missed-class search | `rvs.c` | choose the missed class of each odd prime, bound = sum of the k smallest survivors of the partial sieve | none |
| M3 MaxSAT from the definition | `verify_sat.py` | python-sat RC2; all primes including 2; selector per (prime, class); cardinality k; weight n on x_n; window inductive from its own S(k-1) | none (window uses its own previous value; `--trivial` gives a fully self-contained window, checked to k = 12) |

M1 and M2 keep ties, so their tuple lists are the complete sets of optimal tuples. M3's first run
(`runs/sat-1-60.txt`) enumerated every optimal model and lists tuples with repeats (same tuple, other
selector values); `crosscheck.py` compares sets, so the repeats do not matter.

## Agreement (from `crosscheck.py`)

| pair | k range shared | values | optimal tuple sets |
|---|---|---|---|
| M1 vs M2 (`r500-s0.txt` vs `rvs-400.txt`) | 1..147 | agree | agree |
| M1' vs M2 (`r342-A.txt` vs `rvs-400.txt`) | 1..144 | agree | agree |
| M4 vs M2 (`short-run.txt` vs `rvs-400.txt`) | 1..102 | agree | (values only) |
| M3 vs M1, M2 (`sat-1-60.txt`) | 1..38 | agree | agree (one optimal tuple each) |
| M3 trivial window vs M1 (`sat-1-12.txt`) | 1..12 | agree | agree |
| M1 pruning variants (`-s 0`, `-s 3`, `-A`, `-slack 20`) | 1..40 | agree | agree |

No value conflict at any k. Largest number of distinct optimal tuples at any k in the tie-keeping
runs: 2, and only at k = 109 (all three C runs list the same two tuples there); the optimum is unique
for every other computed k.

Coverage summary:
- k <= 38: three methods (M1, M2, M3) agree on value and on the unique optimal tuple.
- 39 <= k <= 147: two methods with no external data (M1, M2) agree on value and tuple set; M1' agrees
  as well for k <= 144 (still running).
- 148 <= k <= 165: M2 alone (an exact search, but single-implementation); these terms are in
  `S-all-runs.txt`, not in the OEIS b-file. Values (copied by script from that file): 148:62114, 149:63044, 150:63976, 151:64926, 152:65880, 153:66840, 154:67802, 155:68768, 156:69742, 157:70728, 158:71718, 159:72710, 160:73712, 161:74726, 162:75742, 163:76788, 164:77838, 165:78890.

## Other checks

- `greedy.py 2000` equals the A135311 b-file for n = 1..2000 (`diff` empty).
- `smin_short.py 40` (the OEIS PROG program) prints the same 40 terms as the C runs in 0.45 s.
- OEIS term search for `0,2,8,16,28,46,66,92,122,154,190,232,280,330` and for the halved sequence
  returns `null` (no match); the API returns a list for known terms (checked with A135311's terms).

## Facts the drafts rely on

- S(k) = greedy sum for k <= 92; S(93) = 22474 < 22480; S(k) < greedy sum for every k from 93 to
  165 (`crosscheck.py` prints the list of k >= 93 with equality: none).
- B(k)/(k log k): 0.6276 (k = 20), 0.5944 (40), 0.5722 (100), 0.5674 (147).
- B(k)/A(k) near 0.48 for 40 <= k <= 147 (from `runs/table-full.txt`).
- Two optimal tuples at k = 109 (sum 31862), one optimal tuple at every other k <= 165.

## Commands

```
cc -O2 -Wall -Wextra -o smin smin.c && ./smin 147 -s 0 > runs/r500-s0.txt        # M1 (hours at the top end)
./smin 342 -A ../../scratch/b008407.txt > runs/r342-A.txt                        # M1'
cc -O2 -Wall -Wextra -o rvs rvs.c && ./rvs 400 > runs/rvs-400.txt                # M2
../../.venv/bin/python verify_sat.py 1 38 --all > runs/sat-1-60.txt              # M3 with uniqueness
../../.venv/bin/python verify_sat.py 1 12 --trivial > runs/sat-1-12.txt          # M3 self-contained window
../../.venv/bin/python greedy.py 2000 > runs/greedy-2000.txt && head -2000 ../../scratch/b135311.txt | diff - runs/greedy-2000.txt
../../.venv/bin/python crosscheck.py --table-every 10 --bfile-max 147 > runs/crosscheck.txt
```
Single-k check of an already verified k: `./smin 120 -k 120 -S b-file-S.txt -A ../../scratch/b008407.txt -q`
prints `k=120 S=39206 ... nopt=1`, the same value and tuple count as the sequential run.
Quick re-run for a gate: `./smin 60 -s 0 -q` and `./rvs 60 -q` both finish in well under a second
and must print identical S values; `verify_sat.py 1 16` takes about a minute.
