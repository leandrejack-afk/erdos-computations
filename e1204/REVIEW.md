VERDICT: SAFE WITH EDITS. Every value and every observation in comment.txt reproduces independently; one ratio sentence overstates the data ("near 0.48" for values that run 0.457 to 0.480) and the linked public README still says the review is in progress and names the model.

Reviewer: independent adversarial review, fresh context, 2026-09-04 (about 60 minutes). Scratch code and outputs:
/private/tmp/claude-501/-Users-leandrejack-projects-math-contributions/b9999d76-7575-45d2-b9a6-9daac0292484/scratchpad/rev1204/

## Pinned inputs

    MD5 (workspace/targets/e1204/comment.txt)        = 1181847509cc8373bd1fbd140048e448
    MD5 (erdos-computations/e1204/README.md)         = 00d38cffd958a0c20872b1d620a51b2e
    MD5 (raw.githubusercontent.com .../e1204/README.md) = 00d38cffd958a0c20872b1d620a51b2e   (remote == local)
    b-file-S.txt, smin.c, rvs.c, runs/crosscheck.txt, runs/rvs-400.txt: remote md5 == local md5
    erdos-computations: `git status -sb` -> `## main...origin/main` (nothing unpushed, no uncommitted change under e1204)
    https://github.com/leandrejack-afk/erdos-computations/tree/main/e1204 -> HTTP 200 without authentication

## The four bars

