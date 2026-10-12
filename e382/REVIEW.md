# Independent review, 12 Oct 2026

An adversarial review of this folder by a separate reviewer that had not seen the work before
(also carried out with the assistance of a large language model). It read the README, the three
programs and the logs, checked the OEIS state, and wrote and ran its own program from the
definition (now `direct.c`). Line numbers refer to the files as they were at review time.

## Verdict: safe with edits

All five values are correct. The reviewer's definition-only program, which tests every interval of
up to 128 consecutive integers against the definition (no square-point enumeration, no windows),
confirms
a(9) and a(10), reproduces the 10,000-term b-file of A388654 and a(0) to a(8), and matches the
per-length run counts and the 17,548,374 square points of `cand.c` below 40533366300.

## Findings and what was done

1. **a(14) wording (should fix).** The entry's name ends "or 0 if no such interval exists", and
   whether a maximal bad interval of width 14 exists at all is open (it is the second question of
   the problem). The entry's own comments write "0 < a(9) <= ...". Write "a(14) = 0 or
   a(14) > 2*10^13". **Done** in the README and the staged OEIS edit.
2. **Pending runs (blocker until done).** The sieve pieces over (5*10^11, 2*10^13] were 3 of 78
   done, all matching; the README spoke of them as finished, and the editor note said "three
   separate programs". `cand` was being rerun with the current build. **Done:** the README now says
   the sieve check is running and that nothing relies on it; the note says two programs. The
   rerun of `cand` finished and is byte-identical to the earlier logs at 5*10^11 and 2*10^13.
3. **Attribution (should fix).** Step 1 of the reduction is the start of Section 6 and Lemma 6.1 of
   Tao, arXiv:2603.27990 (Sylvester-Schur gives p0 > H, so p0^2 divides one member and every member
   is p0-smooth; Bertrand gives H <= N). **Done:** cited in step 1, checked against the paper (v3).
4. **Independence (should fix).** `sieve.c` and `check_runs.gp` also mark the windows W(m), so all
   four original checks share the reduction. **Done:** the README says so and adds the two
   definition-only checks, `direct.c` and `check_definition.gp`.
5. **Unlogged claims (should fix).** "All three programs reproduce a(0) to a(8)" and the b-file
   agreement had no log on disk, and "with timings" was not true for every run. **Done:**
   `check_small.sh` writes `logs/small_checks.txt`; the timing sentence was corrected.
6. **Nit.** `cand2.c` compares each kept window with the piece of the union built so far, not every
   pair of windows. **Done:** reworded.
7. **Nit.** The test log says 832,745 random integers, not 830,000, and the list of pseudoprimes in
   `test_cand2.c` included numbers with factors below 50, which `bpsw()` is never given. **Done:**
   number corrected; the test now uses only numbers without factors below 50, as its comment says.
8. **Nit, publishing.** Do not publish `probe.c` (a heuristic search used for no claim), the
   binaries, the driver output, `*.part`/`*.err` leftovers or the old-build logs; use `/blob/` in
   the link to the README. **Done:** the published folder holds only the files listed in the
   README; link fixed.
9. **Nit.** `check-a388850.sh` said `check_runs.gp` works "from the definition"; it rebuilds the
   windows. **Done:** the script now uses `check_definition.gp`, which does work from the
   definition.

## Checked and found correct

- The reduction (steps 1 to 3): the Sylvester-Schur hypothesis is exactly u > v - u + 1; the
  Bertrand prime satisfies q >= u because q > v/2 >= u - 1/2; the nesting lemma follows from the
  maximality of W(m2); step 3 proves the equivalence of "maximal bad interval" and "run" that the
  OEIS name assumes.
- The earlier build of `cand.c` (Miller-Rabin with 7 bases, stopping for n >= 341550071728321)
  cannot differ from the current one on these runs: every number tested is below that bound, where
  both use the same 7 bases, which are exact there (OEIS A014233).
