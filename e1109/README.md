# Erdős Problem 1109: exact values of f(N) for N <= 3000

Problem: https://www.erdosproblems.com/1109. Let f(N) be the size of the largest A in {1, ..., N}
such that every element of A + A is squarefree. Estimate f(N); in particular, is f(N) <= N^{o(1)},
or even f(N) <= (log N)^{O(1)}? Here A + A is the full sumset, so a + a = 2a is included. This is the
convention of OEIS A392164 and A392165, of their programs, and of the forum comment by AlexisOlson.
It forces every element of A to be odd and squarefree, and since a + b is never 0 mod 4, A lies in a
single class mod 4.

## What was already known

The problem page records the bounds log N << f(N) << N^{3/4} log N (Erdős and Sárközy [ErSa87]),
an alternative proof of the lower bound by Gyarmati [Gy01], and Konyagin's improvement [Ko04] to
log log N (log N)^2 << f(N) << N^{11/15 + o(1)}.

Exact data: AlexisOlson computed f(N) for 1 <= N <= 250 on the forum thread (04 Dec 2025), and
Terence Tao suggested submitting the transition points to the OEIS. OEIS A392164 (f(N)) has a b-file
to N = 700 by William Alexander Carney, and A392165 (least N with f(N) = k) has 39 terms, the last
a(39) = 1103, by Chai Wah Wu. Since f is nondecreasing and rises by at most 1 per step, those two
entries pin f(N) exactly for every N <= 1103. Nothing beyond N = 1103 was on the OEIS or on the
problem page.

## Result

f(N) is now known exactly for every N <= 3000 (table in `b392164_ext.txt`, one line per N, and
`f3000.tsv`). The record indices a(k) = least N with f(N) = k reproduce A392165 for k = 1..39 and
continue with

| k | a(k) | k | a(k) | k | a(k) |
|---|------|---|------|---|------|
| 40 | 1255 | 49 | 1735 | 58 | 2287 |
| 41 | 1277 | 50 | 1779 | 59 | 2355 |
| 42 | 1293 | 51 | 1835 | 60 | 2427 |
| 43 | 1335 | 52 | 1919 | 61 | 2455 |
| 44 | 1407 | 53 | 1991 | 62 | 2627 |
| 45 | 1509 | 54 | 1999 | 63 | 2729 |
| 46 | 1535 | 55 | 2071 | 64 | 2741 |
| 47 | 1595 | 56 | 2141 | 65 | 2993 |
| 48 | 1707 | 57 | 2193 |    |      |

so f(N) = k for a(k) <= N < a(k+1) and f(N) = 65 for 2993 <= N <= 3000. Selected values: f(1500) = 44,
f(2000) = 54, f(2500) = 61, f(3000) = 65. The full record list with one extremal set per record is
`f3000.records` (`b392165_ext.txt` holds the indices alone). For a non-record N with
a(k) <= N < a(k+1) the set listed at a(k) is extremal for N as well.

Nothing is claimed for N > 3000.

## Method

Vertices are the odd squarefree a <= N in one class r mod 4, with an edge between a and b when a + b
is squarefree; f(N) is the larger of the two clique numbers. The primary solver `bbmc.c` is a
bit-parallel branch and bound with a greedy colouring bound (San Segundo's BBMC with Tomita-style
re-colouring, degeneracy vertex order), preceded by residue-pair branching: for p = 3 and p = 5 and
each pair {s, p^2 - s} of residues mod p^2, a clique cannot meet both classes (such a pair sums to
0 mod p^2), so the search branches on which class to drop and prunes each branch with the colouring
bound. This split gave a factor of about 100 at N = 2000 over plain vertex branching.

Values were produced incrementally: for each odd squarefree N the solver decides whether N lies in a
clique of size f(N-1) + 1 in the class graph (a clique search in the neighbourhood of N), which is a
valid induction because f rises by at most 1 per step, and yields f(N) and a witness at each jump.
Two passes (one per class mod 4) over N = 1..3000 took 285 s and 370 s of CPU with the slowest single
N under 20 s (`logs/inc_r1_3000.txt`, `logs/inc_r3_3000.txt`).

