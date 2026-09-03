#!/usr/bin/env python3
"""Generate a CNF asking for a clique of size >= k in the class-r graph at N.

usage: satgen.py N r k out.cnf [--struct] [--enc totalizer|seqcounter|cardnetwrk|kmtotalizer]

Variables x_a for each odd squarefree a <= N with a == r (mod 4).  A clause
(-x_a v -x_b) for every pair with a+b not squarefree.  Cardinality
sum x_a >= k via pysat encodings.  With --struct, add selector variables
y_{p,s} for each prime p and residue s mod p^2: x_a -> y_{p, a mod p^2} and
(-y_{p,s} v -y_{p,-s}); implied constraints that expose the residue structure
to the solver.
"""
import sys
from pysat.card import CardEnc, EncType
from pysat.formula import CNF


def squarefree_table(lim):
    t = bytearray([1]) * (lim + 1)
    t[0] = 0
    p = 2
    while p * p <= lim:
        q = p * p
        for m in range(q, lim + 1, q):
            t[m] = 0
        p += 1
    return t


def primes_upto(n):
    s = bytearray([1]) * (n + 1)
    s[0] = s[1] = 0
    for i in range(2, int(n ** 0.5) + 1):
        if s[i]:
            for j in range(i * i, n + 1, i):
                s[j] = 0
    return [i for i in range(n + 1) if s[i]]


def main():
    args = sys.argv[1:]
    struct = "--struct" in args
    if struct:
        args.remove("--struct")
    enc = EncType.totalizer
    if "--enc" in args:
        i = args.index("--enc")
        enc = {"totalizer": EncType.totalizer, "seqcounter": EncType.seqcounter,
               "cardnetwrk": EncType.cardnetwrk, "kmtotalizer": EncType.kmtotalizer,
               "sortnetwrk": EncType.sortnetwrk, "mtotalizer": EncType.mtotalizer}[args[i + 1]]
        del args[i:i + 2]
    N, r, k = int(args[0]), int(args[1]), int(args[2])
    out = args[3]
    sqf = squarefree_table(2 * N + 2)
    verts = [a for a in range(1, N + 1) if a % 4 == r and sqf[a]]
    var = {a: i + 1 for i, a in enumerate(verts)}
    cnf = CNF()
    for i, a in enumerate(verts):
        for b in verts[i + 1:]:
            if not sqf[a + b]:
                cnf.append([-var[a], -var[b]])
    top = len(verts)
    if struct:
        for p in primes_upto(int((2 * N) ** 0.5) + 1):
            if p == 2:
                continue
            q = p * p
            if q > 2 * N:
                break
            ys = {}
            for s in range(1, q):
                top += 1
                ys[s] = top
            for a in verts:
                s = a % q
                if s == 0:
                    cnf.append([-var[a]])
                else:
                    cnf.append([-var[a], ys[s]])
            for s in range(1, q):
                t = q - s
                if s < t:
                    cnf.append([-ys[s], -ys[t]])
    card = CardEnc.atleast(lits=[var[a] for a in verts], bound=k, top_id=top, encoding=enc)
    cnf.extend(card.clauses)
    cnf.to_file(out)
    print(f"vertices={len(verts)} vars={cnf.nv} clauses={len(cnf.clauses)}")


if __name__ == "__main__":
    main()
