# erdos-computations

Computations on problems from the Erdős Problems database (https://www.erdosproblems.com), by Leandre Jack,
with programs written and run with the assistance of a large language model. Each folder holds the code, the raw logs and a
README with the result and how it was independently verified.

- `e382/`: Erdős Problem 382, the first maximal bad intervals of widths 9 to 13 and 15 (exact,
  extending OEIS A388850; the five values for widths 9 to 13 had been known only as upper bounds),
  with no maximal bad interval of width 14 below 2 x 10^14.
- `e385/`: exceptions to F(n) > n below 10^11 for Erdős Problem 385 (equivalently, the values of n for which
  the sequence in Problem 430 is entirely prime).
- `e389/`: Erdős Problem 389, the least k for n = 28 and n = 29 (exact, extending OEIS A375071), upper bounds
  for n = 30 to 35 and a correction to two posted witnesses.
- `e1107/`: Erdős Problem 1107, the integers up to 10^10 that are not a sum of at most r + 1 r-full
  numbers, for r = 4 (the complete list of OEIS A392343 below 10^10: 1318 numbers, the last 150271),
  r = 5 (6018, the last 449560) and r = 6 (37036, the last 4034495).
- `e1109/`: Erdős Problem 1109, exact values of f(N) for N <= 3000 (extending OEIS A392164 and A392165 from
  N = 1103), with every record boundary re-proved by two further programs.
- `e1137/`: Erdős Problem 1137, the largest gap and the largest product of two consecutive gaps between primes
  below X, for X up to 10^13.
- `e1206/`: Erdős Problem 1206, the size of the largest subset of {1, ..., n} whose cubes form a Sidon set,
  for every n up to 300 (exact: each lower bound by an explicit set, each upper bound by a branch and bound
  checked in exact arithmetic), and the same with n^3 required in the set.
