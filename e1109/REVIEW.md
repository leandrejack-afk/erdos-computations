# Adversarial review of e1109 (Erdős Problem 1109, f(N) for N <= 3000)

VERDICT: SAFE WITH EDITS (see the list at the end; the mathematics checks out,
the comment overstates the state of the second-solver evidence at the time of
review and misquotes one numeric range)

Reviewer: independent reviewer, fresh context, 2026-09-03 16:57 to 17:30 local.
Nothing below is taken on the implementer's word; every check was re-run with
the reviewer's own scripts in the session scratchpad
(rv_witness.py, rv_clique.c, rv_boundaries*.sh). Commands and observed output
are quoted so each line can be re-run.

## Step 1: problem statement and convention

Fetched https://www.erdosproblems.com/1109 through tools/erdos.py: "Let f(N)
be the size of the largest subset A of {1,...,N} such that every n in A+A is
squarefree." Standard sumset, so a+a is included. The page links A392164 and
A392165.

Fetched both OEIS entries live (curl, fmt=json). A392164's Python program
takes vertices `v = [e ... if is_squarefree(2*e)]` and the Mathematica program
uses `SquareFreeQ[2*#]`, so the OEIS convention includes a+a; A392165's
program iterates odd squarefree n (`count(1,2)`). A392165 data (39 terms, last
1103, keyword "more"), A392164 b-file by W. A. Carney n = 1..700. The forum
comment (AlexisOlson, N <= 250) uses the same convention ("odd squarefree
integers a <= N"). Site statement and OEIS convention match, and the delivered
files use it. Confirmed.

Live b-file (https://oeis.org/A392164/b392164.txt, 700 lines) is byte
identical to the local b392164.txt and to the first 700 lines of
b392164_ext.txt. A392165 live data equals b392165_ext.txt for k = 1..39 and
equals the k = 1..39 list in results.md.

## Step 2: independent witness verification (rv_witness.py)

Own squarefree sieve (multiples of p^2 marked) cross-validated against
sympy.factorint on 1..6002 (agree, 3648 squarefree). Each set: k distinct
integers in [1, N], max = N, and every a+b with a <= b (so 2a included)
squarefree.

    f3000.records: 65 sets checked, failures: 0
    results.md: parsed 26 sets, k = 40..65 ; failures: 0
    (each results.md set equals the f3000.records set; the stated class mod 4 is correct for each)
    results.md new table == b392165_ext k=40..65: True
    comment.txt a(40..65) == b392165_ext: True
    f3000.tsv == b392164_ext: True ; f3000.tsv == step function of b392165_ext: True
    selected: f(1500)=44, f(2000)=54, f(2500)=61, f(3000)=65

So every LOWER bound in the deliverables is proved by an explicit set. No
failure of any kind.

## Step 3: independent upper bounds (rv_clique.c, own bitset MCQ)

Own solver: Tomita-style branch and bound, non-increasing-degree vertex order
(bbmc uses degeneracy order, solver2 numeric order), greedy colouring bound,
optional case split on residue pairs {s, q-s} mod 9 and 25 (re-derived: such
a pair sums to 0 mod p^2, so no clique meets both classes). Shares no code
with bbmc.c or solver2.c.

Validation: for N = 1..700 the max over the two classes matched the OEIS
b-file on all 700 values (0 mismatches, slowest run 0.02 s). Plain mode (no
split) and split mode agree on N = 1254 (both classes) and N = 1276 class 1
(the plain runs at 3 to 4 s each; the remaining plain runs were cut by a tool
timeout, not by a disagreement).

Boundaries, exact clique number per class (incumbent 0, exhaustive), claim is
f(a(k+1)-1) = k, i.e. max over classes equals k:

    k   N     omega_1 omega_3  bbmc log values  verdict
    39  1254  39      39       39 39            OK
    40  1276  40      40       40 40            OK   <- requested small record
    41  1292  41      41       41 41            OK   <- requested small record
    42  1334  42      42       42 42            OK
    43  1406  43      43       43 43            OK
    44  1508  44      44       44 44            OK
    45  1534  45      45       45 45            OK
    46  1594  46      46       46 46            OK
    47  1706  47      47       47 47            OK
    48  1734  47      48       47 48            OK
    49  1778  48      49       48 49            OK
    50  1834  49      50       49 50            OK
    51  1918  51      51       51 51            OK   <- larger record, 1.7 s

Every per-class value also equals the per-class running value read off the
raw incremental logs inc_r1_3000.txt / inc_r3_3000.txt (26 of 26 agree,
including the three N where class 1 is strictly below class 3). Runs for
k = 52..65 (N up to 3000) were launched in the background and are reported in
the addendum at the end of this file.

## Step 4: claims vs evidence files

Re-ran the implementer's own evidence commands:

    python3 merge.py 700 f700_rerun inc_r1_700.txt inc_r3_700.txt --bfile b392164.txt
      -> "bfile compared: 700 values, 0 mismatches"; output tsv identical to f700.tsv
    .venv/bin/python check_witness.py f3000.records f3000.tsv
      -> "witnesses checked: 65, failures: 0" / "table checked to N=3000: 0 problems"
    f3000.tsv[1..700] == f700.tsv ; f3000.tsv[1..1103] == s2_1103.tsv (solver2 per-N)
    s2_1103.records (k,N) == f3000.records k<=39 ; both s2_r*_1103.txt end with DONE
    inc_r1_3000.txt: 605 decisions, 285 s, slowest N=2985 17.9 s ; inc_r3_3000.txt: 609, 370 s, slowest N=2967 19.8 s
      (matches the runtime table in results.md); the decided N are exactly all odd
      squarefree N in each class; per-class values are monotone with steps of at most 1;
      max of the two classes equals f3000.tsv for every N.
    class counts on [1104,3000] from the logs: class 3 alone 887, class 1 alone 296, tie 714 (matches results.md).

bbmc.c and solver2.c read as correct: incremental mode decides "is there an
L-clique in the neighbourhood of N" with L = f_r(N-1), which is a valid
induction since f_r rises by at most 1; full mode with incumbent k is
exhaustive (no early stop when target = 0), so "best=k" plus DONE means no
(k+1)-clique. boundary_check.py requires both.

Gaps between the text and the files AT THE TIME OF REVIEW (17:07):

- verify.md table row "bbmc full-mode boundary proofs, k = 1..65, both
  classes, all rows OK": boundary_bbmc.tsv had rows through k = 63 only
  (k = 64 and 65 still running, `./bbmc full 1 2992 64 3,5` was live in ps).
  Every row present says OK.
- verify.md row "solver2 full-mode boundary proofs, k = 39..65": the files
  held k = 39..55 (class 1) and k = 39..54 (class 3), all OK, with the
  processes still running. verify.md's own caveat ("any row missing ... has
  not yet been re-proved by solver2") covers this, but comment.txt does not
  (see step 5).
- verify.md preamble says every claim has raw output in the directory. Two
  rows do not: the kissat results (210 s UNSAT at N = 1103, 409 s SAT at
  N = 2000) have no CNF or solver log on disk, only a "rerun" recipe, and the
  "check_witness.py output" row has no saved output (I reproduced it above,
  so the claim is true, but the preamble is not).
- STATUS.md is a stale interim note ("single program", "not yet re-proved")
  and contradicts results.md; it is not a posting file.

## Step 5: comment.txt audit

- Em dashes: none in comment.txt, results.md or verify.md (scanned for
  U+2014, U+2013, curly quotes; the only non-ASCII characters are in the
  names Östergård and Erdős). PASS.
- Convention statement "sumset with a+a included, as in OEIS": correct.
- a(40..65) list, f(1500), f(2000), f(2500), f(3000): all correct (step 2).
- "reproduce the b-file and A392165": correct (step 4).
- "every record boundary was re-proved by an independently written second
  solver": NOT TRUE as of 17:07; solver2 has re-proved k = 39..55 (class 1)
  and 39..54 (class 3). The same applies to the AI-disclosure sentence
  "every value was cross-checked with two independent programs". Either wait
  for boundary_solver2_r1.tsv and boundary_solver2_r3.tsv to reach k = 65
  with every row OK, or state the range actually covered.
- Structure claims, tested on all 65 listed sets: for every listed set with
  k >= 30 the residues mod 9 are exactly one from each pair {1,8},{2,7},
  {3,6},{4,5}; mod 25 exactly one from each of the 12 pairs; mod 49 between
  17 and 24 residues (min 17 at k = 40, 24 attained many times), never both
  members of a pair. Both classes mod 4 occur (18 sets in class 3, 8 in
  class 1 among k = 40..65). PASS on the facts, two wording issues:
  (a) "each record set" reads as a property of all extremal sets; only the one
  set found per record was examined, so say "the extremal set found for each
  record"; (b) "17 to 24 of the 24 admissible residues mod 49" miscounts:
  there are 48 nonzero residues mod 49 in 24 pairs, and a set can use at most
  one per pair; say "17 to 24 residues mod 49, never both members of a pair
  {s, 49-s}, so at most 24".
- Growth numbers: f/(log N)^2 = 0.795 at 1103 and 1.014 at 3000 (quoted 0.80
  and 1.01, fine). "f(N)/sqrt(N) stays in [1.07, 1.22] on [500, 3000]" is
  WRONG at both ends: the minimum is 1.057 (N = 516, f = 24) and the maximum
  is 1.231 (N = 2455, f = 61); 20 values lie below 1.07 and the values just
  above 2455 exceed 1.22. Say "between 1.05 and 1.24". results.md repeats the
  same range and also misdescribes the forum example: {5,17,...,101} is a
  9-term progression with difference 12, not "a 12-term AP".

## Findings

1. [MAJOR, wording] comment.txt claims every record boundary was re-proved by
   the second solver; at review time the evidence covers k <= 55 (class 1)
   and k <= 54 (class 3). Repro: `tail -1 boundary_solver2_r1.tsv` ->
   "55 2140 1 ..."; `tail -1 boundary_solver2_r3.tsv` -> "54 2070 3 ...".
   Fix: post only after both files reach k = 65 with all rows OK, or reword.
2. [MINOR] f(N)/sqrt(N) range misquoted ([1.07, 1.22]; actual [1.057, 1.231])
   in comment.txt and results.md. Repro: rv_witness.py prints
   "f/sqrt(N) on [500,3000]: min 1.057 max 1.231".
3. [MINOR] results.md calls the forum example "a 12-term AP"; it is a 9-term
   AP with common difference 12.
4. [MINOR] comment.txt "24 admissible residues mod 49" miscounts (48 residues
   in 24 pairs); "each record set" should be "the extremal set found for each
   record".
5. [MINOR] verify.md preamble ("raw output is in this directory") is not met
   for the kissat rows and the check_witness.py row.
6. [NIT] STATUS.md is stale relative to results.md.

No BLOCKER. No value, set or index in the deliverables is wrong.

## Attacks attempted

- Own witness checker with two squarefree tests on all 65 record sets and the
  26 sets in results.md (including a+a and max = N): all pass.
- Own exact clique solver validated on 700 OEIS values, then run exhaustively
  at 13 record boundaries in both classes (26 clique numbers): all equal the
  claimed values; per-class values equal the raw log values.
- Plain versus split mode of my own solver: agree where both finished.
- Diffed live OEIS b-file and A392165 data against every local copy and
  every list in the three posting files.
- Re-derived the step function from the records and compared with the per-N
  table; checked the decided N set, monotonicity, step size per class, class
  count claims, runtime table claims from the raw logs.
- Re-ran merge.py and check_witness.py; diffed f700 / s2_1103 / f3000.
- Read bbmc.c and solver2.c for the induction and exhaustiveness arguments.
- Tested all structural residue claims and the AP claim on all listed sets.
- Scanned for em dashes and non-ASCII.

## Required edits before posting

1. Do not post comment.txt until boundary_solver2_r1.tsv and
   boundary_solver2_r3.tsv both contain rows k = 39..65 all OK (and
   boundary_bbmc.tsv rows k = 64, 65), or change "every record boundary was
   re-proved by an independently written second solver" and "every value was
   cross-checked with two independent programs" to the range actually
   covered.
2. Replace "[1.07, 1.22]" by "[1.05, 1.24]" (or "between 1.05 and 1.24") in
   comment.txt and results.md.
3. comment.txt: "each record set with k >= 30" -> "the extremal set found for
   each record with k >= 30"; "17 to 24 of the 24 admissible residues mod 49"
   -> "17 to 24 residues mod 49, never both of a pair {s, 49-s}".
4. results.md: "a 12-term AP" -> "a 9-term AP with difference 12".
5. verify.md: either save the kissat and check_witness outputs to files or
   soften the preamble sentence.

## Addendum: own solver at k = 52..65

(pending, background run rv_boundaries2.sh; filled in below when done)
