# Review of the e1206 computation

A separate fresh-context review was asked to find any way the 300 values in `../b1206.txt` could be
wrong. Verdict: all 300 values correct; the required edits (to the README, and a coverage check in
`verify_sets.py`) are made. Besides checking the programs and logs in the parent folder, it
recomputed a(n) by other routes, each written from the definition, with the collisions taken from
all pair sums x^3 + y^3 with x <= y (x = y included) and no reduction to 4-sets assumed:

- `oracle_def.c brute`: for n <= 54, tries every removal set of each size in turn and checks the
  rest by computing all its pair sums (`brute54.txt`, 54 of 54 agree).
- `oracle_def.c bb`: an exact combinatorial branch and bound, each n from scratch, for n <= 196
  (`bb_1_160.txt`, `bb_161_300.txt`, 196 of 196 agree).
- `make_cnf.py`, `certify.sh`: at the 22 values of n up to 134 where a(n) = a(n - 1), a CNF
  stating that some Sidon subset has b(n) + 1 elements is unsatisfiable (kissat, with the proof
  checked by drat-trim: "s VERIFIED"), and the one for b(n) elements is satisfiable with a model
  that `check_model.py` checks from the definition (`certify_12_134.txt`). `test_card.py` tests the
  cardinality encoding exhaustively (9,212 assignments, `test_card.log`).
- `direct_model.py highs`: HiGHS on a different model (maximize |A| with one constraint per sum
  value, relative gap 0) at n = 120, 150, 200, 250, 300 gives 100, 126, 166, 204, 244, with each
  optimal set checked from the definition (`direct_highs.txt`).
- `enum_sums.py`: the reduction itself. Up to 300 there are 258 sums with two representations,
  none with three, none involving x = y, and no two colliding pairs share an element
  (`enum_sums.log`).

The exact branch and bound in the parent folder (`exact_bb.py`) and the direct check of the
deletion sets (`check_witnesses.py`) also come from this review.
