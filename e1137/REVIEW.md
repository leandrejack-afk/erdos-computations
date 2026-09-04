VERDICT: SAFE WITH EDITS. Every number in the draft reproduces from my own code and from OEIS, but one sentence is false (OEIS A120384 already tabulates the record products, 54 terms identical to the log) and the opening "the heuristic above" points the wrong way under the thread's default newest-first order. Two required edits, then post.

Reviewer: opus-reviewer (fresh context), 2026-09-04, about 65 minutes, two threads.

## Pinned inputs

- `/Users/leandrejack/projects/math-contributions/workspace/targets/lit-sweep.md` md5 `98c66f0c1bd7250cfdc2eba9a9395325` (draft = the blockquote under "## #1137", lines 182 to 192)
- `/Users/leandrejack/projects/math-contributions/workspace/targets/lit-sweep-1137-table.md` md5 `ccb33d6b73967d3531e48b4ce3023f39`
- Evidence folder `/Users/leandrejack/projects/math-contributions/workspace/evidence/e1137-prime-gaps/` (gaps.c, gapseg.c, mk1137.py, gaps_1e13.log, segs/seg12_1..9, seg13_1..18)
- Public folder: `erdos-computations` commit `e7933df` (pushed 2026-09-04 23:03 UTC, repo public). `table.md`, `gaps.c`, `gapseg.c`, `logs/gaps_1e13.log` and all 27 `segs/*.txt` are byte-identical to the evidence folder (`diff` clean); `mk1137.py` differs only in the log path (`logs/gaps_1e13.log`).
- Scratch: `/private/tmp/claude-501/-Users-leandrejack-projects-math-contributions/b9999d76-7575-45d2-b9a6-9daac0292484/scratchpad/rev1137/` (myscan.c, myseg.c, cramer.c, fetched OEIS/thread files, outputs).

## What I checked and how

### 1. Independent reproduction of M(X), P(X) (bar item 1)

Own program `myscan.c` (not derived from gaps.c: it classifies each gap by the decade of its upper prime, keeps per-decade maxima and prefix-maxes at the end; it also counts primes per decade). Compiled `cc -O2 -I/opt/homebrew/include -L/opt/homebrew/lib myscan.c -lprimesieve`. Run `./myscan 100000000000`, one thread, 29.0 s:

```
X=10^6 pi=78498 M=114 after 492113 P=3800 = 100*38 mid 396833 ratio=0.292398
X=10^7 pi=664579 M=154 after 4652353 P=8400 = 60*140 mid 8917523 ratio=0.354191
X=10^8 pi=5761455 M=220 after 47326693 P=15120 = 140*108 mid 72546283 ratio=0.312397
X=10^9 pi=50847534 M=282 after 436273009 P=26928 = 132*204 mid 476956933 ratio=0.338615
X=10^10 pi=455052511 M=354 after 4302407359 P=41748 = 142*294 mid 9085929179 ratio=0.333142
X=10^11 pi=4118054813 M=464 after 42652618343 P=62160 = 222*280 mid 31587561361 ratio=0.288719
```

The prime counts equal pi(10^k) (78498, 664579, 5761455, 50847534, 455052511, 4118054813), so the iterator saw every prime. Every M, after-prime, P, factor pair, middle prime and ratio equals the draft rows for 10^7 to 10^11 and the table's 10^6 row.

Second oracle without primesieve: a numpy sieve to 10^8 (`np.diff` on the prime array) gives the same three rows for 10^6, 10^7, 10^8 and the same first 37 record products and middle primes as the log.

### 2. The 10^12 and 10^13 rows: own block scans (bar items 1 and 3)

Own program `myseg.c` (gaps whose lower prime is in [A,B], pairs whose middle prime is in [A,B], iterator started 20000 below A). Results, each on one thread:

```
BLOCK [5000000000000,5499999999999] primes=17071328988 M=650 after 5120731250207 P=127500 = 510*250 mid 5061226833937   (4 min 31 s; equals segs/seg13_9.txt)
BLOCK [7000000000000,7499999999999] primes=16885152411 M=674 after 7177162611713 P=101376 = 384*264 mid 7401357517607   (4 min 57 s; equals segs/seg13_13.txt)
BLOCK [100000000000,999999999999]  primes=33489857205 M=540 after 738832927927 P=86156 = 238*362 mid 883442069849   (7 min 51 s; the whole 10^12 row in one pass; primes = pi(10^12) - pi(10^11) = 37607912018 - 4118054813)
BLOCK [1000000000000,1499999999999] primes=17955297911 M=588 after 1408695493609 P=109296 = 414*264 mid 1032148488557   (4 min 38 s; equals segs/seg13_1.txt)
BLOCK [3500000000000,3999999999999] primes=17269963451 M=554 after 3621153039299 P=115056 = 408*282 mid 3605572653889   (4 min 25 s; equals segs/seg13_6.txt)
```

