#!/usr/bin/env python3
"""Independent checks for Erdős problem 389 (shares no code with e389_sieve.c).

Problem: least k >= 1 with n(n+1)...(n+k-1) | (n+k)...(n+2k-1).

Three separate methods, from most definitional to most efficient:

  divides_product(n, k)   big-integer divisibility of the two products (gmpy2).
  divides_binomial(n, k)  C(2N, n-1) | C(2N, N) with N = n+k-1 (gmpy2.bincoef).
  divides_valuation(n, k) prime valuations by Legendre's formula, over the
                          prime factors of 2N, 2N-1, ..., 2N-n+2 (sympy).

The valuation method is the only one that scales to N ~ 1e13; the other two are
used to confirm it on small cases, and the product form confirms the algebra
that turns the original statement into the binomial statement.
"""
import argparse
import random
import sys

import gmpy2
from gmpy2 import mpz
from sympy import factorint


def prod_range(lo, hi):
    """Product of the integers lo, lo+1, ..., hi-1 as an mpz (balanced tree)."""
    if hi - lo <= 64:
        r = mpz(1)
        for x in range(lo, hi):
            r *= x
        return r
    mid = (lo + hi) // 2
    return prod_range(lo, mid) * prod_range(mid, hi)


def divides_product(n, k):
    left = prod_range(n, n + k)
    right = prod_range(n + k, n + 2 * k)
    return right % left == 0


def divides_binomial(n, k):
    N = n + k - 1
    a = n - 1
    return gmpy2.bincoef(2 * N, N) % gmpy2.bincoef(2 * N, a) == 0


def v_fact(x, p):
    """v_p(x!) by Legendre's formula."""
    s = 0
    while x:
        x //= p
        s += x
    return s


def divides_valuation(n, k, return_bad=False):
    """C(2N, a) | C(2N, N) checked prime by prime.

    A prime can only matter if it divides C(2N, a), hence if it divides one of
    the a consecutive integers 2N, 2N-1, ..., 2N-a+1. Those are factored fully.
    """
    N = n + k - 1
    a = n - 1
    primes = set()
    for i in range(a):
        primes.update(factorint(2 * N - i).keys())
    for p in sorted(primes):
        top = v_fact(2 * N, p)
        v_left = top - v_fact(a, p) - v_fact(2 * N - a, p)
        v_right = top - 2 * v_fact(N, p)
        if v_left > v_right:
            return (False, p) if return_bad else False
    return (True, None) if return_bad else True


def min_k_incremental(n, kmax):
    """Definitional search with incrementally maintained big products.

    left(k) = n...(n+k-1), right(k) = (n+k)...(n+2k-1); moving k -> k+1
    multiplies left by n+k and right by (n+2k)(n+2k+1)/(n+k).
    """
    left, right = mpz(n), mpz(n + 1)
    for k in range(1, kmax + 1):
        if right % left == 0:
            return k
        left *= n + k
        right = right * (n + 2 * k) * (n + 2 * k + 1) // (n + k)
    return None


def main():
    ap = argparse.ArgumentParser()
    sub = ap.add_subparsers(dest="cmd", required=True)
    s = sub.add_parser("pair", help="check one (n, k) with every applicable method")
    s.add_argument("n", type=int)
    s.add_argument("k", type=int)
    s = sub.add_parser("bruteforce", help="definitional minimal k for small n")
    s.add_argument("n", type=int)
    s.add_argument("kmax", type=int)
    s = sub.add_parser("agree", help="compare product and valuation methods on all k <= kmax")
    s.add_argument("n", type=int)
    s.add_argument("kmax", type=int)
    s = sub.add_parser("sample", help="random rejected k below kmin must fail the valuation check")
    s.add_argument("n", type=int)
    s.add_argument("kmin", type=int)
    s.add_argument("count", type=int)
    s.add_argument("--seed", type=int, default=389)
    args = ap.parse_args()

    if args.cmd == "pair":
        n, k = args.n, args.k
        N = n + k - 1
        ok, bad = divides_valuation(n, k, return_bad=True)
        print(f"n={n} k={k} N={N} valuation: {'DIVIDES' if ok else 'fails at p=' + str(bad)}")
        if k <= 20_000_000:
            print(f"  binomial: {'DIVIDES' if divides_binomial(n, k) else 'FAILS'}")
        if k <= 4_000_000:
            print(f"  product : {'DIVIDES' if divides_product(n, k) else 'FAILS'}")
    elif args.cmd == "bruteforce":
        print(f"n={args.n} min k (definitional, k<={args.kmax}): {min_k_incremental(args.n, args.kmax)}")
    elif args.cmd == "agree":
        n = args.n
        hits = []
        for k in range(1, args.kmax + 1):
            a = divides_product(n, k)
            b = divides_valuation(n, k)
            if a != b:
                print(f"DISAGREE n={n} k={k} product={a} valuation={b}")
                sys.exit(1)
            if a:
                hits.append(k)
        print(f"n={n}: product and valuation agree for all k<={args.kmax}; solutions {hits}")
    elif args.cmd == "sample":
        rng = random.Random(args.seed)
        n, kmin = args.n, args.kmin
        for _ in range(args.count):
            k = rng.randrange(1, kmin)
            ok, bad = divides_valuation(n, k, return_bad=True)
            if ok:
                print(f"UNEXPECTED: n={n} k={k} < kmin divides")
                sys.exit(1)
        print(f"n={n}: {args.count} random k < {kmin} all rejected by the valuation check")


if __name__ == "__main__":
    main()
