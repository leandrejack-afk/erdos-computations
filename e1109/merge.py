#!/usr/bin/env python3
"""Merge per-class incremental outputs of bbmc into f(N) for all N, and compare
with the OEIS b-file when given.

usage: merge.py N_max out_prefix inc_file [inc_file ...] [--bfile b392164.txt]
Writes out_prefix.tsv (N, f(N)) and out_prefix.records (k, N, witness).
"""
import sys


def parse(path):
    """Return ({N: L}, {N: witness list}) for one bbmc 'inc' output."""
    vals, wits = {}, {}
    done = False
    with open(path) as fh:
        for line in fh:
            line = line.strip()
            if not line:
                continue
            if line == "DONE":
                done = True
                continue
            if line.startswith("W "):
                head, rest = line[2:].split(":")
                wits[int(head)] = [int(x) for x in rest.split()]
                continue
            parts = line.split()
            vals[int(parts[0])] = int(parts[1])
    if not done:
        raise SystemExit(f"{path}: no DONE marker, run incomplete")
    return vals, wits


def main():
    args = sys.argv[1:]
    bfile = None
    if "--bfile" in args:
        i = args.index("--bfile")
        bfile = args[i + 1]
        del args[i:i + 2]
    nmax = int(args[0])
    prefix = args[1]
    files = args[2:]
    per_file = [parse(p) for p in files]
    f = [0] * (nmax + 1)
    witness = {}
    # f(N) = max over files of the running bound at the last processed N' <= N
    for vals, wits in per_file:
        run = 0
        keys = sorted(vals)
        j = 0
        for N in range(1, nmax + 1):
            while j < len(keys) and keys[j] <= N:
                run = max(run, vals[keys[j]])
                j += 1
            if run > f[N]:
                f[N] = run
        for N, w in wits.items():
            witness.setdefault((N, len(w)), w)
    # monotone fill (f is nondecreasing)
    for N in range(2, nmax + 1):
        if f[N] < f[N - 1]:
            f[N] = f[N - 1]
    with open(prefix + ".tsv", "w") as out:
        for N in range(1, nmax + 1):
            out.write(f"{N}\t{f[N]}\n")
    records = []
    for N in range(1, nmax + 1):
        if N == 1 or f[N] > f[N - 1]:
            records.append((f[N], N))
    with open(prefix + ".records", "w") as out:
        for k, N in records:
            w = witness.get((N, k))
            ws = " ".join(map(str, w)) if w else ""
            out.write(f"{k}\t{N}\t{ws}\n")
    print("records:", " ".join(str(N) for _, N in records))
    if bfile:
        bad = 0
        cnt = 0
        with open(bfile) as fh:
            for line in fh:
                if line.startswith("#") or not line.strip():
                    continue
                N, a = map(int, line.split())
                if N > nmax:
                    continue
                cnt += 1
                if a != f[N]:
                    bad += 1
                    print(f"MISMATCH N={N} bfile={a} ours={f[N]}")
        print(f"bfile compared: {cnt} values, {bad} mismatches")


if __name__ == "__main__":
    main()