So the 10^12 row is independently reproduced over its whole range in a single pass, and for 10^13 the block that produces P(10^13) = 127500 and the block that produces M(10^13) = 674 are independently reproduced. The README's own cross-check claims also reproduce with my program: `myseg 0 499999999` gives 282 after 436273009 and 26928 = 132*204 (mid 476956933); `myseg 500000000 999999999` gives 276 after 649580171 and 23460 = 138*170 (mid 879353381).

Coverage and recombination (script over the 27 `SEG A B` headers): first A = 99999990000, last B = 9999999999999; every boundary has B + 1 = next A except the two deliberate 10^4 overlaps at 10^11 and 10^12 (a repeated window cannot change a maximum; the 10^11 window was also inside the single scan). Per block, the block's max gap equals the largest A005250 record whose lower prime lies in the block, or lies below the running record when the block holds none: 27 of 27 consistent. Re-deriving the recombination from the seg files by my own script:

```
10^12: 540 738832927927 86156 (238, 362) 883442069849 0.295460
10^13: 674 7177162611713 127500 (510, 250) 5061226833937 0.280666
segment records: 65436, 81320, 85860, 86156, 109296, 115056, 127500 (same seven as table.md)
```

gapseg.c convention: a gap counts by its lower prime, a pair by its middle prime, so a gap straddling 10^12 or 10^13 would be counted in the lower row. No record is near either boundary (the primes just below 10^12 and 10^13 are followed by gaps of 50 and 66), so no row changes. The README states this convention correctly.

### 3. Every M(X) row against OEIS (bar item 2)

`b005250.txt` and `b002386.txt` fetched 2026-09-04, 85 lines each, strictly increasing. For X = 10^6 to 10^13 the largest record gap with both primes below X, and the largest with lower prime below X, both equal the draft's (gap, prime): 114/492113, 154/4652353, 220/47326693, 282/436273009, 354/4302407359, 464/42652618343, 540/738832927927, 674/7177162611713. Eight of eight. The record after 674 is 716 after 13829048559701 > 10^13, so "the largest gap below 10^13 is 674" is right. A005250's 85th and last term is 1854 after 101412319996363309069 (about 1.014e20).

### 4. Every number in the draft (bar item 4)

All seven products factor as written. Ratios: 0.354191, 0.312397, 0.338615, 0.333142, 0.288719, 0.295460, 0.280666, which round to the draft's 0.354, 0.312, 0.339, 0.333, 0.289, 0.295, 0.281. Rows 10^7 to 10^10 lie in [0.3124, 0.3542] ("0.31 to 0.35" holds), rows 10^11 to 10^13 in [0.2807, 0.2955] ("0.28 to 0.30" holds), all seven in [0.28, 0.36]. The 10^6 row, omitted from the comment, is 0.292 (see optional edit 4). Middle primes 31587561361 and 5061226833937 and the pairs 222/280 and 510/250 are as in the data.

### 5. Format (bar item 5)

Blockquote extracted (9 lines, 333 words with the tag stripped). No U+2014, U+2013, U+2012, U+2212 or non-breaking space; the only non-ASCII character is "é" in Cramér. "AI disclosure:" present; no model or vendor named ("model" occurs once, in "large language model"). The forum accepts the draft's link form: the post preview script whitelists exactly `<a href="...">text</a>` and `{PROBLEM=n}`, escapes everything else and turns newlines into `<br>`; server-side rendering of such anchors is visible on threads 385 (the author's own e385 link) and 389 (an OEIS link). The seven table lines will render one per line.

### 6. Problem page and thread today (2026-09-04)

Fetched with `python3 workspace/tools/erdos.py 1137 forum/thread/1137`. Three comments, unchanged: Przemek Chojecki, 12:24 on 02 Feb 2026, gives the Cramér-Poisson argument and ends "on the order of a positive constant, plausibly about 1/4 in this simplified model"; FelixPernegger, 25 Jan 2026, "The numerator is the record version of A083550 and the denominator the square of A005250"; old-bielefelder, 25 Jan 2026, asks how to see a record version. Nobody has posted numbers. The problem page's OEIS field lists A083550 and A005250 only. Default thread order is newest first (`<a href="/forum/thread/1137?order=newest" class="order active">`), so a new comment appears at the top, above Chojecki's.

### 7. The heuristic itself