1. Independent agreement: PASS. Two different exact searches (smin.c, rvs.c) agree for k <= 147 (I re-parsed both logs myself, see below), and I reproduced the values, the optimal tuples and the uniqueness pattern with my own program written from the definition for k <= 127 (see "What I did not get to" for the ceiling), with a reduction-free z3 model for k in {3, 5, 8, 12} and with a reduction-free brute force for k <= 7.
2. Adversarial review run: this file. Required edits are listed below; the verdict assumes they are applied.
3. Code and logs public, comment links to them: PASS (md5s above; the comment's href points at the e1204 folder, which contains smin.c, rvs.c, runs/crosscheck.txt, runs/rvs-400.txt with the tuples, and b-file-S.txt with 147 terms).
4. Disclosure, no model name, no dashes: PASS for comment.txt. `grep -nP '\x{2014}|\x{2013}'` finds nothing; `grep -P '[^\x00-\x7F]'` finds nothing (pure ASCII); `grep -iE 'claude|gpt|fable|opus|sonnet|gemini|openai|anthropic'` finds nothing; the sentence "AI disclosure: the programs were written and run with the assistance of a large language model; ..." is present. Word count 266. The forum rules (fetched today from /forum) say "AI assistance in generating ideas or helping to formulate the text of a comment is allowed, but should be disclosed" and "The contents of all comments, including any mathematical claims, should be independently verified by a human before posting here": the disclosure satisfies the first; the second is Leandre's obligation, and this review gives him the material.

## What I checked and how

### Problem page and forum thread, fetched today

`python tools/erdos.py 1204` and `python tools/erdos.py forum/thread/1204` (2026-09-04, 19:05 local): the statement on the page is

    We call a sequence of integers $0\leq a_1<\cdots <a_k$ admissible if it is missing at least one congruence class modulo every prime $p$. Let $A(k)=\min a_k$. Estimate $A(k)$ - in particular, is it true that A(k) ~ k log k? Estimate B(k)=\min (a_1+...+a_k)/k.

The comment's "S(k) = min(a_1 + ... + a_k) over admissible k-tuples, so B(k) = S(k)/k" matches. Both pages show `Comments (0)`, `Proof claims (0)`, "last edited 07 April 2026"; nothing overlapping has been posted since 2026-09-03. OEIS links on the page: A008407, A023193, A135311.

### The three reductions (paragraph 1)

- a_1 = 0 at the optimum: if (a_i) is admissible then (a_i - a_1) is admissible (translation permutes the residue classes mod every p) and the sum drops by k a_1; so every optimum has a_1 = 0. Correct.
- Only primes p <= k matter: k integers occupy at most k classes mod p, so for p > k a class is always missed. Correct.
- All a_i even: 0 is in the tuple, so class 0 mod 2 is hit and class 1 mod 2 must be the missed one. Correct. (The b-space step used by both programs, a_i = 2 b_i with (b_i) missing a class mod every odd p <= k, is correct because x -> 2x is a bijection on Z/p for odd p.)
- Reduction-free confirmation: brute force over all increasing tuples of nonnegative integers with sum <= S(k) (a_1 free, admissibility checked for every prime up to the largest element) for k <= 7 returns the table values with a unique optimum, each starting at 0:

      k=6: min sum 46 (table 46) optimal tuples [(0, 2, 6, 8, 12, 18)]
      k=7: min sum 66 (table 66) optimal tuples [(0, 2, 6, 8, 12, 18, 20)]

### Are smin.c and rvs.c two exact, exhaustive, tie-keeping searches? (read line by line)

- smin.c: depth-first over increasing b-tuples; an integer is forbidden only when it would complete all residues of some prime, which is exactly the admissibility constraint. Bounds: LB1 = sum of the m smallest non-forbidden integers above the last element (valid: forbidden stays forbidden), LB2 = m*x + S(m)/2 (valid: the tail is an admissible m-tuple translated to 0), optional -A elementwise bounds from A008407 (valid, external, only in the M1' run). Cap b_k <= incumbent - S(k-1)/2 is valid because the first k-1 elements form an admissible (k-1)-tuple starting at 0 and any tuple violating it has sum >= best + 1. Every cut is a strict ">" (`win > slack`, `x > cap2` with floor division, `lb > best`, `sum > best` in record), so ties are kept; the leaf re-checks `sum > best` so a lowered incumbent cannot pollute the tuple list (I introduced exactly that bug in my own first version, see below). MAXOPT = 64 never binds (max 2 optima).
- rvs.c: fixes a missed class r_p != 0 per odd prime p <= k (r_p = 0 is impossible since 0 is in the tuple); the k smallest survivors are the unique sum-minimal k-subset of the sieve, so S(k)/2 = min over choices, and the partial-sieve k-sum is a monotone lower bound. The "settle" shortcut (a class with no survivor in [0, xlim] is chosen without branching) loses no optimal tuple: any other choice gives a subset of the survivors, whose k smallest have a larger sum unless they are the same k elements. Cuts `s > best`, `c > best`, `cost[r] > best` are strict; record dedups. Exhaustive.
- The two programs share the reduction and the greedy incumbent idea but not the search; they are different methods. They were written by the same tool, so the README's "share no code" is generous (the prime loops are near-identical); the comment's "two different exact searches" is the accurate phrase.
- verify_sat.py: written from the definition (all primes including 2, no even reduction, x_0 forced, exact cardinality k, soft weight n on x_n, window from its own S(k-1) or trivial). Sound.

### Rebuild and re-run

    cc -O3 -Wall -Wextra -o smin smin.c ; cc -O3 -Wall -Wextra -o rvs rvs.c      (no warnings)
    ./smin 60 -s 0 -q > smin60.txt ; ./rvs 60 -q > rvs60.txt ; cmp -> "smin/rvs 60 S values identical"; all 60 equal b-file-S.txt
    ./rvs 120 > rvs120.txt   (3.6 s)  -> the (k, S, nopt) fields of its 120 lines have md5 37fb6cc43176e347b8c8f5dd3c36161a,
                                          identical to the first 120 k of the public runs/rvs-400.txt; only k=109 has nopt=2
    verify_sat.py 1 14 rerun (and a second run 1 10): all 14 values equal b-file-S.txt, one tuple each, the greedy tuple;
                                          each k solves in under 0.1 s (the inductive window is only the last greedy element)

### Re-parse of the public logs (my own parser and my own admissibility check, primes up to 2k)

    rvs k range 1 174 ; smin 1 147 ; smin -A 1 144
    value disagreements with b-file among rvs/smin for k<=147: []
    tuple-set disagreements rvs vs smin k<=147: []
    nopt != len(tuples) anywhere? [] []
    k<=147 with nopt>1 (rvs): [(109, 2)]  (smin): [(109, 2)]  (rvs k>147): []
    tuples failing my own check (length, a_1=0, increasing, sum, admissibility): 0     (rvs, smin, smin -A: 460 tuples)
    sat-1-60 k range 1 38, mismatch vs b-file: [], distinct optimal tuples per k (max): 1
    greedy sums printed by rvs-400.txt and r500-s0.txt vs A135311 partial sums: none differ

### My own exact search from the definition (indep.c, a-space, all primes including 2, no even reduction, ties kept)

`./indep 160 -t` (LB1 window bound plus the tail bound m*x + S(m) from its own run; one thread). Result at the end of the budget (k = 127 completed; k = 128 was still running when this was written, about 15 minutes per k by then, left running):

    indep (fixed) reached k = 127 | value mismatches vs b-file: [] | nopt>1: [(109, 2)] | tuple-set mismatches vs rvs log: []
    k=93 my tuple == log tuple: True
    K=127 S=44394 nopt=1 secs=845.10

So for every k <= 127 my program returns the same S(k), the same set of optimal tuples (including the two at k = 109, and the k = 93 tuple behind Observation (1)), and a unique optimum everywhere else. k = 128..147 rest on the two public programs plus my rebuilt-binary reproduction of the rvs.c log to k = 120.

My first version recorded leaf tuples without re-checking the sum against an incumbent lowered by a sibling subtree, which produced spurious extra "optima" for k = 94..104 (sums 23010, 25212, ... above S(k)); fixed (`if(sum>best)return;` at the leaf) and rerun. I mention it because it is exactly the bug class that would break Observation (2), and both public programs guard against it.

### Reduction-free z3 decision checks (z3 5.1.0; x_n free for n in [0, N], a_1 NOT forced to 0, every prime <= k, exact cardinality, pble sum <= T; window N = T - (k-1)(k-2)/2 needs no reduction)

    k=3  T=8   sat   T=7   unsat
    k=5  T=28  sat   T=27  unsat
    k=8  T=92  sat   T=91  unsat
    k=12 T=232 sat (tuple [0, 2, 6, 8, 12, 18, 20, 26, 30, 32, 36, 42])   T=231 unsat
    k=16 T=448: stopped after 6 minutes without an answer (z3's pseudo-Boolean engine, N = 343 variables); k=20 not attempted

Every satisfying tuple z3 returned starts at 0 although x_0 was free, consistent with the translation reduction.

### Observations (1), (2), (3) recomputed from the OEIS b-files (fetched today) and the logs

    my greedy (from the definition) == A135311 b-file for n <= 200: True
    k with S(k) == greedy sum: every k <= 92; none in 93..170        (S-all-runs.txt, values confirmed by both logs to 147)
    S(93) = 22474, greedy sum 22480; greedy minus optimum = [282, 530]; optimum minus greedy = [366, 440]
    k=109: two tuples, both admissible, both sum 31862, starts (0, 2, 6, 8, 12, 18, 20, 26) and (0, 4, 6, 10, 16, 18, 24, 28); neither is greedy
    k=20  B/(k log k)=0.6276  A=80   B/A=0.4700
    k=40  B/(k log k)=0.5944  A=186  B/A=0.4715
    k=100 B/(k log k)=0.5722  A=558  B/A=0.4723
    k=147 B/(k log k)=0.5674  A=878  B/A=0.4741
    B/A over 40..147: min 0.4571 at k=77, max 0.4795 at k=41, median 0.4696, mean 0.4688; 5 of 108 values >= 0.475, 24 below 0.465
    B/(k log k) is not monotone: it rises at 20 of the 127 steps in 20..147 (e.g. k=62, 68..71, 99..101, 124, 129, 130)
    A(k)/(k log k) at k=147: 1.1968
    S(k) >= sum_{j<=k} A(j) for all k <= 147: True ;  S(k) < k A(k) for 2 <= k <= 147: True ;  all S(k) even: True

A008407 offset 1 with a(1) = 0, a(2) = 2 (b-file, 342 terms; its comment says terms past a(342) are only best known, so "confirmed minimal for k <= 342" in the README is right).

### OEIS: is S(k) or B(k) already there?

Term searches (fmt=text) for `0,2,8,16,...,330`, `2,8,16,...,386`, `8,16,28,...,448`, the halved forms `1,4,8,14,...,193` and `4,8,14,...,224`: all "No results." Definition searches `"admissible" "minimal sum"`, `"Erdos problem 1204"`, `"1204" erdos admissible`: "No results."; `admissible k-tuple sum smallest` and `A135311` return only A008407, A292224 (counts of admissible tuples), A274261, A388062, A388999, prime-tuplet lists. Cross-references of A008407 (A020497, A083409), A135311 (A008407, A020497), A023193 (A047947, A066081, A062571, A292224, A364678), A388999, A020497: none is a minimal-sum or partial-sum sequence. The comment claims no novelty ("Exact values of B(k)"), so nothing hinges on this, but the claim in the README that no OEIS entry exists is supported.

### Forum rendering of the comment

Post bodies on the site (threads 1, 3, 5, 10, 44, 1000, 1003, 1004, 1005 fetched today) contain user-written `<a href=...>` anchors, re-emitted by the site with `rel="nofollow noopener noreferrer ugc" target="_blank"`; the only tags that survive are a, br, p, div. 19 of 19 anchors have custom link text, and no bare URL is auto-linked, so the comment's `<a href>` form is the right one. Plain-text `>` inside posts is stored escaped (`c_1&gt;0`), so "<=" will display literally. Underscores in "a_1" are safe (no markdown conversion is visible in any post).

## Required edits before posting

1. comment.txt, paragraph "Observations", item (3). The ratio does not sit near 0.48: it runs from 0.457 (k = 77) to 0.480 (k = 41) with median 0.470.
   old: `B(k)/A(k) stays near 0.48 for 40 <= k <= 147.`
   new: `B(k)/A(k) stays between 0.45 and 0.48 for 40 <= k <= 147.`

2. Same sentence in the public folder, so that the linked README agrees with the posted comment.
   erdos-computations/e1204/README.md, bullet 3 under "Facts from the computed range":
   old: `B(k)/A(k) stays near 0.48 for 40 <= k <= 147.`
   new: `B(k)/A(k) stays between 0.45 and 0.48 for 40 <= k <= 147.`
   erdos-computations/e1204/oeis-draft.md, COMMENTS:
   old: `The ratio B(n)/A008407(n) stays near 0.48 for 40 <= n <= 147.`
   new: `The ratio B(n)/A008407(n) stays between 0.45 and 0.48 for 40 <= n <= 147.`

3. Public README and REVIEW.md: the folder the comment links to must not say the review is unfinished once it is finished. Copy this file to erdos-computations/e1204/REVIEW.md (replacing the stub), then in README.md:
   old: `The adversarial review of this folder was still in progress when it was packaged; ``REVIEW.md`` is a copy of the review record as it stood at that time and may need refreshing.`
   new: `` `REVIEW.md` is the independent adversarial review of this folder (2026-09-04); its required edits are applied here.``
   old: `` `verify.md`: the verification record. `REVIEW.md`: the review record (in progress).``
   new: `` `verify.md`: the verification record. `REVIEW.md`: the independent review record.``
   Then commit and push, and re-check that raw.githubusercontent.com serves the new README before posting.

## Optional edits

- README.md line 125 names the model ("Programs written and run with Claude Code (Claude Fable 5.1) assistance") while the comment deliberately does not. If the intent is no model name anywhere public, use: `Programs written and run with the assistance of a large language model; every claim above comes from the executed runs.` Otherwise leave it, knowingly. (The stale public REVIEW.md also names it; edit 3 removes that.)
- comment.txt (3): "decreases slowly" is a sampled trend, not a monotone fact (the ratio rises at 20 of 127 steps). `drifts down slowly` would be exact; the four sampled values are correct either way.
- comment.txt (3): "still well above the expected 1/2 + o(1)" reads as doubt about the conjecture, but the same range has A(k)/(k log k) = 1.20 at k = 147, above even the constant 1 of the known upper bound, so lower-order terms dominate here. Neutral alternative: `compared with the expected 1/2 + o(1) (for scale, A(k)/(k log k) = 1.20 at k = 147)`.
- README.md "Result": `two independent exact searches that share no code` -> `two different exact searches`; both files were written by the same tool and share the reduction, the greedy incumbent and near-identical prime loops. "Different" is the defensible word and is what the comment says.
- Private STATUS.md and verify.md carry "B(k)/A(k) near 0.48" as well; harmless, but fix if they are ever published.

## Open questions (not demonstrated, not blocking)

- I could not test how the forum renders a plain-text "<=" outside math; the sampled posts only have "<" inside LaTeX, where it is stored escaped. A sanitizer that keeps `<a>` and escapes stray "<" will display "<=" correctly; the risk is low. `$\leq$` would be the zero-risk form but changes the style of the whole comment.
- Literature beyond the OEIS and the problem page was not searched (Polymath8b and Sutherland's tables concern the diameter, not the sum). The comment makes no priority claim, so nothing depends on this.

## What I did not get to

- Independent exact reproduction stops at k = 127 (one thread, budget); k = 147 would need hours (the public smin run took 9 h). Values, tuple sets and the uniqueness pattern agree on every k reached; above that, the evidence for k <= 147 is the agreement of smin.c and rvs.c (both read and judged exhaustive) plus my reproduction of the rvs.c log to k = 120 with a rebuilt binary.
- z3 reduction-free checks stop at k = 12 (k = 16 was stopped after 6 minutes to free the thread for the MaxSAT rerun; k = 20 not attempted). Reduction-free confirmation therefore covers k <= 12 by z3 and k <= 7 by brute force; the even-number and p <= k reductions are proven above, not just tested.
- Did not rerun the 9-hour smin run, the MaxSAT run to k = 38, or the M1' run; I re-parsed and re-verified their logs instead.
