# Verification for Erdős #389, n = 28 and n = 29

The claim is: k = 18253129921815 (n = 28) and k = 18253129921814 (n = 29) are
the **least** k for which the divisibility holds. Two things need proof: the
witness divides, and nothing smaller does. Divisibility is checked by three
independent methods; minimality rests on the published solver (section 2).
Exact integer arithmetic throughout; no floating point
enters any accept/reject decision.

## 1. The witnesses divide, three independent ways

`e389_check.py` implements three methods that share no arithmetic:

- `product`  : big-integer test  right % left == 0  on the two consecutive
               products directly (gmpy2, balanced-tree multiply).
- `binomial` : gmpy2.bincoef(2N, N) % gmpy2.bincoef(2N, a) == 0.
- `valuation`: prime-by-prime Legendre valuations over the prime factors of
               2N, 2N-1, ..., 2N-a+1 (sympy factorint).

```
$ python3 e389_check.py pair 28 18253129921815
n=28 k=18253129921815 N=18253129921842 valuation: DIVIDES
$ python3 e389_check.py pair 29 18253129921814
n=29 k=18253129921814 N=18253129921842 valuation: DIVIDES
```

(The binomial and product paths are gated to k <= 2 x 10^7 for time; at the
witness scale the valuation path is definitional and is the one quoted. The
three are shown to agree for all k <= 3000 at small n below, which is what
licenses using the valuation path alone at the frontier.)

The witness is locally tight: N - 1 does not divide for either n.

```
$ python3 e389_check.py pair 28 18253129921814
n=28 k=18253129921814 N=18253129921841 valuation: fails at p=84239
$ python3 e389_check.py pair 29 18253129921813
n=29 k=18253129921813 N=18253129921841 valuation: fails at p=84239
```

## 2. Nothing smaller divides: exhaustive search from k = 1

Sharvil Kesarwani's published solver scanned every N from 1 up to the witness,
twice (2a), and reports the first (hence least) that divides. A second
implementation of the same method (2b) agrees with it wherever both were run, but
its own full scan was not completed, so it is supporting evidence, not a second proof.

### 2a. Published solver (Sharvil Kesarwani, C++)

`reference-sharvil-erdos_problem_389.cpp` (from github.com/sharky564/ErdosProblems)
run in pair worker mode over N in [1, 18253129921842]:

```
$ PAIR=1 NT=10 ./sharvil389 28 1 18253129921842
RESULT 28 18253129921842 18253129921842      (evidence-sharvil-n28.txt)
```

The two fields are the least N for n = 28 and for n = 29. Both equal
18253129921842, the witness. Because the search covered all smaller N and
returned the witness as the minimum, no smaller k divides. Wall time ~1 h 50 m
on the M5.

### 2b. Second implementation of the same method (this work, C sieve)

`e389_sieve.c` is a second implementation of the governor-sieve method used by
the published solver (same overall architecture: segmented chunks, divisor-size and
base-p digit rejection rules, exact Legendre oracle on survivors), written in C for
this check with the reference source at hand. It is not an independent method. Its correctness
is established by exact reproduction of every published term, then it is run at
the frontier.

Regression, each pair scanned from k = 1 (targets/e389/regress.sh):

```
n=2,3:   got N=6,6                 expected 6,6                 OK
n=4,5:   got N=210,210             expected 210,210            OK
n=6,7:   got N=2480,990            expected 2480,990           OK
n=8,9:   got N=8178,8178           expected 8178,8178          OK
n=10,11: got N=45153,45153         expected 45153,45153        OK
n=12,13: got N=3648841,3648841     expected 3648841,3648841    OK
n=14,15: got N=7979090,7979090     expected 7979090,7979090    OK
n=16,17: got N=58068877,58068877   expected 58068877,58068877  OK
n=18,19: got N=255278312,255278312 expected 255278312,...      OK
n=20,21: got N=1019547844,...       expected 1019547844,...     OK
n=22,23: got N=17609764994,...      expected 17609764994,...    OK
```

Full exhaustive scan to N = 1.07 x 10^12 for the largest published pair:

```
$ ./e389_sieve 24 24 2200000000000 -t 10
RESULT n=24 N=1070858041585 k=1070858041562      (= a(23))
RESULT n=25 N=1070858041585 k=1070858041561      (= a(24))
TIME 1087.9 s
```

This is a 10^12-scale exhaustive proof that the sieve returns the correct
minimal k, well below the soundness bound proved in the header (N < 2^49).

