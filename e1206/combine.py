"""Combine the reviewer's certificates into the claim a(n) = b(n) for n = 1..300.

Upper bound: at every n where b(n) = b(n-1) (the jumps of t(n) = n - b(n)), lp_bb.py must report
CLOSED with t = n - b(n). For other n, a(n) <= a(m) + (n - m) for m < n (restrict a Sidon set to
{1..m}), and b(n) = b(m) + (n - m) back to the last jump m, so the bound carries over; before the
first jump b(n) = n. Lower bound: check_witnesses.py output (every logged set checked directly).
usage: combine.py lp_bb_output b-file"""
import re
import sys

b = {}
for line in open(sys.argv[2]):
    n, v = map(int, line.split())
    b[n] = v
N = max(b)
jumps = [n for n in range(1, N + 1) if (b[n] == b[n - 1] if n > 1 else b[n] == 0)]
closed = {}
for line in open(sys.argv[1]):
    m = re.match(r"n=(\d+) t=(\d+) .*CLOSED", line)
    if m:
        closed[int(m.group(1))] = int(m.group(2))
missing = [n for n in jumps if closed.get(n) != n - b[n]]
steps_ok = b[1] == 1 and all(b[n] - b[n - 1] in (0, 1) for n in range(2, N + 1))
# carry the upper bound forward from the last jump
ub_ok = True
last = None
for n in range(1, N + 1):
    if n in jumps:
        last = n
    bound = n if last is None else b[last] + (n - last)
    if bound != b[n]:
        ub_ok = False
print(f"jumps: {len(jumps)}; closed with the right t: {len(jumps) - len(missing)}; missing: {missing}")
print(f"b(n) - b(n-1) in {{0,1}}: {steps_ok}; upper bound carried to every n <= {N}: {ub_ok and not missing}")
