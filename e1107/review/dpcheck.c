// dpcheck.c (review): exact minimum number of r-full summands by the coin-change DP. No bitset sumsets.
// usage: dpcheck r N listfile [N2]
//   listfile: ascending r-full numbers (1 included), complete up to max(N, N2).
// m[0] = 0, m[n] = CAP (n > 0); for each r-full a: for n = a..N ascending, m[n] = min(m[n], m[n-a] + 1).
// This is the unbounded coin-change recurrence, so afterwards m[n] = min number of positive r-full
// summands of n (exact while < CAP). Exceptions: n in [1, N] with m[n] > r + 1, printed to stdout.
// Optional tail (N2 > N): S_k = {x <= N : m[x] <= k} as bitsets; every n in (N, N2] is tested for
// n = a + x (a r-full, x in S_r); leftovers are tested for n = a + b + x (x in S_{r-1}); anything still
// left is printed as "UNRESOLVED n" (this is a sufficient test only).
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CAP 250

static void relax(unsigned char *restrict dst, const unsigned char *restrict src, size_t len) {
  for (size_t i = 0; i < len; i++) { unsigned char v = (unsigned char)(src[i] + 1); dst[i] = v < dst[i] ? v : dst[i]; }
}

static uint64_t *F; static size_t nf;

static void readlist(const char *path) {
  FILE *f = fopen(path, "r");
  if (!f) { perror(path); exit(1); }
  size_t cap = 1024; F = malloc(cap * sizeof *F); nf = 0;
  unsigned long long v;
  while (fscanf(f, "%llu", &v) == 1) {
    if (nf == cap) F = realloc(F, (cap *= 2) * sizeof *F);
    if (nf && v <= F[nf - 1]) { fprintf(stderr, "list not strictly ascending at %llu\n", v); exit(1); }
    F[nf++] = v;
  }
  fclose(f);
}

// 64 bits of S starting at bit offset off (off >= 0); S has zero padding words past the last used bit.
static inline uint64_t get64(const uint64_t *S, uint64_t off) {
  uint64_t w = off >> 6, b = off & 63;
  return b ? (S[w] >> b) | (S[w + 1] << (64 - b)) : S[w];
}

