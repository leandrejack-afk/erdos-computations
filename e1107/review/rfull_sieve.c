// rfull_sieve.c (review): list every r-full n <= N by testing EVERY integer 1..N, segment by segment.
// n is r-full iff n == 1 or every prime p | n has p^r | n. Such n has all primes <= N^(1/r), so for each
// prime p <= N^(1/r): a multiple of p that is not a multiple of p^r is marked bad; a multiple of p^r
// gets prod *= p^v_p(n). Then n is r-full iff !bad[n] && prod[n] == n (no prime factor left over).
// usage: rfull_sieve r N > list.txt
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
  if (argc < 3) { fprintf(stderr, "usage: rfull_sieve r N\n"); return 2; }
  int r = atoi(argv[1]);
  uint64_t N = strtoull(argv[2], 0, 10);
  // P = floor(N^(1/r)) by exact integer search
  uint64_t P = 1;
  for (;;) {
    uint64_t q = P + 1, t = 1; int over = 0;
    for (int e = 0; e < r; e++) { if (t > N / q) { over = 1; break; } t *= q; }
    if (over) break;
    P = q;
  }
  char *comp = calloc(P + 2, 1);
  uint64_t *pr = malloc((P + 2) * sizeof *pr), np = 0;
  for (uint64_t i = 2; i <= P; i++) if (!comp[i]) { pr[np++] = i; for (uint64_t j = i * i; j <= P; j += i) comp[j] = 1; }
  uint64_t *ppr = malloc((np + 1) * sizeof *ppr);  // p^r (<= N by choice of P)
  for (uint64_t k = 0; k < np; k++) { uint64_t t = 1; for (int e = 0; e < r; e++) t *= pr[k]; ppr[k] = t; }
  const uint64_t SEG = 1u << 22;
  uint64_t *prod = malloc(SEG * sizeof *prod);
  unsigned char *bad = malloc(SEG);
  uint64_t count = 0, last = 0;
  for (uint64_t L = 1; L <= N; L += SEG) {
    uint64_t len = (N - L + 1 < SEG) ? N - L + 1 : SEG;
    for (uint64_t i = 0; i < len; i++) prod[i] = 1;
    memset(bad, 0, len);
    for (uint64_t k = 0; k < np; k++) {
      uint64_t p = pr[k], q = ppr[k];
      uint64_t m0 = ((L + p - 1) / p) * p;
      for (uint64_t m = m0; m < L + len; m += p) if (m % q) bad[m - L] = 1;
      m0 = ((L + q - 1) / q) * q;
      for (uint64_t m = m0; m < L + len; m += q) {
        uint64_t t = m, pe = 1;
        while (t % p == 0) { t /= p; pe *= p; }
        prod[m - L] *= pe;
      }
    }
    for (uint64_t i = 0; i < len; i++)
      if (!bad[i] && prod[i] == L + i) { printf("%" PRIu64 "\n", L + i); count++; last = L + i; }
  }
  fprintf(stderr, "r=%d N=%" PRIu64 " P=%" PRIu64 " primes=%" PRIu64 ": %" PRIu64 " r-full, largest %" PRIu64 "\n",
          r, N, P, np, count, last);
  return 0;
}
