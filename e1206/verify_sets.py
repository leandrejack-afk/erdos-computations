"""Check the deletion sets logged by sidon_cpsat.py against the b-file: every n = 1..N has exactly
one logged set, its size t equals n - b(n), and it meets every 4-set {a, b, c, d}
(a^3 + b^3 = c^3 + d^3) with all elements <= n. This certifies a(n) >= b(n) without trusting the
solver. usage: verify_sets.py log.err b-file"""
import ast
import sys

rows = []
for line in open(sys.argv[1]):
    if not line[:1].isdigit() or "[" not in line:
        continue
    n, t, rest = line.split(" ", 2)
    rows.append((int(n), int(t), set(ast.literal_eval(rest))))
b = {}
for line in open(sys.argv[2]):
    n, v = map(int, line.split())
    b[n] = v
N = max(b)
bad = 0
if sorted(n for n, _, _ in rows) != list(range(1, N + 1)) or sorted(b) != list(range(1, N + 1)):
    bad += 1
    logged = {n for n, _, _ in rows}
    print("n values do not cover 1..N exactly once; missing:", sorted(set(range(1, N + 1)) - logged))
M = max([N] + [n for n, _, _ in rows])
cubes = [i ** 3 for i in range(M + 1)]
seen = {}
E = []
for d in range(2, M + 1):
    for c in range(1, d):
        s = cubes[c] + cubes[d]
        for (a, bb) in seen.get(s, []):
            E.append(((a, bb, c, d), d))
        seen.setdefault(s, []).append((c, d))
for n, t, D in rows:
    if len(D) != t or any(not 1 <= v <= n for v in D) or n - t != b.get(n):
        bad += 1; print("bad size, range or b(n) at", n); continue
    for e, mx in E:
        if mx <= n and not (set(e) & D):
            bad += 1; print("unhit 4-set", e, "at n =", n); break
print(f"checked n = 1..{N}: {len(rows)} deletion sets, {len(E)} 4-sets in all, problems: {bad}")
