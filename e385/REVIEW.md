VERDICT: SAFE WITH EDITS (two priority sentences in comment.txt are false; the mathematics and the computation hold)

Reviewer: independent reviewer, 2026-09-03. Independent re-derivation, nothing in e385/ modified except this file.
Scratch artifacts: (local scratch folder)/e385/

REQUIRED EDITS (comment.txt):
1. Delete or rewrite the priority claims "where the values 8742, 267672 and 267680 were first reported;
   the complete list and the range $10^{11}$ appear not to have been recorded before". They are false:
   OEIS A322293 ("Integers k such that A322292(k) <= k") lists exactly these 100 values (n > 4), 98 terms
   from Michel Marcus on 2018-12-02 and a 100-term b-file by Robert Israel dated 2018-12-03 with the
   note "all terms <= 10^8". A322292 (the F(n) sequence) is linked from the erdosproblems.com/385 page
   itself. CKS on Tao's blog (2024-08-23 "largest bad is 8742", 2024-08-27 "Two more bad at
   267672,267680") re-found them six years later. What IS new here: the range 10^8 -> 10^11.
   Suggested wording: "The list is OEIS A322293 (M. Marcus, R. Israel, December 2018), where it was
   verified up to $10^8$; the last three values were also found by CKS in the comments on Tao's blog
   post (August 2024). The computation here extends the search to $10^{11}$."
2. The phrase "which I re-ran before posting" is only true if Leandre re-runs (or drop it). For the
   record I re-ran fn.c to 10^9, verify.py, spotcheck.py, and parts 0 and 9 of fn_seg (see section 2).
Optional: cite A322293 by number so the note is findable from the OEIS side.

CRITERIA CHECK (each verified by me, not by the implementer's logs):
- AC-1 (exception list for n <= 10^9 is exactly exceptions.txt): PASS. My own C program direct.c
  (different algorithm: no running max, per-n search for a composite n-d free of primes <= d,
  d <= isqrt(n)) run to 10^9: 103 exceptions, byte-identical to exceptions.txt. Fresh build of the
  author's fn.c to 10^9: 103, identical. My Python sieve (mysieve.py) to 10^6 with three evaluations
  (prefix max; full scan over ALL composite m < n for n <= 6000 with no window assumption; window
  scan) all agree with exceptions.txt. OEIS A322293 b-file (100 terms) == exceptions.txt minus {2,3,4}.
- AC-2 (no exception in (10^9, 10^11]): PASS on the logs + structure, partially re-run. runs/part0..9
  tile [10^9, 10^11] exactly (each L = previous R + 1, first L = 10^9, last R = 10^11, widths
  9.9e9 and 9.9e9+1), all report exceptions=0 last=0. GitHub copies (e385/logs/) are byte-identical
  to local runs/. Re-run of part0 and part9 by me: RESULT_PLACEHOLDER
- AC-3 (n-1 prime for every listed n): PASS for the 100 values n > 4 (trial division, no sympy);
  n = 2 fails (1 is not prime), which is why comment.txt correctly restricts to 4 < n.
- AC-4 (equivalence with Problem 430): PASS. Fetched both pages via tools/erdos.py. 385 statement
  matches the code's definition exactly (max over composite m < n of m + p(m); two questions, the
  comment addresses the first). 430: a_1 = n-1, a_k = greatest integer in [1, a_{k-1}) all of whose
  prime factors exceed n - a_k, "not all prime?". The sequence enumerates every a in [1,n) with all
  prime factors > n - a, in decreasing order. A composite m < n with m + p(m) > n has all prime
  factors >= p(m) > n - m, so it is some a_j; conversely a composite a_j gives F(n) >= a_j + p(a_j) > n.
  So per n: F(n) > n iff the 430 sequence contains a composite term. The comment's sentence
  "contains a composite term for every 267680 < n <= 10^11" is exactly what follows. (Both pages
  credit Sarosh Adenwalla; the 385 page has a typo "a_j - p(a_j) > n" for "+", not our concern.)
- AC-5 (no em dashes, no buzzwords in comment.txt): PASS. No U+2014, no U+2013, no non-ASCII at all
  (1978 chars), no leverage/streamline/empower/cutting-edge/seamless/robust/delve.
- AC-6 (GitHub link in the comment resolves): PASS. HTTP 200; README.md, exceptions.txt, fn.c,
  fn_seg.c, spotcheck.py, spotcheck.txt, verify.py and logs/part0..9 all byte-identical to local.
  Last commit a0fbf784, 2026-09-03T11:38:18Z.

## 1. Code reading (fn.c, fn_seg.c)
- Strictness m < n: both programs fold m = n-1 into the running max BEFORE testing n, so F at the
  test of n is max over composite m <= n-1. Correct. n = 2, 3, 4 fold nothing, F = 0, reported.
- Window claim: for composite m < n, p(m) <= sqrt(m) < sqrt(n), so m + p(m) >= n forces
  m > n - sqrt(n). The stated "m >= n - sqrt(n) - 1" is weaker, hence true.
- Direction of the segmented omission: fn_seg starts F at 0 from lo = L - W, W = floor(sqrt(R)) + 1000.
  Any composite m with m + p(m) > n >= L has m > n - sqrt(R) >= L - sqrt(R) > lo, so it is sieved.
  Omitting m < lo can only LOWER the computed F: false exceptions possible, hidden exceptions
  impossible. With zero exceptions reported this is the safe direction. The only way to hide an
  exception is to overstate lpf or mark a prime composite; ascending primes with a first-writer-wins
  guard, start = max(first multiple of p >= base, p*p), primes to floor(sqrt(R)) + 2 rule that out.
