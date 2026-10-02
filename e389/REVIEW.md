# Adversarial review: Erdős 389, n = 28 and n = 29

VERDICT: SAFE WITH EDITS (edits E1 to E6 below; E1 to E4 are required before posting)

Reviewer: independent reviewer, fresh context, 2026-09-03 17:00 to 17:50 local, bounded review.
Pinned inputs (md5): comment.txt 70349428c39a109d4e6ea142647a1824, results.md e50c2a0baa1f3533d4c7a8cf88221494,
verify.md b7ca51639c6de962442ab4faf4782ca7, oeis-extension.txt 4f9f877e3d2ff5eb4d0f9ae68668298d,
e389_sieve.c 477a6c4289d19a8146e06515bd44a927, e389_sieve (binary) f5b86ae1ef50025a0fbaa2a06a534ee9,
reference-sharvil-erdos_problem_389.cpp 0c61bca7a1a83543e8c6bafc277cacc3, evidence-sharvil-n28.txt ee998a2287a95970ca6f0c7c925e0587.
The target directory was being edited during the review (the author's n = 28 sieve run was killed and its logs deleted,
a new binary e389_sieve.new appeared); everything below refers to the pinned files.
Reviewer code and raw outputs: the session scratchpad (folder rev389/)
(mycheck.py = own valuation checker from the definition; bf389.c = own incremental brute force; compare.py; my_sharvil389 = own build of the published solver.)

## Bottom line

The mathematical claim is right: k = 18253129921815 (n = 28) and k = 18253129921814 (n = 29) are witnesses, and
nothing smaller works. Minimality rests on ONE completed exhaustive run (Sharvil Kesarwani's published solver over
[1, 18253129921842]) plus the C sieve's exact brute force below 2^20. The comment overstates the evidence in three
places and understates a correction of another poster's numbers in a fourth. Fix the wording and it is safe to post.

## Required edits to comment.txt

E1 (MAJOR, overclaim). "The search was run twice, with independent code." At review time only Sharvil's solver has
completed the frontier scan. The C sieve's own n = 28 scan reached N ~ 1.1e11 of 1.8e13 (under 1 percent) at 259 M/s
and was then killed; run_28.out was empty and both run_28.* files and surv_28_full.txt have since been deleted.
verify.md section 2b admits the run was in progress. Either finish the C run (about 20 h at the observed rate on
this loaded machine) and keep the sentence, or rewrite along the lines of: "Minimality was established by an
exhaustive scan with Sharvil Kesarwani's published solver over [1, 18253129921842]. A second implementation of the
same governor-sieve method in C reproduces every published term a(1)..a(24), including a full scan to 1.07e12 for
n = 24, 25, and agrees with the published solver on a 2^36 window near 1e13, but its own frontier scan was not
completed."

E2 (MAJOR, overclaim). "from-scratch C governor sieve" / "independent code", and verify.md's "different data
structures, different rejection rules, different exact oracle". The C sieve is a port of Sharvil's design: same
strider / big-power / bucket / active-stride structures, 2^26 chunks, 27-bit offset item packing, scale-4 u8
log-mass accumulator, chunk-wide base-p digit skip (Sharvil's "Kummer skips"), Legendre oracle on the factored
window. Sharvil's source was in the workspace at 08:15 and the C sieve's first regression output is timestamped
08:38. Call it a second implementation of the same method, not an independent method.

E3 (MAJOR, overclaim). "The witnesses were confirmed to divide three unrelated ways (big-integer product,
central-binomial ratio, prime valuations)." verify.md section 1 says the product and binomial paths are gated to
k <= 2e7; at the witness only the valuation path ran. Say: confirmed by prime valuations, with the big-integer and
binomial paths used at small k to validate that path.

