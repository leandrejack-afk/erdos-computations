#!/usr/bin/env python3
"""Third-party cross-check: the OEIS A392164 Python program (Chai Wah Wu),
verbatim apart from the harness, run at the given N values."""
import sys, time
from itertools import combinations
from networkx import empty_graph, find_cliques
from sympy import factorint

def A392164(n):
    def is_squarefree(n): return max(factorint(n).values(),default=1)<2
    v = [e for e in list(range(1,n+1)) if is_squarefree(2*e)]
    G = empty_graph(v)
    G.add_edges_from((a,b) for a, b in combinations(v,2) if is_squarefree(a+b))
    return max(len(c) for c in find_cliques(G))

for N in map(int, sys.argv[1:]):
    t0 = time.time()
    print(N, A392164(N), f"{time.time()-t0:.1f}s", flush=True)