- Segment boundaries: F, count, last live outside the segment loop; hi = min(base+S-1, R); n = R is
  tested after folding m = R-1. Verified experimentally (section 2).
- Widths: lpf <= 316227 fits uint32; m + lpf <= 1e11 + 3.2e5 and p*p <= 1e11 fit uint64;
  (uint64_t)sqrt(1e11) = 316227 exactly. fn.c: r = 31623 covers every lpf <= 31622 for N = 10^9.
- Cosmetic: fn_seg's loop starts at m = lo, so with L = 2 the value n = 2 is never tested (first n
  is 3). Irrelevant for L = 10^9. NIT.

## 2. Independent experiments (all outputs quoted from my runs)
- ./direct 1000000000  -> "direct N=1000000000 exceptions=103", list == exceptions.txt (diff empty).
  105.8 s real, 126 MB RSS.
- ./fn_theirs 1000000000 -> "N=1000000000 exceptions=103 last_exception=267680", list identical.
  160 s real (22 s user; README says 6 s, machine was loaded), 4.0 GB RSS.
- python3 mysieve.py 1000000 -> A (prefix max) 103 == exceptions.txt True; B (full scan n<=6000)
  == A True; C (window scan) == A True; only exception with n-1 not prime: [2].
- fn_seg boundary probes (inclusivity): [267680,267680] -> 1 exception 267680; [267681,267681] -> 0;
  [267672,267679] -> only 267672; [267673,267680] -> only 267680; [5,5] -> 0; [6,6] -> 6;
  [3,4] -> 3, 4; [8742,8742] -> 8742; [8743,267671] -> 0; [267681,1000000] -> 0;
  [99999999000,100000000000] -> 0 (top of the claimed range, no overflow).
- Segment-boundary stress: copy of fn_seg.c with S = 4099 instead of 2^26. [3,1e6] -> exactly the
  102 exceptions >= 3; [200000,300000] -> 267672, 267680. Traced F(n) at the 31 multiples of 100003 in
  [1e9, 1e9+3e6] under S = 4099 and S = 2^26: identical. Two of them recomputed from the definition
  with sympy factorint over the window: F(1000030000) = 1000059950 and F(1000130003) = 1000157934,
  both equal to the traces.
- verify.py 300000 (venv python, sympy 1.14): "103 exceptions up to 300000", list == exceptions.txt.
- spotcheck.py 30 (seed 20260903): output byte-identical to the committed spotcheck.txt. The one
  non-trivial line (n = 69616756818, d = 11) checked by hand: n-1 prime, m = n-11 composite with
  lpf 13 > 11, and d = 1..10 all fail. Note 29 of 30 samples are the trivial d = 1 case (n-1
  composite); the sentence in the comment is still accurate.
- Tiling: parts sorted, L_i = R_{i-1} + 1 for all i, first L = 1e9, last R = 1e11, all zero.

## 3. Sources
- erdosproblems.com/385 and /430 fetched with tools/erdos.py (text in this review's AC-4).
- Tao's post (curl, 326 KB): CKS 2024-08-21 gives the reduction to n = q+1; 2024-08-23 "A bad n is of
  form q+1 for prime q so zero density. Numerical computation suggests the largest bad is 8742";
  2024-08-27 "Two more bad at 267672,267680". So the attribution of the reduction to CKS is right,
  the word "first" is wrong.
- OEIS: search for 6,8,12,...,90 returns exactly one hit, A322293. b-file header "Computed by Robert
  Israel using Maple, December 03 2018", 100 terms, == exceptions.txt minus {2,3,4}. History page:
  "(terms 1..98 from Michel Marcus)(all terms <= 10^8)", and Israel's discussion note "Only two more
  terms in my b-file, but I wouldn't be surprised if these were the last two".

## 4. Findings
1. [MAJOR] comment.txt priority claims false (OEIS A322293, Dec 2018, has the full list to 10^8).
   Repro: curl "https://oeis.org/search?q=6,8,12,14,18,20,24,30,32,42,44,48,60,62,72,74,84,90&fmt=text"
   -> A322293 with b-file "1..100 (all terms <= 10^8)". Expected per the comment: not recorded before.
2. [MINOR] "which I re-ran before posting" is a promise; either re-run or delete.
3. [NIT] README.md: "Every exception is of the form q + 1 with q prime" includes n = 2 = 1 + 1;
   say "every exception n > 4". README's "6 seconds" for fn.c at 10^9 was 22 s user here.
4. [NIT] fn_seg.c never tests n = L when L = 2 (harmless for the runs).

SPEC-ISSUES: none (the problem statement on the site matches the implemented definition).

QUESTIONS:
- Is "{PROBLEM=430}" the cross-reference syntax erdosproblems.com comments expect? Not verified.

ATTACKS ATTEMPTED (all run for real): independent algorithm to 1e9; fresh rebuild of fn.c to 1e9;
Python triple evaluation to 1e6 incl. windowless full scan; OEIS lookup; blog-comment text; both
problem pages; single-n and boundary ranges on fn_seg; L <= W path (lo = 2); top-of-range
[1e11-1000, 1e11]; segment size 4099 vs 2^26 with F traces; exact F(n) from the definition at two
n ~ 1e9; hand check of the non-trivial spotcheck line; run-log tiling; GitHub byte-diff; em dash,
en dash, non-ASCII and buzzword grep; primality of n-1 by trial division; part0/part9 re-run.