E4 (MAJOR, factual). The aside "A375077's a(16) = 20021368432952099 gives smaller n = 34, 35 witnesses than quoted
earlier, k = 20021368432952066 and 20021368432952065" implies the earlier values were witnesses. They are not.
Sharvil Kesarwani's forum values (25 Aug 2026), k = 20021368432952132 (n = 34) and 20021368432952131 (n = 35),
FAIL at p = 67: v_67(left product) = 303354067165939 > v_67(right product) = 303354067165938 (direct Legendre on
the definition intervals, both n). They equal N + n - 1 instead of N - n + 1 for N = 20021368432952099. The
replacement values do divide (own check). Say so plainly, since this corrects another poster's numbers, and
attribute them to Sharvil Kesarwani: results.md wrongly says "the n = 34, 35 witnesses KentaKitamura quoted" and
calls them "not minimal even as witnesses".

E5 (MINOR, editorial, oeis-extension.txt). The live A375071 entry's %E line reads "a(19)-a(27) from Sharvil
Kesarwani, Mar 30 2026" but the data field ends at a(26) (27 terms, no b-file). Bhavik Mehta's line "a(10)-a(18)"
likewise disagrees with the problem page's "1 <= n <= 18" (= a(0)..a(17)). Tell the editor the existing credit
lines look shifted by one so the new a(27), a(28) do not collide.

E6 (MINOR, housekeeping, verify.md). It cites run_28.out / run_28.err (deleted) and "u8 threshold ceiling (2^59)";
the source guards Nhi < 2^49 and the header proves soundness for N < 2^49.

## CRITERIA CHECK (delegation items 1 to 5)