int main(int argc, char **argv) {
  if (argc < 4) { fprintf(stderr, "usage: dpcheck r N listfile [N2]\n"); return 2; }
  int r = atoi(argv[1]);
  uint64_t N = strtoull(argv[2], 0, 10);
  uint64_t N2 = argc > 4 ? strtoull(argv[4], 0, 10) : N;
  readlist(argv[3]);
  if (nf == 0 || F[0] != 1) { fprintf(stderr, "list must start with 1\n"); return 1; }
  size_t ncoin = 0; while (ncoin < nf && F[ncoin] <= N) ncoin++;
  fprintf(stderr, "r=%d N=%" PRIu64 ": %zu coins <= N (largest %" PRIu64 "), list has %zu (largest %" PRIu64 ")\n",
          r, N, ncoin, F[ncoin - 1], nf, F[nf - 1]);
  unsigned char *m = malloc(N + 1);
  if (!m) { fprintf(stderr, "out of memory\n"); return 1; }
  m[0] = 0; memset(m + 1, CAP, N);
  for (size_t c = 0; c < ncoin; c++) {
    uint64_t a = F[c];
    if (a == 1) { for (uint64_t n = 1; n <= N; n++) { unsigned char v = (unsigned char)(m[n - 1] + 1); if (v < m[n]) m[n] = v; } }
    else for (uint64_t s = a; s <= N; s += a) { uint64_t len = N + 1 - s < a ? N + 1 - s : a; relax(m + s, m + s - a, len); }
  }
  uint64_t hist[CAP + 1] = {0}, nex = 0, last = 0;
  for (uint64_t n = 0; n <= N; n++) hist[m[n]]++;
  for (uint64_t n = 1; n <= N; n++) if (m[n] > r + 1) { printf("%" PRIu64 "\n", n); nex++; last = n; }
  fprintf(stderr, "exceptions in [1, N]: %" PRIu64 ", largest %" PRIu64 " (min summands %d)\n", nex, last, last ? m[last] : -1);
  fprintf(stderr, "min-summand histogram over [0, N]:");
  for (int k = 0; k <= CAP; k++) if (hist[k]) fprintf(stderr, " %d:%" PRIu64, k, hist[k]);
  fprintf(stderr, "\n");
  { // last n needing exactly k summands, k = r+1 .. r (for context)
    for (int k = r - 1; k <= r + 2; k++) { uint64_t lk = 0; for (uint64_t n = 1; n <= N; n++) if (m[n] == k) lk = n; fprintf(stderr, "largest n <= N with min summands %d: %" PRIu64 "\n", k, lk); }
  }
  fflush(stdout);
  if (N2 <= N) return 0;
  // ---- tail (N, N2] ----
  uint64_t WS = N / 64 + 2;
  uint64_t *Sr = calloc(WS, 8), *Sr1 = calloc(WS, 8);
  for (uint64_t x = 0; x <= N; x++) {
    if (m[x] <= r) Sr[x >> 6] |= 1ull << (x & 63);
    if (m[x] <= r - 1) Sr1[x >> 6] |= 1ull << (x & 63);
  }
  free(m);
  if (F[nf - 1] < N2) fprintf(stderr, "WARNING: list ends at %" PRIu64 " < N2\n", F[nf - 1]);
  const uint64_t SEGW = 1u << 18, SEGB = SEGW * 64;  // 16.7M numbers per segment
  uint64_t *cov = malloc(SEGW * 8);
  uint64_t holes1 = 0, unresolved = 0, segs = 0;
  uint64_t *hole = malloc(sizeof *hole * 1024); size_t hcap = 1024;
  for (uint64_t L = N + 1; L <= N2; L += SEGB) {
    uint64_t len = N2 - L + 1 < SEGB ? N2 - L + 1 : SEGB;
    memset(cov, 0, SEGW * 8);
    for (size_t c = 0; c < nf && F[c] <= L + len - 1; c++) {
      uint64_t a = F[c];
      if (a + N < L) continue;  // n - a > N for every n in the segment
      for (uint64_t w = 0; w * 64 < len; w++) {
        uint64_t n0 = L + w * 64;      // bits n0 .. n0+63
        if (n0 + 63 < a) continue;     // all n - a < 0
        if (n0 > a + N) break;         // all n - a > N
        uint64_t v;
        if (n0 >= a) v = get64(Sr, n0 - a);
        else v = get64(Sr, 0) << (a - n0);  // bit i of v is n0 + i - a >= 0
        cov[w] |= v;
      }
    }
    // collect holes in this segment
    size_t nh = 0;
    for (uint64_t i = 0; i < len; i++) if (!((cov[i >> 6] >> (i & 63)) & 1)) {
      if (nh == hcap) hole = realloc(hole, (hcap *= 2) * sizeof *hole);
      hole[nh++] = L + i;
    }
    holes1 += nh;
    for (size_t h = 0; h < nh; h++) {
      uint64_t n = hole[h]; int ok = 0;
      for (size_t i = 0; i < nf && F[i] <= n && !ok; i++)
        for (size_t j = 0; j <= i && F[i] + F[j] <= n && !ok; j++) {
          uint64_t x = n - F[i] - F[j];
          if (x <= N && ((Sr1[x >> 6] >> (x & 63)) & 1)) ok = 1;
        }
      if (!ok) { printf("UNRESOLVED %" PRIu64 "\n", n); unresolved++; }
    }
    segs++;
  }
  fprintf(stderr, "tail (%" PRIu64 ", %" PRIu64 "]: %" PRIu64 " segments, %" PRIu64 " not covered by F + S_r, %" PRIu64 " unresolved after F + F + S_{r-1}\n",
          N, N2, segs, holes1, unresolved);
  return 0;
}
