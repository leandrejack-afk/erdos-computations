#!/usr/bin/env python3
"""Cross-check every run file for Erdős problem 1204 and print the agreement record plus the table.

Reads runs/*.txt produced by smin (element branch and bound), rvs (missed-class search) and
verify_sat.py (MaxSAT); parses "k=.. S=.." lines and the "tuple:" lines that follow. Every tuple is
re-checked here for admissibility and sum. Reports, for each pair of runs, the range of k covered
by both and whether values and tuple sets agree. Then prints S(k), B(k), A(k) (A008407), and
B(k)/(k log k) for the k covered by the primary run, and writes S(k) as a b-file.
Usage: crosscheck.py [--table-every N] [--bfile-max K]   (b-file limited to K terms, default all)
"""
import glob
import math
import os
import re
import sys
from fractions import Fraction

HERE = os.path.dirname(os.path.abspath(__file__))
A_FILE = os.path.join(HERE, "..", "..", "scratch", "b008407.txt")


def primes_upto(n):
    return [p for p in range(2, n + 1) if all(p % d for d in range(2, int(p ** 0.5) + 1))]


def admissible(t):
    return all(len({x % p for x in t}) < p for p in primes_upto(len(t)))


def parse(path):
    runs = {}
    cur = None
    for line in open(path):
        m = re.match(r"k=(\d+) S=(\d+)", line)
        if m:
            cur = int(m.group(1))
            runs[cur] = {"S": int(m.group(2)), "tuples": [], "line": line.strip()}
            continue
        if line.startswith("  tuple:") and cur is not None:
            runs[cur]["tuples"].append(tuple(int(v) for v in line.split()[1:]))
    return runs


def check_tuples(name, runs):
    bad = 0
    for k, r in runs.items():
        for t in r["tuples"]:
            if not (len(t) == k and t[0] == 0 and sum(t) == r["S"] and admissible(t)):
                bad += 1
                print(f"  BAD tuple in {name} k={k}: {t}")
    return bad


def main():
    every = 1
    if "--table-every" in sys.argv:
        every = int(sys.argv[sys.argv.index("--table-every") + 1])
    bfile_max = None
    if "--bfile-max" in sys.argv:
        bfile_max = int(sys.argv[sys.argv.index("--bfile-max") + 1])
    files = sorted(glob.glob(os.path.join(HERE, "runs", "*.txt")))
    runs = {}
    for f in files:
        name = os.path.basename(f)
        if name.startswith("greedy"):
            continue
        r = parse(f)
        if r:
            runs[name] = r
    print("== run files (k range, tuples re-verified)")
    for name, r in runs.items():
        ks = sorted(r)
        bad = check_tuples(name, r)
        ntup = sum(len(x["tuples"]) for x in r.values())
        print(f"  {name}: k={ks[0]}..{ks[-1]} ({len(ks)} values, {ntup} tuples listed, {bad} bad)")
    print("== pairwise agreement")
    names = list(runs)
    for i in range(len(names)):
        for j in range(i + 1, len(names)):
            a, b = runs[names[i]], runs[names[j]]
            common = sorted(set(a) & set(b))
            if not common:
                continue
            vdiff = [k for k in common if a[k]["S"] != b[k]["S"]]
            tdiff = [k for k in common if a[k]["tuples"] and b[k]["tuples"] and set(a[k]["tuples"]) != set(b[k]["tuples"])]
            print(f"  {names[i]} vs {names[j]}: k={common[0]}..{common[-1]}, value mismatches {vdiff or 'none'}, tuple-set mismatches {tdiff or 'none'}")
    # consensus values: every run that covers k must agree
    S = {}
    for name, r in runs.items():
        for k, x in r.items():
            if k in S and S[k] != x["S"]:
                raise SystemExit(f"CONFLICT at k={k}: {name} says {x['S']} vs {S[k]}")
            S[k] = x["S"]
    A = {}
    for line in open(A_FILE):
        n, v = line.split()
        A[int(n)] = int(v)
    kmax = max(S)
    print(f"== consensus S(k) for k=1..{kmax} (no conflicts)")
    greedy = {}
    seq, x = [0], 0
    while len(seq) < kmax:
        x += 1
        if admissible(seq + [x]):
            seq.append(x)
    first_nongreedy = None
    for k in range(1, kmax + 1):
        greedy[k] = sum(seq[:k])
        if first_nongreedy is None and S[k] < greedy[k]:
            first_nongreedy = k
    print(f"   first k with S(k) < greedy sum: {first_nongreedy}")
    back_to_greedy = [k for k in range(first_nongreedy or kmax + 1, kmax + 1) if S[k] == greedy[k]]
    print(f"   k >= {first_nongreedy} with S(k) = greedy sum: {back_to_greedy or 'none'}")
    # uniqueness: the smin and rvs runs keep ties, so their distinct tuple count is the number of optima
    maxopt = 0
    for name, r in runs.items():
        if name.startswith("sat"):
            continue
        for k, x in r.items():
            maxopt = max(maxopt, len(set(x["tuples"])))
    print(f"   largest number of distinct optimal tuples at any k in the tie-keeping runs: {maxopt}")
    sumA = 0
    print("k S(k) B(k) A(k) B(k)/(k log k) S(k)-sumA(j) greedy-S(k)")
    for k in range(1, kmax + 1):
        sumA += A[k]
        B = Fraction(S[k], k)
        ratio = float(B) / (k * math.log(k)) if k > 1 else float("nan")
        if k % every == 0 or k <= 10 or k == kmax or k == first_nongreedy:
            print(f"{k} {S[k]} {B} {A[k]} {ratio:.4f} {S[k] - sumA} {greedy[k] - S[k]}")
    kb = kmax if bfile_max is None else min(kmax, bfile_max)
    with open(os.path.join(HERE, "b-file-S.txt"), "w") as fp:
        for k in range(1, kb + 1):
            fp.write(f"{k} {S[k]}\n")
    print(f"wrote b-file-S.txt with {kb} terms")
    with open(os.path.join(HERE, "S-all-runs.txt"), "w") as fp:
        for k in range(1, kmax + 1):
            fp.write(f"{k} {S[k]}\n")
    print(f"wrote S-all-runs.txt with {kmax} terms (every k any run reached)")


if __name__ == "__main__":
    main()