- 1 Problem statement, OEIS, forum: PASS. Page: least k with n(n+1)...(n+k-1) | (n+k)...(n+2k-1), open, formalised,
  OEIS A375071. A375071 offset 0, a(0)=1 (n=1), a(1)=5 (n=2): index i is n = i+1. Last published term a(26) =
  5048891644620 (n = 27); no b-file (synthesized). Forum: KentaKitamura, 08:16 on 01 Aug 2026, posted n = 28..33
  with "These are upper-bound witnesses only"; Sharvil Kesarwani, 13:58 on 25 Aug 2026, posted (34, ...132),
  (35, ...131) and pointed to github.com/sharky564/ErdosProblems. Comment's attributions for n = 28, 29 and for
  the solver are exact. A375077 a(13) = 18253129921842 (Dehorty and Kesarwani, certified minimal for #396) equals
  the claimed N, consistent with the pattern for n = 20..27.
- 2 Witnesses divide, k-1 fails (own code, not e389_check.py): PASS.
  `python3 mycheck.py pair 28 18253129921815` -> DIVIDES (9619 primes: all primes <= 1e5 plus every prime factor
  of 2N-i, i < 27; Legendre and Kummer-carry counts asserted equal at every prime).
  `pair 29 18253129921814` -> DIVIDES. `pair 28 18253129921814` -> FAILS at p=84239. `pair 29 18253129921813` ->
  FAILS at p=84239. Calibration: `mycheck.py selftest 400` (valuation == raw big-integer product for n = 2..9,
  k <= 400) and `mycheck.py oeis` (all 27 published terms divide, each k-1 fails).
- 3 Sieve soundness: PASS on paper and by experiment (details below). Note the delegation's literal test
  "n = 2..12, k <= 5000" cannot exercise the rules: SIEVE_START = 2^20 sends every N < 2^20 through the exact
  oracle, so I tested the rules above 2^20 instead.
- 4 Sharvil run evidence: PASS with a caveat. Solver file is byte-identical to the GitHub master file (md5
  0c61bca7...). RESULT semantics (source lines 820-905, 980-1010): "RESULT n min1 min2" where min1/min2 are the
  least N > m passing exact_check389 for m = n-1 and m = n (PAIR=1), scanned chunk by chunk from start_L while
  chunk start < end_L and < max(min1, min2). exact_check389 is complete: p = 2 via popcount, odd p <= m by full
  Legendre, and every prime > m dividing N, N-1, ..., N-m by trial division with the grown prime table
  (16x sqrt headroom, so cofactors are fully resolved at this size). Evidence: scratchpad/sharvil-run has the
  binary and an empty stderr at 08:15:21 and result-n28.txt "RESULT 28 18253129921842 18253129921842" at
  10:10:46, i.e. 1 h 55 m, consistent with the claimed ~1 h 50 m and with a full scan from 1 at the benchmarked
  ~3.4 G candidates/s. Caveat: the command line and the range are not recorded anywhere (the evidence file is the
  bare RESULT line); the range [1, 18253129921842] is asserted in verify.md only.
- 5 comment.txt: em dashes 0 (byte scan for U+2014, all four deliverables), no non-ASCII. Overclaims: E1 to E4.

## FINDINGS

1. [MAJOR] Comment claims two completed independent exhaustive runs; only one completed. Repro: at 16:59 `ps`
   showed the n = 28 sieve at N ~ 1.1e11 (run_28.err last line "progress: N ~ 112676831219 ... chunks
   1679/271993"); at 17:05 PID 99417 was gone and run_28.out (0 bytes), run_28.err, surv_28_full.txt deleted.
   Expected per comment: a finished second scan. (E1)
2. [MAJOR] "from-scratch" / "independent" / "different data structures, rejection rules, oracle" is not accurate;
   the C sieve reimplements Sharvil's architecture (see E2 for the one-to-one correspondence).
3. [MAJOR] "three unrelated ways" at the witness: only valuations ran there (verify.md section 1 says so). (E3)
4. [MAJOR] Aside on n = 34, 35 misdescribes the earlier values as witnesses and results.md misattributes them.
   Repro: `python3 mycheck.py pair 34 20021368432952132` -> FAILS at p=67; `pair 35 20021368432952131` -> FAILS at
   p=67; direct Legendre v_67 left 303354067165939 vs right 303354067165938. (E4)
5. [MINOR] OEIS %E line collision with the new a(27). (E5)
6. [MINOR] verify.md stale references and the 2^59 vs 2^49 ceiling. (E6)
7. [NIT] Sharvil run evidence records no command line or range.

## Sieve soundness (delegation item 3), in my own words

Reduction (checked): with a = n-1, N = n+k-1, the condition is C(2N, a) | C(2N, N). For a prime p > a at most one
of 2N, ..., 2N-a+1 is a multiple of p; if that is 2N-i with i odd the low e digits of N are (p+i)/2 followed by
(p-1)/2's, so N+N carries at least e times and p never fails; if i = 2j then m = N-j = p^e u and, since 2j < p,
the carries of N+N equal the carries of u+u, so the condition at p is exactly v_p(m) <= carries_p(u) ("m is a
p-governor") for each of the h = ceil(a/2) numbers N, ..., N-h+1. Rule 1: a prime factor p > sqrt(2m) gives
u = m/p < p/2, a single digit below p/2, zero carries, e = 1: not a governor. Rule 2: for p || m, governor iff
some base-p digit of u is >= (p+1)/2; for p^2 | m, if no digit of m/p is >= (p+1)/2 then carries(u) = 0 < e.
Both sound. Mass bookkeeping: each prime-power factor adds floor(4 log2 p); a fully accounted m < 2^49 has
mass >= 4 log2 m - 30 (fewer than 31 odd prime-power factors, each losing < 1), threshold is floor(4 log2 bL) - 36
with bL the block base, so every fully accounted m is kept, while one withheld prime (>= 4096, mass >= 48) or one
unsieved prime (> sqrt(2R), mass >= 42 for m >= 2^20) pushes the mass below the threshold, and unsieved primes are
exactly those rule 1 already condemns because the prime table runs to isqrt(2(Nhi + CHUNK + MAX_H)) + 2 and each
chunk sieves up to isqrt(2R) + 1. Chunks overlap by h-1 and start at 2^20 - (h-1), so the first window ends at
2^20 and the brute force covers [n, 2^20). The exact oracle exact389 factors 2N-i for i < a with trial division
to 65536 then deterministic 7-base Miller-Rabin and Pollard-Brent, and compares Legendre valuations at every prime
found; primes not dividing the numerator of C(2N, a) cannot fail, so it is complete. The multi-thread stop rule
(break only when the chunk start exceeds every current best) processes every chunk below the final minimum.

Experiments (all with the pinned binary, no source changes):
- h = 2 window test: `e389_sieve 4 1048576 3145728 -t 2 -x 13 -o survB.txt` (560,825 survivors) versus my
  brute force `bf389 20 69206016 4 19` (incremental prime-valuation tracking, no sieve, no rules). Result of
  compare.py: 61,138 brute-force solutions for n' = 4..11 in [2^20, 2^20 + 2^21); 0 solutions missing from the
  survivor list (no false rejection); 0 false accepts among 8 verdict-1 lines; 0 disagreements among 1,193,031
  verdict-0 lines; the sieve's RESULT minimum equals the brute-force least N >= 2^20 for all ten n' (4..13, the
  last two NONE on both sides). Five sampled rejected survivors re-derived with mycheck.py: blamed primes agree.
- Exact prefix at the frontier: `e389_sieve 28 28 1048576 -t 3` -> "RESULT n=28 NONE in [28, 1048576)" and the
  same for n = 29 (30.3 s; every N below 2^20 through the exact oracle).
- Independent low-range cover at the frontier: my brute force `bf389 29 134217728 28 29` (no sieve, no rules,
  exact prime-valuation tracking) finished with "done: 0 solutions in [29, 134217728)": no k works for n = 28 or
  n = 29 with N below 2^27, by a third method.
- h = 3 two-chunk test (`e389_sieve 6 1048576 69206016 -x 19`, chunk boundary at 2^20 + 2^26) was still in its
  exact-check phase (12 min, machine shared with the author's 10-thread run) when the review budget closed, and my
  brute force over the same range was at N = 3.6e7 of 6.9e7; partial outputs are in the scratchpad (survA.txt /
  sieveA.out, bf_sols.txt). Nothing in the completed tests suggests they will disagree.

## Published solver, my own build (delegation item 4)

`g++ -O3 -march=native -std=c++23` on the pinned source, then:
- `PAIR=1 NT=4 ./my_sharvil389 28 1 1073741824` -> RESULT 28 0 0 (no solution below 2^30 for either n).
- `PAIR=1 NT=4 ./my_sharvil389 28 18253129787624 18253129921842` -> RESULT 28 18253129921842 18253129921842.
- same with end 18253129921841 -> still reports 18253129921842: the end bound is rounded up to the chunk, so the
  author's range argument covered the witness.
- `PAIR=1 NT=4 ./my_sharvil389 22 1 17609764995` -> RESULT 22 17609764994 17609764994 = a(21), a(22).

## SPEC-ISSUES
- OEIS-side: the A375071 credit lines are shifted by one relative to the data (see E5). Not the author's error,
  but the submission should mention it.

## ATTACKS ATTEMPTED
- Own valuation checker from the definition (Legendre on the two product intervals, Kummer carries asserted equal
  at every prime, deterministic factoring); self-test against raw big-integer products; reproduction of all 27
  published terms with k-1 failing each time.
- Witnesses and k-1 for n = 28, 29 (both pass / fail as claimed, failing prime 84239).
- n = 34, 35 aside: both the author's replacement values (divide) and the forum values (fail at 67).
- Sieve rules re-derived and each proven; threshold arithmetic and prime bound rechecked against the constants;
  chunk overlap, first-window start, u8 range, stop rule, oracle completeness read line by line.
- Brute-force-versus-survivor comparison over 2^21 values with 61k solutions (zero false rejections).
- Exact prefix below 2^20 for n = 28, 29 rerun.
- Own build of Sharvil's solver on three ranges plus a calibration pair; end-bound semantics probed with end =
  witness - 1.
- Provenance: reference solver md5 matches the GitHub master file; results-389-master.txt shows Sharvil published
  only to n = 25, so n = 28, 29 are new.
- Evidence timeline: binary / stderr 08:15:21, result 10:10:46 in scratchpad/sharvil-run; author's n = 28 C run
  observed live at under 1 percent and then deleted.
- Em-dash and non-ASCII byte scan of all deliverables (clean).

## QUESTIONS
- The Sharvil run's start argument (1) is not recorded in any artifact; the 1 h 55 m wall time is consistent with
  a scan from 1 at the benchmarked speed but does not prove it. A rerun logging the command line would close this.
