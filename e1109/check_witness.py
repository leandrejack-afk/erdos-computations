#!/usr/bin/env python3
"""Independent checker for Erdős 1109 extremal sets.

Reads a records file (lines "k<TAB>N<TAB>a1 a2 ... ak") and verifies, from the
problem statement alone, that every listed set A satisfies:
  - A is a set of k distinct integers in [1, N];
  - every n in A+A (including a+a = 2a) is squarefree.
Squarefreeness is decided with sympy.factorint, which shares no code with the
solvers.  Also checks that the record sizes rise by exactly one per line.

usage: check_witness.py records_file [f_table.tsv]
If the f table (lines "N<TAB>f(N)") is given, it also checks that the table
is nondecreasing, rises by at most 1 per step, and that f(N) = k exactly on
[a(k), a(k+1)-1] for the listed records.
"""
import sys
from sympy import factorint


def squarefree(n):
    return n >= 1 and all(e == 1 for e in factorint(n).values())


def main():
    recs = []
    with open(sys.argv[1]) as fh:
        for line in fh:
            parts = line.rstrip("\n").split("\t")
            k, N = int(parts[0]), int(parts[1])
            A = [int(x) for x in parts[2].split()] if len(parts) > 2 and parts[2].strip() else []
            recs.append((k, N, A))
    bad = 0
    prev_k = 0
    for k, N, A in recs:
        if k != prev_k + 1:
            print(f"record sizes not consecutive at k={k}")
            bad += 1
        prev_k = k
        if len(A) != k or len(set(A)) != k:
            print(f"k={k} N={N}: set has {len(set(A))} distinct elements, expected {k}")
            bad += 1
            continue
        if min(A) < 1 or max(A) > N:
            print(f"k={k} N={N}: element outside [1,N]")
            bad += 1
            continue
        sums = {a + b for a in A for b in A}
        nonsf = sorted(s for s in sums if not squarefree(s))
        if nonsf:
            print(f"k={k} N={N}: non-squarefree sums {nonsf[:5]}")
            bad += 1
    print(f"witnesses checked: {len(recs)}, failures: {bad}")
    if len(sys.argv) > 2:
        f = {}
        with open(sys.argv[2]) as fh:
            for line in fh:
                N, v = map(int, line.split())
                f[N] = v
        nmax = max(f)
        tbad = 0
        for N in range(2, nmax + 1):
            if f[N] < f[N - 1] or f[N] > f[N - 1] + 1:
                print(f"table not a step function at N={N}")
                tbad += 1
        starts = {N: k for k, N, _ in recs}
        for N in range(1, nmax + 1):
            k = starts.get(N)
            if k is not None and f[N] != k:
                print(f"table f({N})={f[N]} but record says {k}")
                tbad += 1
            if N > 1 and f[N] != f[N - 1] and N not in starts:
                print(f"table jumps at N={N} without a record line")
                tbad += 1
        print(f"table checked to N={nmax}: {tbad} problems")
        bad += tbad
    sys.exit(1 if bad else 0)


if __name__ == "__main__":
    main()
