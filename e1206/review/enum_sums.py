"""Reviewer check of the reduction, straight from the definition.

Enumerate every sum x^3 + y^3 with 1 <= x <= y <= N (x = y INCLUDED), exact Python integers,
and report: collisions involving x = y, colliding representations that share an element, the
largest number of representations of one sum (and the first sum with 3), the number of
conflicting pairs of representations with max <= n for several n, and whether two different
collisions ever give the same 4-set.
usage: enum_sums.py N
"""
import sys
from collections import defaultdict

N = int(sys.argv[1])
reps = defaultdict(list)
for x in range(1, N + 1):
    for y in range(x, N + 1):
        reps[x ** 3 + y ** 3].append((x, y))

multi = {s: r for s, r in reps.items() if len(r) >= 2}
diag = [(s, r) for s, r in multi.items() if any(a == b for a, b in r)]
share = []
sets = defaultdict(list)
for s, r in multi.items():
    for i in range(len(r)):
        for j in range(i + 1, len(r)):
            u = set(r[i]) | set(r[j])
            if len(u) < 4:
                share.append((s, r[i], r[j]))
            sets[frozenset(u)].append((s, r[i], r[j]))
dup = {k: v for k, v in sets.items() if len(v) > 1}
mult = max((len(r) for r in multi.values()), default=1)
triples = sorted((s, r) for s, r in multi.items() if len(r) >= 3)
print(f"N={N}: {len(reps)} distinct sums of pairs x<=y, {len(multi)} sums with >= 2 representations")
print(f"  collisions involving some x = y: {len(diag)}")
print(f"  colliding representation pairs sharing an element: {len(share)}")
print(f"  same 4-set from two different collisions: {len(dup)}")
print(f"  largest number of representations: {mult}")
if triples:
    s, r = triples[0]
    print(f"  smallest sum with >= 3 representations: {s} = {r}, max element {max(max(p) for p in r)}")
    print(f"  sums with >= 3 representations and all parts <= N: {len(triples)}")
for n in (12, 50, 100, 120, 150, 200, 250, 300, 1000):
    if n <= N:
        cnt = sum(1 for k in sets if max(k) <= n)
        print(f"  conflicting 4-sets with max <= {n}: {cnt}")
