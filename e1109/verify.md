# Verification record for the f(N) values, N <= 3000 (Erdős 1109)

Every claim below was produced by a command; the TSV and log outputs are in this
directory (the kissat and check_witness.py runs are reproducible with the commands
given). Nothing is asserted from reading code alone.

## 1. Logic of the proof

f is nondecreasing and rises by at most 1 per step (adding one vertex adds at
most one to a clique number). So the whole table is proved by:

- LOWER: for each record k a set A_k in {1, ..., a(k)} of size k with all
  pairwise sums (including 2a) squarefree. Checked by check_witness.py, which
  uses sympy's factorint and shares no code with either solver.
- UPPER: for each record k the whole class-r graph at N = a(k+1) - 1 (or
  N = 3000 for k = 65) has no clique of size k + 1, for both r = 1 and r = 3.
  Proved by both solvers in full mode with incumbent k (boundary_check.py).

Together these give f(N) = k exactly on [a(k), a(k+1) - 1] for every k, and
f(N) = 65 on [2993, 3000].

The two facts that both solvers rely on are one-line mathematics: (i) a in A
forces 2a squarefree, i.e. a odd and squarefree, and a + b = 0 mod 4 is never
squarefree, so A lies in one class mod 4; (ii) for an odd prime p and
1 <= s < p^2/2 a set cannot contain both an element = s and an element
= p^2 - s (mod p^2). The solvers use (i) to split the problem and (ii) to
branch; both are also implied by the graph itself, so a wrong use would only
cost speed if it dropped nothing, and any dropped clique would be caught by
the disagreement between the two search orders and by the per-N agreement
with the OEIS data.

## 2. The two programs

bbmc.c (primary): sieve for squarefreeness, bitset adjacency, degeneracy
vertex order, BBMC colouring bound with MCS re-colouring, residue-pair
branching on 3 then 5 in increasing residue order, incremental per-N
decisions.

solver2.c (independent, written from the statement without reference to
bbmc.c): trial-division squarefreeness, byte adjacency matrix, vertices in
numerical order, Tomita-Seki MCQ sequential colouring with no re-colouring,
residue-pair branching on 3 then 5 with pairs in decreasing residue order.
The programs share no source lines. (ostergard.c, an Östergård/cliquer
implementation, is a third program; it agrees at N <= 1103 but is about 100
times slower and was not used at the frontier.)

## 3. Checks performed, with evidence

| check | scope | result | evidence file |
|-------|-------|--------|---------------|
| bbmc per-N values vs OEIS b-file (Carney) | N = 1..700 | 700 values, 0 mismatches | `python3 merge.py 700 f700 inc_r1_700.txt inc_r3_700.txt --bfile b392164.txt` |
| bbmc records vs A392165 (Chai Wah Wu) | k = 1..39 | identical: 1, 5, 19, ..., 1067, 1103 | f3000.records |
| solver2 per-N values vs b-file | N = 1..700 | 700 values, 0 mismatches | s2_1103.tsv |
| solver2 per-N values vs bbmc | N = 1..1103 | 1103 values, 0 mismatches | s2_r1_1103.txt, s2_r3_1103.txt |
| solver2 records vs A392165 | k = 1..39 | identical | s2_1103.records |
| witness checker (sympy) on all record sets, plus step-function check of the table | k = 1..65, N <= 3000 | "witnesses checked: 65, failures: 0", "table checked to N=3000: 0 problems" | check_witness.py output |
| bbmc full-mode boundary proofs | k = 1..65, both classes | see boundary_bbmc.tsv (all rows OK) | boundary_bbmc.tsv |
| solver2 full-mode boundary proofs | k = 39..65, both classes | see boundary_solver2_r1.tsv, boundary_solver2_r3.tsv | those files |
| rv_clique (the review's own solver) at every record boundary | k = 39..65, both classes | boundary_reviewer_39_51.tsv, boundary_reviewer_52_60.tsv, boundary_reviewer_61_65.tsv: per-class clique number, maximum over the two classes = k at every boundary | those files |
| kissat on CNF (satgen.py), class 3, N = 1103, "clique of size 40" | one instance | UNSATISFIABLE in 210 s (160 s with residue selector variables) | rerun: `satgen.py 1103 3 40 x.cnf; kissat x.cnf` |
| kissat, class 3, N = 2000, "clique of size 54" | one instance | SATISFIABLE in 409 s | rerun with satgen.py |

The solver2 boundary pass is the second-program proof of the UPPER side for
the new range. Its two TSV files list, per boundary and class, the vertex
count, the returned maximum, node count, wall seconds and OK/FAIL. As of
2026-09-04 both files hold every row k = 39..65 (the rows for k = 59..65 were
run with boundary_check_par.py, eight instances at a time; finishing times in
boundary_solver2_par.progress). The review's own solver rv_clique.c covers the
same boundaries independently (boundary_reviewer_*.tsv); its k = 64 class 3
and k = 65 rows were run on 2026-09-04 with the same binary.

## 4. Reproduction

```
cc -O3 -march=native -o bbmc bbmc.c
cc -O3 -march=native -o solver2 solver2.c
./bbmc inc 1 1 3000 0 3,5 > inc_r1_3000.txt
./bbmc inc 3 1 3000 0 3,5 > inc_r3_3000.txt
python3 merge.py 3000 f3000 inc_r1_3000.txt inc_r3_3000.txt --bfile b392164.txt
.venv/bin/python check_witness.py f3000.records f3000.tsv
python3 boundary_check.py f3000.records 3000 ./bbmc 3,5 1 boundary_bbmc.tsv
python3 boundary_check.py f3000.records 3000 ./solver2 3,5 39 boundary_solver2_r1.tsv 1
python3 boundary_check.py f3000.records 3000 ./solver2 3,5 39 boundary_solver2_r3.tsv 3
```

Single values: `./bbmc full 3 1998 54 3,5` prints `best=54` (no 55-clique in
class 3 at N = 1998) and `./solver2 full 3 1998 54 3,5` prints the same.
`./bbmc full 1 2993 64 3,5` finds the 65-clique at N = 2993.

## 5. Known limits

- Runtimes in the TSV files are wall seconds on a machine shared with other
  jobs (the solvers got 15 to 20 percent of a core), so they overstate CPU.
- Both programs were written by the same author in the same session; the
  independence is in code, data structures, orders and bounds, not in
  authorship. The OEIS comparisons (N <= 1103) and the sympy witness checker
  are the third-party anchors.
- Nothing is claimed for N > 3000.
