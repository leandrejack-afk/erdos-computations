#!/usr/bin/env python3
"""merge_sieve.py: combine the sieve logs (logs/sieve_5e11.txt and the completed pieces in
logs/sieve-chunks/) and compare them with the enumeration log of cand (and cand2 if present).

Prints the range covered without gaps, the number of square points counted by the sieve in it,
whether every run of length >= 10 starting in it is identical in the sieve and in the
enumerations, and the first run of each length found by the sieve.
usage: python3 merge_sieve.py
"""
import glob, os, re, sys

here = os.path.dirname(os.path.abspath(__file__))
def path(*p): return os.path.join(here, *p)

pieces = []          # (lo, hi, cands, runs, first)
def read_sieve(fn):
    lo = hi = cands = None; runs = []; first = {}; complete = False
    for line in open(fn):
        w = line.split()
        if not w: continue
        if w[0] == 'RANGE': lo, hi = int(w[1]), int(w[2])
        elif w[0] == 'CANDIDATES': cands = int(w[1])
        elif w[0] == 'RUN': runs.append((int(w[1]), int(w[2]), int(w[4])))
        elif w[0] == 'FIRST':
            complete = True
            first[int(w[2].rstrip('+'))] = (int(w[4]), int(w[6]))
    return (lo, hi, cands, runs, first) if complete else None

p = read_sieve(path('logs', 'sieve_5e11.txt'))
if p: pieces.append(p)
for fn in glob.glob(path('logs', 'sieve-chunks', '*.txt')):
    if fn.endswith('.part'): continue
    q = read_sieve(fn)
    if q: pieces.append(q)
pieces.sort()
covered_lo = 1; covered_hi = 1; cands = 0; runs = []; first = {}
for lo, hi, c, r, f in pieces:
    if lo != covered_hi:
        break                      # gap: stop at the first missing piece
    covered_hi = hi; cands += c; runs += r
    for L, (s, n) in f.items():
        if L not in first: first[L] = [s, 0]
        first[L][0] = min(first[L][0], s); first[L][1] += n
print(f"sieve covers run starts in [1, {covered_hi}) without gaps, from {len(pieces)} complete pieces")
print(f"square points counted by the sieve: {cands}")

def read_enum(fn):
    out = []
    for line in open(fn):
        w = line.split()
        if w and w[0] == 'RUN': out.append((int(w[1]), int(w[2]), int(w[4])))
    return out
ok = True
for name in ['cand_2e13_min10.txt', 'cand2_2e13_min10.txt']:
    fn = path('logs', name)
    if not os.path.exists(fn) or os.path.getsize(fn) == 0:
        print(f"{name}: missing"); continue
    e = [x for x in read_enum(fn) if x[0] < covered_hi]
    same = sorted(e) == sorted(runs)
    ok &= same
    print(f"{name}: {len(e)} runs of length >= 10 start in the covered range; identical to the sieve: {same}")
    if not same:
        print("  only in enumeration:", sorted(set(e) - set(runs))[:10])
        print("  only in sieve:", sorted(set(runs) - set(e))[:10])
for L in sorted(first):
    if L >= 9: print(f"FIRST len {L} start {first[L][0]} count {first[L][1]}")
sys.exit(0 if ok else 1)
