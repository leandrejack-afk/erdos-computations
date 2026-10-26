// sumfull.c: numbers n <= N that are not a sum of at most r+1 r-full numbers
// (r = 2: A056828, r = 3: A392342, r = 4: A392343). n is r-full if p | n implies p^r | n; 1 is r-full.
// r-full numbers are generated prime by prime (depth-first over exponents >= r), then
// S_0 = {0}, S_{j+1} = S_j + ({0} u F) on bitsets, j < r+1.
// usage: sumfull r N
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int R; static uint64_t N;
static uint64_t *pr, npr, *F, nf, capf;

static void add(uint64_t v) { if (nf == capf) F = realloc(F, (capf = capf ? 2 * capf : 1024) * 8); F[nf++] = v; }

static void dfs(uint64_t i, uint64_t cur) {
  for (; i < npr; i++) {
    uint64_t p = pr[i], v = cur;
    int ok = 1;
    for (int e = 0; e < R; e++) { if (v > N / p) { ok = 0; break; } v *= p; }
    if (!ok) break;  // primes ascend, so no larger prime fits either
    while (1) { add(v); dfs(i + 1, v); if (v > N / p) break; v *= p; }
  }
}

static int cmpu(const void *a, const void *b) {
  uint64_t x = *(const uint64_t *)a, y = *(const uint64_t *)b; return x < y ? -1 : x > y;
}

int main(int argc, char **argv) {
  R = atoi(argv[1]); N = strtoull(argv[2], 0, 10);
  uint64_t P = 2; while (1) { uint64_t t = 1; int big = 0; for (int e = 0; e < R; e++) { if (t > N / P) { big = 1; break; } t *= P; } if (big) break; P++; }
  char *comp = calloc(P + 1, 1); pr = malloc((P + 1) * 8);
  for (uint64_t i = 2; i <= P; i++) if (!comp[i]) { pr[npr++] = i; for (uint64_t j = i * i; j <= P; j += i) comp[j] = 1; }
  add(1); dfs(0, 1);
  qsort(F, nf, 8, cmpu);
  fprintf(stderr, "r=%d N=%" PRIu64 ": %" PRIu64 " r-full numbers, largest %" PRIu64 "\n", R, N, nf, F[nf - 1]);
  uint64_t W = N / 64 + 2;
  uint64_t *A = calloc(W, 8), *B = calloc(W, 8);
  if (!A || !B) { fprintf(stderr, "out of memory\n"); return 1; }
  A[0] = 1;
  for (int j = 0; j <= R; j++) {
    memcpy(B, A, W * 8);
    for (uint64_t i = 0; i < nf; i++) {
      uint64_t a = F[i], ws = a / 64, bs = a % 64;
      if (bs == 0) { for (uint64_t k = W - 1; k >= ws && k < W; k--) B[k] |= A[k - ws]; }
      else { for (uint64_t k = W - 1; k > ws && k < W; k--) B[k] |= (A[k - ws] << bs); B[ws] |= A[0] << bs; }
    }
    uint64_t *t = A; A = B; B = t;
  }
  uint64_t nex = 0, last = 0;
  for (uint64_t n = 1; n <= N; n++)
    if (!((A[n / 64] >> (n % 64)) & 1)) { printf("%" PRIu64 "\n", n); nex++; last = n; }
  fprintf(stderr, "%" PRIu64 " exceptions in [1, N], largest %" PRIu64 "\n", nex, last);
  return 0;
}
