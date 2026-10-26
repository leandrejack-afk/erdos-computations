"""Check that 2a^3 = c^3 + d^3 has no solution with c < d <= N (so sums x + x never collide)."""
import sys
import numpy as np
N = int(sys.argv[1])
a = np.arange(1, N + 1, dtype=np.int64)
A, B = np.triu_indices(N, k=1)
S = np.sort(a[A] ** 3 + a[B] ** 3)
D = 2 * a ** 3
hits = np.isin(D, S)
print(f"N={N}: {int(hits.sum())} values 2a^3 (a <= N) equal to a sum c^3 + d^3 with c < d <= N")
