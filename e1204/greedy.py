#!/usr/bin/env python3
"""Greedy admissible sequence (Erdős problem 1204, OEIS A135311): a(1) = 0 and a(n+1) is the least
integer above a(n) such that a(1..n+1) misses a class modulo every prime (only p <= n+1 can matter).
Prints n a(n) for n = 1..NMAX, b-file style. Usage: greedy.py NMAX
"""
import sys


def primes_upto(n):
    return [p for p in range(2, n + 1) if all(p % d for d in range(2, int(p ** 0.5) + 1))]


def greedy(nmax):
    primes = primes_upto(nmax)
    covered = {p: set() for p in primes}   # residues hit so far, by prime
    seq = [0]
    for p in primes:
        covered[p].add(0)
    x = 0
    while len(seq) < nmax:
        x += 1
        n = len(seq) + 1
        # x is allowed if for every prime p <= n it does not complete the residue system mod p
        ok = True
        for p in primes:
            if p > n:
                break
            if len(covered[p]) == p - 1 and (x % p) not in covered[p]:
                ok = False
                break
        if ok:
            seq.append(x)
            for p in primes:
                covered[p].add(x % p)
    return seq


if __name__ == "__main__":
    nmax = int(sys.argv[1]) if len(sys.argv) > 1 else 500
    for i, a in enumerate(greedy(nmax), 1):
        print(i, a)