Chojecki's argument is right at leading order and the draft does not overstate it. With N gaps, normalised gaps u_n roughly i.i.d. Exp(1): max u_n is about log N. For a product, P(u v > t) = integral of e^{-u - t/u} du = 2 sqrt(t) K_1(2 sqrt(t)), about sqrt(pi) t^{1/4} e^{-2 sqrt(t)}; setting N times this to 1 gives sqrt(t) about (1/2) log N + (1/4) log(log N / 2) + O(1), so max product / (max gap)^2 tends to 1/4 from above, roughly 1/4 + log(log N / 2) / (2 log N). Monte Carlo of the exact i.i.d. model (`cramer.c`, xoshiro256**; mean over trials of max adjacent product / max^2):

```
N=1e5 T=200 mean ratio=0.3211 (sd 0.083)   formula 0.326
N=1e6 T=100 mean ratio=0.3175 (sd 0.067)   formula 0.320
N=1e7 T=30  mean ratio=0.3086 (sd 0.046)   formula 0.315
N=1e8 T=10  mean ratio=0.2964 (sd 0.048)   formula 0.310
N=1e9 T=3   mean ratio=0.3240 (sd 0.031)   formula 0.306
```

So the model predicts about 0.30 to 0.32 at the N of these rows (6.6e5 to 3.5e11 primes) with a realisation-to-realisation spread of 0.03 to 0.08, and a limit of 1/4 approached like log log N / log N. The observed 0.28 to 0.35 sits inside that band; the data neither confirm nor refute a drift. The draft's "consistent with the Cramér prediction of a limit near 1/4 rather than 0; seven decades cannot separate a positive limit from a very slow decay" is fair. "Drifts down slowly" is the one phrase the data do not support on their own (optional edit 4).

### 8. Is there already an OEIS table of record products? Yes, in disguise

OEIS A120384, "Isolated primes: geometric mean of distances of a prime to neighboring primes sets record" (Alexis Monnerot-Dumaine, 2006; b-file by Ken Takusagawa, 54 terms; last edit 26 Dec 2024). The geometric mean sqrt(g1 g2) sets a record exactly when the product g1 g2 does, so this is the sequence of middle primes of the record products, which is what Pernegger's "record version of A083550" means. Its 54 terms are term-by-term identical to the 54 `RECORD PROD` middle primes in gaps_1e13.log (3, 5, 7, 23, 53, ..., 15318488291, 24016237123, 31587561361); the last term is the 222*280 record. Nothing else found: term searches for the product values (2,4,8,24,36,48,56,144,180,...), for the middle primes (which return only A120384), keyword searches ("product of two consecutive prime gaps", "adjacent prime gaps", "consecutive gaps ... record"), and A083550's cross-references (A083538 to A083555 are sigma/phi/lcm analogues, A057467 is a gcd) all come back empty or unrelated; A383652 and A375009 are related but not record sequences. So the product values are not in OEIS, but the record positions are, and the draft's sentence "there seems to be no published table of record products of consecutive gaps, so the numerator has to be recomputed" is false as written. The notes in lit-sweep.md line 144 ("OEIS has no sequence of record values of the product of two consecutive prime gaps ... [verified ...]") carry the same error. The upside: the computation confirms the A120384 b-file, and A120384 is missing from the problem page's OEIS field, which is a useful thing for the comment to point out.

### 9. "Two large gaps rather than one maximal gap next to a typical one"

Fair for every row, including 510*250. Using the maximal gap known at each record's middle prime (from A005250):

```
10^7:  60,140   M=154  small/M=0.39 large/M=0.91  small = 3.7 x log(mid)
10^8:  140,108  M=220  0.49  0.64   6.0 x
10^9:  132,204  M=282  0.47  0.72   6.6 x
10^10: 142,294  M=354  0.40  0.83   6.2 x
10^11: 222,280  M=456  0.49  0.61   9.2 x
10^12: 238,362  M=540  0.44  0.67   8.7 x
10^13: 510,250  M=652  0.38  0.78   8.5 x
```

No factor is itself a maximal gap; the smaller factor is always at least 0.38 of the maximal gap at that point and 3.7 to 9.2 times the average gap. The two examples chosen are the extreme cases (most balanced and most lopsided), which is honest.

### 10. "OEIS A005250, 85 terms, complete to about 10^20"

85 terms confirmed. The Andersen/Luhn record page (pzktupel.de/RecordGaps/risinggap.php, updated 03 Sep 2026) lists rank 85, gap 1854 after 101412319996363309069, "Robert Smith; Brian Kehrig, Verification of the 85th maximum gap, 08 May 2026, 22 May 2026", and marks every larger gap from rank 86 on as "Unconfirmed"; Wikipedia (as of May 2026) says the same. Being the 85th maximal gap implies an exhaustive search past 1.014e20, so "complete to about 10^20" is a correct inference, but I found no sentence on either page stating the search limit explicitly (optional edit 5 states the fact instead of the inference).

## Required edits before posting

