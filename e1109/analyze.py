#!/usr/bin/env python3
"""Structural summary of the record extremal sets and of the growth of f(N).

usage: analyze.py records_file f_table.tsv [inc_output ...]
Prints, per record: class mod 4, residues mod 9 used (with counts), residues
mod 36 used, largest arithmetic-progression content, and f(N)/(log N)^2.
With inc outputs, prints solver runtime totals and the slowest N.
"""
import math
import sys
from collections import Counter


def ap_content(A):
    """Size of the largest subset of A lying in one residue class mod d, and
    the d attaining it, over d in {4, 12, 36, 60, 180, 900}."""
    best = (0, 0)
    for d in (4, 12, 36, 60, 180, 900):
        c = Counter(a % d for a in A)
        m = max(c.values())
        if m > best[0] or (m == best[0] and d < best[1]):
            best = (m, d)
    return best


def main():
    recs = []
    with open(sys.argv[1]) as fh:
        for line in fh:
            parts = line.rstrip("\n").split("\t")
            k, N = int(parts[0]), int(parts[1])
            A = [int(x) for x in parts[2].split()] if len(parts) > 2 and parts[2].strip() else []
            recs.append((k, N, A))
    print("k\tN\tmod4\tmod9 residues (count)\tmod36 residues\tf/(logN)^2")
    for k, N, A in recs:
        if not A:
            continue
        m4 = sorted(set(a % 4 for a in A))
        c9 = Counter(a % 9 for a in A)
        r9 = " ".join(f"{r}({c9[r]})" for r in sorted(c9))
        r36 = " ".join(str(r) for r in sorted(set(a % 36 for a in A)))
        ratio = k / math.log(N) ** 2 if N > 1 else float("nan")
        print(f"{k}\t{N}\t{m4}\t{r9}\t{r36}\t{ratio:.3f}")
    # growth table at round N
    f = {}
    with open(sys.argv[2]) as fh:
        for line in fh:
            N, v = map(int, line.split())
            f[N] = v
    print("\nN\tf(N)\tf/(log N)^2\tf/(log N)^2 loglog N")
    for N in (100, 250, 500, 700, 1000, 1103, 1500, 2000, 2500, 3000):
        if N in f:
            L = math.log(N)
            print(f"{N}\t{f[N]}\t{f[N] / L**2:.3f}\t{f[N] / (L**2 * math.log(L)):.3f}")
    # runtimes
    for path in sys.argv[3:]:
        tot = 0.0
        worst = (0.0, 0)
        cnt = 0
        with open(path) as fh:
            for line in fh:
                if line.startswith("W ") or line.startswith("DONE"):
                    continue
                parts = line.split()
                secs = float([p for p in parts if p.startswith("secs=")][0][5:])
                tot += secs
                cnt += 1
                if secs > worst[0]:
                    worst = (secs, int(parts[0]))
        print(f"\n{path}: {cnt} decisions, total {tot:.0f} s, slowest N={worst[1]} at {worst[0]:.1f} s")


if __name__ == "__main__":
    main()
