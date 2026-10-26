// sum4full.c: numbers n <= N that are not a sum of at most five 4-full numbers (OEIS A392343).
// A positive integer is 4-full if every prime factor divides it at least four times; 1 is 4-full.
// S_0 = {0}, S_{j+1} = S_j + ({0} u F) as bitsets; print the n in [1, N] outside S_5.
// usage: sum4full N
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int is4full(uint64_t n) {
  for (uint64_t p = 2; p * p * p * p <= n || p * p <= n; p++) {
    if (n % p == 0) {
      int e = 0;
      while (n % p == 0) { n /= p; e++; }
      if (e < 4) return 0;
    }
    if (p * p > n) break;
  }
  return n == 1;  // leftover prime factor (exponent 1) means not 4-full
}

int main(int argc, char **argv) {
  if (argc < 2) { fprintf(stderr, "usage: sum4full N\n"); return 1; }
  uint64_t N = strtoull(argv[1], 0, 10);
  // Every 4-full n is x^4 b^5 c^6 d^7 for some x, b, c, d >= 1 (write each exponent e >= 4 as
  // 4i + 5j + 6k + 7l), and every such product is 4-full. All products <= N are listed and
  // duplicates removed; the trial-division test below is a redundant safety check.
  uint64_t cap = 1 << 20, cnt = 0;
  uint64_t *F = malloc(cap * 8);
  for (uint64_t d = 1; d * d * d * d * d * d * d <= N; d++)
    for (uint64_t c = 1; d * d * d * d * d * d * d * c * c * c * c * c * c <= N; c++)
      for (uint64_t b = 1; (__int128)d * d * d * d * d * d * d * c * c * c * c * c * c * b * b * b * b * b <= N; b++) {
        __int128 base = (__int128)d * d * d * d * d * d * d * c * c * c * c * c * c * b * b * b * b * b;
        for (uint64_t x = 1; base * x * x * x * x <= N; x++) {
          uint64_t n = (uint64_t)(base * x * x * x * x);
          if (is4full(n)) { if (cnt == cap) F = realloc(F, (cap *= 2) * 8); F[cnt++] = n; }
        }
      }
  // dedupe
  int cmp(const void *a, const void *b);
  qsort(F, cnt, 8, (int (*)(const void *, const void *))cmp);
  uint64_t m = 0;
  for (uint64_t i = 0; i < cnt; i++) if (m == 0 || F[i] != F[m - 1]) F[m++] = F[i];
  cnt = m;
  fprintf(stderr, "%" PRIu64 " 4-full numbers <= %" PRIu64 " (largest %" PRIu64 ")\n", cnt, N, F[cnt - 1]);
  uint64_t W = N / 64 + 2;
  uint64_t *A = calloc(W, 8), *B = calloc(W, 8);
  A[0] = 1;  // S_0 = {0}
  for (int j = 0; j < 5; j++) {
    memcpy(B, A, W * 8);  // adding 0
    for (uint64_t i = 0; i < cnt; i++) {
      uint64_t a = F[i], ws = a / 64, bs = a % 64;
      if (bs == 0) for (uint64_t k = W - 1; k >= ws && k < W; k--) B[k] |= A[k - ws];
      else for (uint64_t k = W - 1; k > ws && k < W; k--) B[k] |= (A[k - ws] << bs) | (A[k - ws - 1] >> (64 - bs));
      if (bs) B[ws] |= A[0] << bs;
    }
    uint64_t *t = A; A = B; B = t;
    fprintf(stderr, "S_%d done\n", j + 1);
  }
  uint64_t nex = 0, last = 0;
  for (uint64_t n = 1; n <= N; n++)
    if (!((A[n / 64] >> (n % 64)) & 1)) { printf("%" PRIu64 "\n", n); nex++; last = n; }
  fprintf(stderr, "%" PRIu64 " exceptions, largest %" PRIu64 "\n", nex, last);
  return 0;
}
int cmp(const void *a, const void *b) {
  uint64_t x = *(const uint64_t *)a, y = *(const uint64_t *)b;
  return x < y ? -1 : x > y;
}