1. Old: `The maximal-gap table (OEIS A005250, 85 terms, complete to about 10^20) gives only the denominator; there seems to be no published table of record products of consecutive gaps, so the numerator has to be recomputed.`
   New: `The denominator is tabulated far beyond this range in OEIS A005250 (85 maximal gaps, the last after a prime near 10^20). For the numerator, OEIS A120384 (primes at which the geometric mean of the two neighbouring gaps sets a record, which is the same as a record product) lists 54 terms ending at 31587561361; they agree exactly with the 54 record products found below 10^11 here, and A120384 could be added to the OEIS links on this page. Above 10^11 only the maximum of each scanned segment was kept, so P(X) at 10^12 and 10^13 is exact but the record list is not extended.`
   Reason: A120384 exists and matches the data term for term (section 8). The current sentence is false and would be corrected by the first reader who knows the entry.

2. Old: `Some numbers, to go with the heuristic above.`
   New: `Some numbers, to go with Przemek Chojecki's Cramér heuristic in this thread.`
   Reason: the thread's default order is newest first, so the new comment will sit above Chojecki's (section 6).

## Optional edits

3. `over these seven decades` -> `over these seven values of X`. 10^7 to 10^13 spans six decades; seven rows.
4. `stays between 0.28 and 0.36 over these seven decades and drifts down slowly (0.31 to 0.35 up to 10^10, 0.28 to 0.30 from 10^11 on)` -> `stays between 0.28 and 0.36 over these seven values of X (0.31 to 0.35 up to 10^10, 0.28 to 0.30 from 10^11 on, and 0.29 at 10^6)`. The parenthetical ranges are true, but the omitted 10^6 value (0.292) sits in the "later" band, and the model's realisation spread (0.03 to 0.08) means seven points cannot show a drift. If you keep "drifts down slowly", the refined heuristic in section 7 does predict a slow approach to 1/4 from above (about 0.30 at 10^13), and you could say so in one clause.
5. `complete to about 10^20` -> `its 85th and last term is the gap 1854 after 101412319996363309069`, if edit 1 is not taken in full. States the fact without the inference.
6. After `rather than one maximal gap next to a typical one`, optionally add `(in every row the smaller of the two gaps is at least 0.38 of the largest gap known at that point, and neither gap is itself a maximal gap)`.

## What the public README (erdos-computations/e1137) must state or fix

- Cite A120384 and the 54-term agreement (section 8). The README currently makes no OEIS-records claim, so nothing there is false, but the record list "the last twelve of which are listed in table.md" should say those are the last twelve of the 54 terms of A120384.
- The annotations "(max gap so far N, ratio r)" on the seven segment records in table.md are misleading for three of them. mk1137.py takes N as the running maximum after the whole segment that holds the record, and in three segments a larger gap lies above the record's middle prime. From A005250, the largest gap below the middle prime and the ratio at the moment of the record are: 65436 at 138465682247: 468, 0.299 (listed 474, 0.291); 81320 at 220578150113: 474, 0.362 (listed 490, 0.339); 109296 at 1032148488557: 540, 0.375 (listed 588, 0.316). The other four (85860, 86156, 115056, 127500) are right. Either recompute N from A005250 (largest record with lower prime below the middle prime) or relabel the field as "largest gap in the segments up to and including this one". Repro: `python3` over `b002386.txt`/`b005250.txt`, `max(g for p,g in rec if p < mid)`.
- README says "An adversarial review ... is in REVIEW.md" and lists `REVIEW.md` under Files, but the folder has no REVIEW.md (checked the pushed tree and the local folder). Add it or drop the two references.
- lit-sweep.md line 144 (notes, not posted) should be corrected the same way as edit 1.
- Nothing else in the README contradicts the evidence: 54 `RECORD PROD` lines (counted), 40 `RECORD GAP` lines, 27 segments, the 10^4 overlaps, the gapseg-vs-gaps cross-check values, the convention paragraph, "stopped after the 10^11 row" (the log ends with the X=100000000000 line), primesieve 12.15 (installed version). "Apple M5 laptop" is not something I can verify.

## Open questions

- None demonstrable. The only unverified factual item in the draft is the phrase "complete to about 10^20" (section 10), which is a correct inference from "85th maximal gap verified" rather than a quoted statement.

## What I did not get to

- Full independent re-scan of [10^12, 10^13): of the 18 seg13 blocks I re-ran 1, 6, 9 and 13 (the blocks that hold every segment record above 10^12 and the 10^13 maxima); all four match the implementer's files exactly. The other 14 blocks are verified only through their max gaps against A005250 (27 of 27 consistent) and could hide a larger product than 127500 only if gapseg.c had a bug that my four matching blocks and the full 10^11 to 10^12 pass did not exercise. Range [10^11, 10^12) was re-scanned in full.
- The exhaustive search limit behind A005250's 85th term was not found stated in words on the record page or Wikipedia.