- `cand2.c` to 2*10^13: 1,773,455,323 square points, no violations, and the same 182 runs (with the
  same m and p) and first runs as `cand.c`.
- Code: window ends, the uniqueness of the enumeration, the maximal-window filter (including
  duplicate windows), the carried next-multiple logic and segment boundaries of `sieve.c`, the
  partial last segment, the candidate range against the marks needed, Montgomery bounds, the
  Miller-Rabin and Baillie-PSW tests, and per-thread state: no defect found.
- Attacks: the definition-only scan with three deliberately broken variants (two caught by the
  b-file and the table to 10^8; the third cannot change any output); `sieve.c` with segment sizes of
  8, 9, 13 and 18 bits and an unaligned range, identical to the definition-only program; member by
  member comparisons with the b-file; log against log at 5*10^11 and 2*10^13.
- OEIS state on 12 Oct 2026: A388850 last edited 7 Jan 2026 (revision 18), nine terms, comments as
  quoted in the README, no pending draft; the live A388654 b-file equals `oeis/b388654.txt`; the new
  values appear in no other entry. A public GitHub repository lists a(9) as an open target; no exact
  values are published anywhere.

## Second pass, 12 Oct 2026: the extension to 2*10^14

The same reviewer checked the new claims: no run of exactly 15 below 2*10^14 (so a(14) = 0 or
a(14) > 2*10^14), a(15) = 52629641927627, and no run of 17 or more. **Verdict: safe with edits;
all three claims are supported.**

Checked and found correct: the largest number reaching the Miller-Rabin test of `cand.c` stays
below 341550071728321, where its 7 bases are exact, with a margin of 1.7; the Montgomery bounds hold
in both programs; the largest p is 14142135 and every p^2 s <= X fits in 64 bits; the square-point
and window counts exceed 2^32 but are held in 64-bit counters; both `.time` files show no abort.
Both enumerations count 10,116,434,980 square points and list the same 616 runs with the same m and
p; `cand2.c` reports no violations; no window of length 15, or of 17 or more, occurs at all; every
listed m lies inside its run with p^2 | m. From the definition: 2*10^14 is a member only by itself
(2*10^14 - 1 = 23 * 8695652173913, and the largest prime of 2*10^14 + 1 is 50867), the only
non-singleton run within 300 of the run of 16 is the run itself, and all 616 listed runs are
maximal with exactly the listed lengths (now `check_all_runs.gp`). OEIS practice for giving a later
term while an earlier one is open: A118539 says "a(15)=87888967 but a(14) is unknown."

Findings and what was done:

1. The publishing script missed `direct.c`, `check_definition.gp`, `check_definition_2e14.gp` and
   `check_small.sh`, and would have copied logs of runs still in progress. **Done:** it copies
   every file listed in the README and only finished logs.
2. His sign-off script `check-a388850.sh` covered only the 2*10^13 claims. **Done:** it now also
   runs `check_definition_2e14.gp` and compares the two enumerations to 2*10^14.
3. "a(14) = 0 or a(14) > 2*10^14, and a(15) = ..." can be misread. **Done:** the comment reads
   "a(15) = 52629641927627, but a(14) is unknown: a(14) = 0 or a(14) > 2*10^14."
4. The editor note should say "no run of exactly 15", and "with the same two programs". **Done.**
5. The header of `direct.c` described an earlier version of its stopping rule and its division,
   and the README said it uses "no square points" although its stopping rule looks ahead for one.
   **Done:** header and README corrected (no square-point enumeration and no windows).
6. Three claims had no log: the `cand.c` counts below 40533366300 that `direct.c` is compared
   with, `sieve.c` reproducing a(0) to a(8), and the Method list missed `check_definition_2e14.gp`.
   **Done:** `check_small.sh` now logs the `cand.c` run, the README cites `logs/sieve_5e11.txt` for
   the sieve, and the list is complete.
