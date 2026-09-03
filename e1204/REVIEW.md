VERDICT: (pending, review in progress, started 2026-09-03)

Reviewer: opus-reviewer (fresh context). Budget about 40 minutes. Nothing in this folder other than
REVIEW.md is edited by the reviewer. Scratch code lives in the session scratchpad and is quoted here.

## 1. Statement from the page (tools/erdos.py 1204, fetched by the reviewer)

Verbatim: "We call a sequence of integers 0 <= a_1 < ... < a_k admissible if it is missing at least
one congruence class modulo every prime p. Let A(k) = min a_k. ... Estimate B(k) = min (a_1 + ... + a_k)/k."
Greedy sequence on the page: "a_1 = 0 and a_{i+1} is the smallest integer > a_{i-1} such that
a_1, ..., a_i is admissible" (the page's indices are garbled; the intent is A135311: a(1) = 0, a(n+1) =
least integer above a(n) keeping the tuple admissible). Page status OPEN, 0 comments, OEIS links
A008407, A023193, A135311.

Normalisation a_1 = 0: if (a_i) misses class c mod p then (a_i - a_1) misses class c - a_1 mod p, so
translation preserves admissibility; the sum drops by k*a_1 >= 0 and the tuple stays in the allowed
range (a_1 - a_1 = 0 >= 0). So the minimum is attained with a_1 = 0 and B(k) = S(k)/k. Confirmed.
Primes p > k: k integers occupy at most k < p classes, so a class is always missed. Confirmed.

