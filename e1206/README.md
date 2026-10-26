# Erdős Problem 1206: the largest Sidon subset of the first n cubes

Problem: https://www.erdosproblems.com/1206. Let a(n) be the size of a largest subset A of
{1, ..., n} such that the cubes {x^3 : x in A} form a Sidon set (all sums x^3 + y^3 with x <= y in
A are distinct). Erdős asked, in effect, whether a(n) grows linearly; the page suggests an OEIS
sequence as "possible". No OEIS entry for this sequence existed on 26 Oct 2026 (searched by name
and by terms). Related entries: A062292 (the greedy Sidon sequence of cubes) and A390813 (the
analogue for squares, Erdős Problem 773).

## Result

a(n) for n = 1, ..., 300, computed and certified: each lower bound by an explicit set checked from
the definition, each upper bound by a branch and bound whose every leaf is checked in exact
rational arithmetic. The first terms are

1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 22, 23, 24, 25,
26, 27, 28, 29, 29, 30, 30, 31, 32, 33, 34, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 45,
46, 46, 47, 47, 48, 49, 50, 51, 51

(the first loss is at n = 12, from 1729 = 1^3 + 12^3 = 9^3 + 10^3), and

| n | 50 | 100 | 150 | 200 | 250 | 300 |
|---|---|---|---|---|---|---|
| a(n) | 45 | 84 | 126 | 166 | 204 | 244 |
| a(n)/n | 0.900 | 0.840 | 0.840 | 0.830 | 0.816 | 0.813 |

All values are in `b1206.txt` ("n a(n)").

## Method

Two different pairs can have the same sum of cubes only if their four numbers are distinct:
a^3 + b^3 = a^3 + c^3 forces b = c, and x^3 + x^3 = c^3 + d^3 with c < d would be a solution of
x^3 + y^3 = 2z^3 in positive integers with x != y, which does not exist (`check_doubles.py` also
confirms that no 2a^3 with a <= 3000 is a sum of two distinct cubes up to 3000). So a subset is
admissible exactly when it contains none of the 4-sets {a, b, c, d} with a^3 + b^3 = c^3 + d^3, and
a(n) = n - tau(n), where tau(n) is the least number of elements of {1, ..., n} that meet every such
4-set with elements up to n (a minimum hitting set). Up to 300 there are 258 of these 4-sets.

Adding n only adds 4-sets that contain n, so tau(n - 1) <= tau(n) <= tau(n - 1) + 1.
`sidon_cpsat.py` walks through n = 1, ..., 300 keeping an optimal deletion set. When n brings new
4-sets and the current set misses one of them, it asks OR-tools CP-SAT whether some tau(n - 1)
elements meet every 4-set up to n; if none exists, tau(n) = tau(n - 1) + 1, and the old set plus n
is optimal.

## Checks

1. Lower bound, a(n) >= b(n). The deletion set for every n is logged in `c300.err`.
   `check_witnesses.py` removes it from {1, ..., n} and checks directly, with exact integers, that
   all sums x^3 + y^3 (x <= y, x = y included) of what remains are distinct and that b(n) numbers
   remain; it uses no reduction to 4-sets. `verify_sets.py` checks the same sets against the list
   of 4-sets and checks that every n from 1 to 300 appears exactly once.
2. Upper bound, a(n) <= b(n). `exact_bb.py n` proves that no n - b(n) - 1 deletions leave a Sidon
   set. It is an LP-based branch and bound over keep and remove decisions, with the collisions
   taken from all pair sums (x = y included). HiGHS only proposes LP solutions. A branch is closed
   only by an exact argument: a collision whose elements are all kept, more forced removals than
   the budget allows, or a fractional packing of collisions, rebuilt as exact fractions and checked,
   whose total exceeds the budget (by weak duality, every set of removals that breaks the remaining
   collisions is at least that large). It is run at the 56 values of n with b(n) = b(n - 1); since
   a(n) <= a(m) + (n - m) for m < n, this covers every n up to 300. `combine.py` checks that
   reasoning against `exact_bb.log` and the b-file (`combine.log`).
