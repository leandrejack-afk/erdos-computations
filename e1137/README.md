# Erdős Problem 1137: record products of two consecutive prime gaps, to 10^13

Problem: https://www.erdosproblems.com/1137. With d_n = p_{n+1} - p_n, is it true that

    max_{n<x} d_n d_{n-1} / (max_{n<x} d_n)^2  ->  0   as x -> infinity?

The forum thread carries the Cramér-model heuristic that the ratio should instead tend to a positive
constant near 1/4. This folder holds the numbers. For a bound X let M(X) be the largest gap between
consecutive primes below X and P(X) the largest product of two consecutive prime gaps below X; a
pair of consecutive gaps is identified by its middle prime, the prime between the two gaps.

## Result

| X | M(X), after the prime | P(X) = g1 * g2, middle prime | P(X) / M(X)^2 |
|---|---|---|---|
| 10^6 | 114 (after 492113) | 3800 = 100 * 38 (396833) | 0.292 |
| 10^7 | 154 (after 4652353) | 8400 = 60 * 140 (8917523) | 0.354 |
| 10^8 | 220 (after 47326693) | 15120 = 140 * 108 (72546283) | 0.312 |
| 10^9 | 282 (after 436273009) | 26928 = 132 * 204 (476956933) | 0.339 |
| 10^10 | 354 (after 4302407359) | 41748 = 142 * 294 (9085929179) | 0.333 |
| 10^11 | 464 (after 42652618343) | 62160 = 222 * 280 (31587561361) | 0.289 |
| 10^12 | 540 (after 738832927927) | 86156 = 238 * 362 (883442069849) | 0.295 |
| 10^13 | 674 (after 7177162611713) | 127500 = 510 * 250 (5061226833937) | 0.281 |

`table.md` is the same table with the list of record products. Below 10^11 the log holds every
record-breaking product (54 `RECORD PROD` lines in `logs/gaps_1e13.log`). Their middle primes are
exactly the 54 terms of OEIS A120384 (Ken Takusagawa, 2006: primes at which the geometric mean of
the two neighbouring gaps sets a record, which is the same event as a record product), term for
term, ending at 31587561361; `table.md` lists the last twelve. So the record sequence below 10^11
was already in the OEIS, and this folder confirms it. Above 10^11 only the maximum of each segment
was recorded, and `table.md` lists the segment maxima that beat the running maximum; that list is
therefore not the complete record sequence above 10^11, while P(X) at X = 10^12 and 10^13 is exact
(it is the maximum over the segments). Nothing is claimed beyond 10^13.

## Method

- `gaps.c`: one pass over all primes with the primesieve iterator, tracking the running largest gap
  and largest product of two consecutive gaps, printing the state at every power of ten and a line
  at every new record. The run `./gaps 10000000000000` was stopped after the 10^11 row; its output
  is `logs/gaps_1e13.log`.
- `gapseg.c`: for a range [A, B], the largest gap whose lower prime lies in [A, B] and the largest
  product of two consecutive gaps whose middle prime lies in [A, B]. It iterates from A - 5000 to
  B + 5000 so that the gaps at both ends of the segment are seen (every gap below 10^13 is far
  smaller than 5000). The range from 10^11 to 10^13 was covered by 27 segments run in parallel:
  nine of width 10^11 for [10^11, 10^12) (`segs/seg12_*.txt`, the first starting at 10^11 - 10^4)
  and eighteen of width 5 x 10^11 for [10^12, 10^13) (`segs/seg13_*.txt`, the first starting at
  10^12 - 10^4). The 10^4 overlaps at 10^11 and 10^12 cannot change a maximum.
- `mk1137.py`: combines the log rows up to 10^11 with the segment maxima into `table.md`.

Conventions: for X <= 10^11 a gap or a pair counts when all its primes are below X (the state is
printed before the first prime above X is processed). For X = 10^12 and 10^13 a gap counts when its
lower prime is below X and a pair when its middle prime is below X. The record gaps and pairs in
the table are far from 10^12 and 10^13, so the two conventions give the same rows.

## Verification

Checks run on 2026-09-04, before posting:

- Every M(X) row equals the maximal prime gap below X in OEIS A005250 (gaps) and A002386 (the
  primes they follow), b-files fetched the same day: eight rows, eight matches, from 114 after
  492113 to 674 after 7177162611713.
- Coverage: the 27 segment ranges are contiguous from 10^11 - 10^4 to 10^13 - 1 (checked by script
  over the `SEG A B` headers of the segment files).
- `gapseg.c` against `gaps.c` below 10^9: `gapseg 0 499999999` returns max gap 282 after 436273009
  and max product 26928 = 132 * 204 (middle prime 476956933), `gapseg 500000000 999999999` returns
  276 after 649580171 and 23460 = 138 * 170 (middle prime 879353381); the maxima over the two halves
  are exactly the single scan's 10^9 row. Recompiling `gaps.c` and rerunning `./gaps 1000000000`
  reproduces the log rows to 10^9 verbatim (0.3 s).
- The ratios recompute from the M and P columns; `python3 mk1137.py` regenerates `table.md`
  byte for byte.
- An adversarial review with fresh context (`REVIEW.md`, 2026-09-04) reproduced every row from 10^6
  to 10^11 with its own scanner, re-scanned [10^11, 10^12) in one pass and four of the eighteen
  segments of [10^12, 10^13) (the ones holding every record above 10^12 and the 10^13 maxima), all
  agreeing; checked the other fourteen segments only through their largest gaps against A005250;
  found the A120384 match; and pointed out that the running-maximum annotations on the segment
  records in the first version of `table.md` were misleading, which is why that list now carries
  no ratios.

## Files

- `gaps.c`, `gapseg.c`: the two scanners (C, primesieve). `mk1137.py`: builds `table.md` from the
  log and the segment files.
- `logs/gaps_1e13.log`: the single scan (every record gap and record product below 10^11, and the
  state at each power of ten). `segs/`: the 27 segment results, one line each.
- `table.md`: the table and the record list. `REVIEW.md`: the adversarial review.

## Reproduce

    cc -O2 -I/opt/homebrew/include -L/opt/homebrew/lib gaps.c -lprimesieve -o gaps
    cc -O2 -I/opt/homebrew/include -L/opt/homebrew/lib gapseg.c -lprimesieve -o gapseg
    ./gaps 100000000000 > gaps_1e11.log          # the rows to 10^11 (about a minute)
    ./gapseg 200000000000 299999999999           # one segment; segs/ lists the 27 ranges used
    python3 mk1137.py > table.md                 # from logs/gaps_1e13.log and segs/

Scans run 2026-09-03 and checks 2026-09-04 on an Apple M5 laptop with primesieve 12.15. Programs
written and run with the assistance of a large language model; every number above comes from the executed runs.