`solver2.c` is a second exact solver written from the statement without reference to `bbmc.c`
(trial-division squarefree test, byte adjacency matrix, numerical vertex order, Tomita and Seki's
sequential colouring without re-colouring, residue pairs taken in the opposite order). `ostergard.c`
(Östergård's algorithm) is a third solver, correct but about 100 times slower on these dense graphs,
and `satgen.py` produces a CNF for kissat, about 500 times slower still. Both programs `bbmc.c` and
`solver2.c` were written by the same author in the same session; the independence is in code, data
structures, orders and bounds, not in authorship.

## Verification

Full detail is in `verify.md`; the adversarial review is `REVIEW.md` (verdict: safe with wording
edits, which this README follows). The proof of the table has two halves. Lower bounds: for each
record k an explicit set of size k in {1, ..., a(k)} with all pairwise sums, including 2a,
squarefree. Upper bounds: at each boundary N = a(k+1) - 1 (or N = 3000 for k = 65) the whole class
graph has no clique of size k + 1, for both classes.

- Lower bounds: `check_witness.py` checks every listed set from the statement alone, with sympy's
  factorint for squarefreeness (no code shared with the solvers), and checks that the table is a
  step function consistent with the records:

      python3 check_witness.py f3000.records f3000.tsv
      witnesses checked: 65, failures: 0
      table checked to N=3000: 0 problems

- Agreement with published data: the bbmc values match all 700 values of the A392164 b-file
  (`merge.py ... --bfile b392164.txt` reports 0 mismatches) and the records reproduce A392165 term
  for term. `solver2` matches the b-file on N <= 700 and bbmc on every N <= 1103
  (`s2_1103.tsv`, `s2_1103.records`, `logs/s2_r1_1103.txt`, `logs/s2_r3_1103.txt`).

- Upper bounds re-proved in full mode (exhaustive search for a clique of size k + 1, no early stop):
  by bbmc at every record boundary for k = 1..65 in both classes (`boundary_bbmc.tsv`, every row
  OK), and by solver2 at every record boundary for k = 39..65 in both classes
  (`boundary_solver2_r1.tsv`, `boundary_solver2_r3.tsv`, every row OK). The solver2 rows for
  k = 59..65 were run on 2026-09-04 with `boundary_check_par.py`, eight instances at a time;
  `logs/boundary_solver2_par.progress` has their finishing times. Wall times for the earlier rows
  were measured on a machine shared with other jobs.

- The review (REVIEW.md) re-checked all 65 sets with its own squarefree sieve, matched the OEIS
  b-file live, and ran its own exact clique solver `rv_clique.c` (Tomita-style branch and bound,
  degree order, no code shared with `bbmc.c` or `solver2.c`; validated against all 700 values of the
  OEIS b-file) at every record boundary for k = 39..65 in both classes. `boundary_reviewer_39_51.tsv`
  gives the exact clique number per class (its verdict column compares the per-class value with k;
  the maximum over the two classes equals k in every row); `boundary_reviewer_52_60.tsv` and
  `boundary_reviewer_61_65.tsv` give the per-class clique number for the later boundaries, all <= k
  (verdict LE_k). The rows for k = 64 (class 3) and k = 65 were run on 2026-09-04 with the same
  program (`rv_boundaries2.sh` is the review's driver).

- Spot checks with kissat on `satgen.py` output: no 40-clique in the class-3 graph at N = 1103
  (UNSAT in 210 s) and a 54-clique in the class-3 graph at N = 2000 (SAT in 409 s). These two runs
  have no saved log; the recipe is in `verify.md`.

## Structural observations

- Both classes mod 4 matter: among N in [1104, 3000], class 3 alone attains f(N) for 887 values,
  class 1 alone for 296, and the two tie for 714. The record sets alternate between the classes
  (18 of the 26 new record sets are in class 3, 8 in class 1).
- The extremal set found for each record with k >= 30 uses exactly one residue from each of the
  four pairs {1,8}, {2,7}, {3,6}, {4,5} mod 9 and exactly one from each of the twelve pairs
  {s, 25-s} mod 25, so those constraints are saturated; mod 49 it uses 17 to 24 residues, never
  both members of a pair {s, 49-s}. Only the one set found per record was examined.
- The sets are not arithmetic progressions: the longest run with common difference 12 inside any
  record set with k >= 39 has length 7 (the f(101) = 9 example on the forum, a 9-term progression
  with difference 12, is a small-N phenomenon).
- Growth: f(N)/(log N)^2 keeps rising, from 0.59 at N = 250 to 0.80 at N = 1103 and 1.01 at
  N = 3000. Over 500 <= N <= 3000 the ratio f(N)/sqrt(N) stays between 1.05 and 1.24, and the local
  exponent log(f(3000)/f(1000))/log 3 is 0.56 (Konyagin's upper bound has exponent 11/15). Nothing
  asymptotic follows from such small N.

## Files

- `bbmc.c`, `solver2.c`: the two exact solvers (compile with `cc -O3 -march=native`);
  `ostergard.c`: the slower third solver; `satgen.py`: CNF generator for kissat.
- `rv_clique.c`: the review's own exact clique solver (usage in its header).
- `merge.py`: combines per-class incremental logs into f(N) and records; `check_witness.py`:
  independent checker of the extremal sets and the table; `boundary_check.py`: re-proves every
  record boundary with a given solver, `boundary_check_par.py` the same several instances at a
  time; `rv_boundaries2.sh`: the review's boundary driver; `analyze.py`: the structural summary;
  `nx_check.py`: the OEIS A392164 Python program (Chai Wah Wu) wrapped for spot checks with
  networkx.
- `f3000.tsv`, `f3000.records`, `b392164_ext.txt`, `b392165_ext.txt`: results. `s2_1103.tsv`,
  `s2_1103.records`: solver2's independent values to N = 1103. `boundary_*.tsv`: the boundary
  re-proofs, one row per boundary and class (bbmc, solver2 and the review's solver). `logs/`: the
  raw incremental solver logs (per N: running bound, vertex count, nodes, seconds, and the witness
  at each jump) and the finishing times of the parallel solver2 boundary run.
- `verify.md`: the verification record. `REVIEW.md`: the adversarial review.

## Reproduce

    cc -O3 -march=native -o bbmc bbmc.c
    cc -O3 -march=native -o solver2 solver2.c
    ./bbmc inc 1 1 3000 0 3,5 > logs/inc_r1_3000.txt
    ./bbmc inc 3 1 3000 0 3,5 > logs/inc_r3_3000.txt
    python3 merge.py 3000 f3000 logs/inc_r1_3000.txt logs/inc_r3_3000.txt --bfile b392164.txt
    python3 check_witness.py f3000.records f3000.tsv                 # needs sympy
    python3 boundary_check.py f3000.records 3000 ./bbmc 3,5 1 boundary_bbmc.tsv
    python3 boundary_check.py f3000.records 3000 ./solver2 3,5 39 boundary_solver2_r1.tsv 1
    python3 boundary_check.py f3000.records 3000 ./solver2 3,5 39 boundary_solver2_r3.tsv 3
    python3 boundary_check_par.py f3000.records 3000 ./solver2 3,5 boundary_solver2_par 8 1:65,64,63,62,61,60 3:65,64,63,62,61,60,59
    ./bbmc full 3 1998 54 3,5        # prints best=54: no 55-clique in class 3 at N = 1998
    ./solver2 full 3 1998 54 3,5     # same
    ./bbmc full 1 2993 64 3,5        # finds the 65-clique at N = 2993
    cc -O3 -march=native -o rv_clique rv_clique.c
    ./rv_clique 1 3000 0 1           # answer=65: the class-1 clique number at N = 3000
    ./rv_clique 3 3000 0 1           # answer=64

`b392164.txt` is the OEIS b-file, https://oeis.org/A392164/b392164.txt (its 700 values are the first
700 lines of `b392164_ext.txt`).

Computed 2026-09-03 and 2026-09-04 on an Apple M5 laptop. Programs written and run with the assistance of a large language model; every claim above comes from the executed runs.