3. Independent solver runs. `sidon_ilp.py` recomputes tau(n) from scratch at every n with the
   HiGHS integer programming solver (floating point, relative gap 0), finding the 4-sets by another
   route (sorting all pair sums with numpy); it agrees with CP-SAT at every n from 1 to 300
   (`i300.txt`). An incremental SAT version (`sidon_sat.py`, CaDiCaL with a totalizer) agrees on
   n = 1..191 (`s300.txt`; that run was stopped there), and a MaxSAT version (`sidon_maxsat.py`,
   RC2) at n = 12, 24, 50, 100, 150 and 200 (`maxsat.log`).
4. Review. A separate fresh-context review (`review/`) found all 300 values correct and recomputed
   them by other routes written from the definition: a brute force over all removal sets for
   n <= 54, its own exact branch and bound for n <= 196, SAT proofs checked by drat-trim at the 22
   values of n up to 134 with a(n) = a(n - 1), and HiGHS on a different model at five values of n.

## Sets that must contain n^3

An OEIS editor asked about the variant c(n), the size of a largest Sidon subset of the first n
cubes that contains n^3. It carries more information, since a(n) = max(c(1), ..., c(n)). For every
n <= 300, c(n) = a(n), except at n = 80, 108, 156 and 238, where c(n) = a(n) - 1. All values are in
`contain1206.txt` ("n c(n)").

1. `contain_ilp.py` computes c(n) at every n with HiGHS, with n fixed as kept, and logs a removal
   set for every n (`contain_ilp.err`).
2. Lower bound. `check_contain.py` removes each logged set from {1, ..., n} and checks from the
   definition that what remains contains n, is a Sidon set (all pair sums, x = y included, exact
   integers) and has c(n) elements (`check_contain.log`).
3. Upper bound. c(n) <= a(n) always, so where c(n) = a(n) the bound is the certified one above. At
   the four exceptions, `exact_bb_contain.py` (`exact_bb.py` with n fixed as kept from the start)
   proves that no Sidon subset containing n^3 has a(n) elements, and as a control finds one with
   a(n) - 1 elements (`exact_bb_contain.log`). `contain_certify.sh` proves two of these bounds
   (n = 80, 108) a second way: the CNF of `review/make_cnf.py` plus the unit clause "n is in the
   set" is unsatisfiable for a(n) elements (kissat, with the proof checked by drat-trim:
   "s VERIFIED") and satisfiable for a(n) - 1, with the model checked from the definition
   (`contain_certify.log`). At n = 156 and 238 this SAT route is too slow (kissat had not finished
   n = 156 after 6 minutes; the review's SAT proofs for a(n) stopped at n = 134), so those two
   bounds rest on the branch and bound, with the HiGHS and CP-SAT values (steps 1 and 4) agreeing.
4. `contain_cpsat.py` recomputes c(n) from scratch at every n with OR-tools CP-SAT, finding the
   4-sets by dictionary lookup instead of sorting, and agrees at all 300 values (`contain_cpsat.txt`).

## Files

Programs: `sidon_cpsat.py`, `exact_bb.py`, `combine.py`, `check_witnesses.py`, `verify_sets.py`,
`check_doubles.py`, `sidon_ilp.py`, `sidon_sat.py`, `sidon_maxsat.py`; for c(n), `contain_ilp.py`,
`check_contain.py`, `exact_bb_contain.py`, `contain_certify.sh`, `contain_cpsat.py`.

Results and logs: `b1206.txt` ("n a(n)", n = 1..300), `c300.txt` and `c300.err` (the CP-SAT values
and deletion sets), `exact_bb.log`, `combine.log`, `check_witnesses.log`, `verify_sets.log`,
`check_doubles.log`, `i300.txt` and `i300.err`, `s300.txt`, `maxsat.log`; for c(n),
`contain1206.txt` ("n c(n)", n = 1..300), `contain_ilp.err`, `check_contain.log`,
`exact_bb_contain.log`, `contain_certify.log`, `contain_cpsat.txt`. The review's programs and logs
are in `review/`.

Python 3 with numpy, scipy, highspy, ortools and python-sat. These programs were written with the
assistance of a large language model; the results rest on the checks listed above.
