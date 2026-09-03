#!/usr/bin/env python3
"""Cross-check e389_sieve survivor verdicts with the independent valuation method.

Survivor file lines look like
    N ok:bad ok:bad ...
with one ok:bad pair per n' in [n, nmax]; ok is 1 (divides), 0 (fails, bad is
the failing prime) or -1 (not checked because N was already past the best
solution for that n' or N <= n'-1).

For every sampled line and every checked n', this script recomputes the
verdict from scratch (sympy factorisation + Legendre) and also confirms that
the sieve's reported failing prime really fails.
"""
import argparse
import random
import sys

sys.path.insert(0, __file__.rsplit("/", 1)[0])
from e389_check import divides_valuation, v_fact  # noqa: E402


def prime_fails(N, a, p):
    top = v_fact(2 * N, p)
    return top - v_fact(a, p) - v_fact(2 * N - a, p) > top - 2 * v_fact(N, p)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("file")
    ap.add_argument("n", type=int, help="first n of the run (even)")
    ap.add_argument("--sample", type=int, default=0, help="0 = all lines")
    ap.add_argument("--seed", type=int, default=389)
    args = ap.parse_args()

    lines = [ln.split() for ln in open(args.file) if ln.strip()]
    if args.sample and args.sample < len(lines):
        rng = random.Random(args.seed)
        lines = rng.sample(lines, args.sample)
    checked = 0
    divides = 0
    for parts in lines:
        N = int(parts[0])
        for idx, tok in enumerate(parts[1:]):
            ok, bad = tok.split(":")
            ok, bad = int(ok), int(bad)
            if ok == -1:
                continue
            nprime = args.n + idx
            a = nprime - 1
            k = N - a
            got, gotbad = divides_valuation(nprime, k, return_bad=True)
            if got != (ok == 1):
                print(f"MISMATCH N={N} n'={nprime}: sieve={ok} python={got} (python bad prime {gotbad})")
                sys.exit(1)
            if ok == 0 and not prime_fails(N, a, bad):
                print(f"MISMATCH N={N} n'={nprime}: sieve says p={bad} fails but it does not")
                sys.exit(1)
            checked += 1
            divides += ok == 1
    print(f"{args.file}: {len(lines)} lines, {checked} (N, n') verdicts re-derived independently, "
          f"all agree; {divides} of them divide")


if __name__ == "__main__":
    main()