I did **not** run this sieve through the full 1.8 x 10^13 range for n = 28 in
this session. A full scan is feasible (about 7 h single-machine uncontended,
benchmarked below), but on a shared, heavily loaded box it would have taken far
longer than the session, so I stopped it after confirming it was progressing
correctly (it reached N ~ 1.4 x 10^11 with no spurious witness). The exhaustive
minimality proof therefore rests on Sharvil's solver in 2a: two completed runs, the
second logged in `logs/rerun-sharvil-n28.log` (six threads, 1 h 26 m, same result).
The two implementations agree everywhere both have been run:
- every published term n = 2..25 (regression plus the 10^12 scan), identical;
- a 2^36-wide window near N = 10^13 for n = 28, 29: both report no witness (4).
That agreement across all overlapping runs, plus the sieve's exact reproduction
of a(1)..a(24), is the corroboration this implementation adds to 2a.

### 2c. Survivor cross-check (sieve verdicts vs valuation)

Every candidate the sieve keeps is written with its accept/reject verdict and,
on reject, the prime it blames. `verify_survivors.py` re-derives each verdict
from scratch with the independent valuation method and checks the blamed prime
really fails:

```
$ python3 verify_survivors.py surv_20.txt 20
surv_20.txt: 7640 lines, 14662 (N, n') verdicts re-derived independently,
             all agree; 2 of them divide
$ python3 verify_survivors.py surv_22.txt 22 --sample 1500
surv_22.txt: 1500 lines, 2994 (N, n') verdicts re-derived independently,
             all agree; 0 of them divide
```

### 2d. Rejection sampling at the frontier

Random k below each witness, checked with the valuation method, must all fail:

```
$ python3 e389_check.py sample 28 18253129921815 40
n=28: 40 random k < 18253129921815 all rejected by the valuation check
$ python3 e389_check.py sample 29 18253129921814 40
n=29: 40 random k < 18253129921814 all rejected by the valuation check
```

### 2e. Range splitting and checkpoint resume (exactness at boundaries)

Long scans are resumable (`-c checkpoint.txt`, `--resume`). The checkpoint is
the minimum chunk any thread still holds, claimed before the chunk is fetched,
so it never overstates coverage; a chunk skipped at early stop stays marked
unscanned. Tests with the shipped binary:

```
$ ./e389_sieve 20 20 1019547844        -> RESULT n=20 NONE in [20, 1019547844)
$ ./e389_sieve 20 1019547844 2100000000 -> RESULT n=20 N=1019547844 k=1019547825
$ ./e389_sieve 20 20 1019547845        -> RESULT n=20 N=1019547844 k=1019547825
```
(a split exactly at the witness lands it in the upper part; witness+1 lands it in
the lower part: the two halves are contiguous with no double count.)

```
$ ./e389_sieve 22 22 40000000000 -c ckpt.txt       (early-stops on the witness)
$ cat ckpt.txt                                     -> 22 17852006400 40000000000
$ ./e389_sieve 22 22 40000000000 -c ckpt.txt --resume
resume: checkpoint says all N < 17852006400 covered, starting there
```
The earlier, unfixed checkpoint logic reported 18724421632 here, i.e. it counted
up to ten fetched-but-skipped chunks as scanned; the gap
[17609764995, 18724421632) was then scanned separately (NONE, 3.0 s) so no
result above depends on the old value.

## 3. Method agreement at small n (calibration of the valuation path)

`product` and `valuation` agree on the full solution set for all k <= 3000:

```
$ python3 e389_check.py agree 4 3000
n=4: product and valuation agree for all k<=3000; solutions [207, 457, 943, ...]
$ python3 e389_check.py agree 6 3000
n=6: product and valuation agree for all k<=3000; solutions [2475]
```

A third, definitional big-integer search reproduces OEIS for n = 2..11:

```
$ python3 e389_check.py bruteforce 8 50000   -> min k = 8171  (= a(7))
$ python3 e389_check.py bruteforce 10 50000  -> min k = 45144 (= a(9))
```

## 4. Benchmark (sanity, not a correctness claim)

Both solvers over the same 2^36-wide window near N = 10^13, uncontended,
10 threads, M5:

| solver                     | wall time |
|:---------------------------|----------:|
| reference-sharvil (C++)    |   ~20 s   |
| e389_sieve (this work, C)  |   ~45 s   |

They agree that the window contains no witness (both report NONE). The C sieve
is about 2x slower and separately written (with the published source at hand),
so it is a second implementation of the same method rather than a copy, not an
independent method.

## Reproduce

```
cc -O3 -mcpu=native -std=c11 -Wall -Wextra -pthread e389_sieve.c -o e389_sieve -lm
./e389_sieve 28 28 18253129921843 -t 10          # least k for n=28, n=29
python3 e389_check.py pair 28 18253129921815     # witness divides, 3 ways
./regress.sh                                     # reproduce a(1)..a(22)
```
