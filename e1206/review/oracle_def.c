// oracle_def.c: reviewer's exact computation of a(n) = largest A in {1..n} whose cubes form a
// Sidon set (all sums x^3 + y^3 with x <= y in A distinct), straight from the definition.
//
// Mode "brute" (n <= 60): for t = 0, 1, 2, ... try EVERY t-subset R of {1..n} and test whether
//   {1..n} \ R is Sidon by computing all its pair sums x^3 + y^3 with x <= y (x = y included).
//   The first t that works is n - a(n). No reduction of any kind is used.
// Mode "bb" (n <= 1000): a collision is two different representations (x,y) != (u,v), x <= y,
//   u <= v, of one sum value, both inside the current set; it is found by sorting ALL pair sums
//   of {1..n} with x <= y (x = y included), so nothing about x = y or about four distinct
//   elements is assumed. A Sidon subset must omit at least one element of every collision, so
//   we branch on which element of a collision is removed (elements tried earlier in the same
//   collision are marked "kept" in later branches, an exact partition), with a lower bound from
//   a greedy family of collisions whose removable elements are pairwise disjoint. Iterative
//   deepening on the number of removals gives the minimum. Every solution found is re-checked
//   by recomputing all pair sums of the final set.
// Each n is computed from scratch (no use of tau(n-1)).
// usage: oracle_def brute NMAX | oracle_def bb NLO NHI   (prints "n a(n) removed=[...] nodes=..")
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXN 1001
static uint64_t cube[MAXN];

// ---------- direct Sidon test of a set given as a 0/1 array over 1..n ----------
#define HBITS 20
#define HSIZE (1u << HBITS)
static uint64_t hkey[HSIZE];
static uint32_t hstamp[HSIZE];
static uint32_t curstamp = 0;
static int hinsert(uint64_t s) {  // returns 1 if s was already present
  uint32_t h = (uint32_t)((s * 0x9E3779B97F4A7C15ULL) >> (64 - HBITS));
  for (;;) {
    if (hstamp[h] != curstamp) { hstamp[h] = curstamp; hkey[h] = s; return 0; }
    if (hkey[h] == s) return 1;
    h = (h + 1) & (HSIZE - 1);
  }
}
static int is_sidon(const char *in, int n) {
  curstamp++;
  if (curstamp == 0) { memset(hstamp, 0, sizeof hstamp); curstamp = 1; }
  for (int x = 1; x <= n; x++) if (in[x])
    for (int y = x; y <= n; y++) if (in[y])
      if (hinsert(cube[x] + cube[y])) return 0;
  return 1;
}

// ---------- mode brute ----------
static char inb[MAXN];
static int brute_n, brute_found, rem[64];
static long long brute_checks;
static void brute_rec(int start, int left, int depth) {
  if (brute_found) return;
  if (left == 0) {
    brute_checks++;
    if (is_sidon(inb, brute_n)) {
      brute_found = 1;
      printf("%d %d removed=[", brute_n, brute_n - depth);
      for (int i = 0; i < depth; i++) printf(i ? ",%d" : "%d", rem[i]);
      printf("] checks=%lld\n", brute_checks);
    }
    return;
  }
  for (int v = start; v <= brute_n - left + 1 && !brute_found; v++) {
    inb[v] = 0; rem[depth] = v;
    brute_rec(v + 1, left - 1, depth + 1);
    inb[v] = 1;
  }
}

// ---------- mode bb ----------
typedef struct { uint64_t s; int a, b; } pr_t;
static int cmp_pr(const void *p, const void *q) {
  const pr_t *x = p, *y = q;
  if (x->s != y->s) return x->s < y->s ? -1 : 1;
  return x->a - y->a;
}
typedef struct { int m; int e[4]; } col_t;
static col_t *col; static int ncol;
static char removed[MAXN], kept[MAXN];
static int remlist[MAXN];
static long long nodes;
static uint32_t *mark; static uint32_t markstamp;
static int *order_buf;  // per-depth scratch for alive collisions, allocated as ncol * (depth+1)

static int dfs(int depth, int limit, int n) {
  nodes++;
  // collect alive collisions and their removable sizes
  int *alive = order_buf + (size_t)depth * (ncol + 1);
  int na = 0, best = -1, bestm = 5;
  for (int k = 0; k < ncol; k++) {
    int dead = 0, r = 0;
    for (int i = 0; i < col[k].m; i++) {
      int v = col[k].e[i];
      if (removed[v]) { dead = 1; break; }
      if (!kept[v]) r++;
    }
    if (dead) continue;
    if (r == 0) return 0;               // a collision that can no longer be broken
    alive[na++] = k * 8 + r;            // pack index and removable count
    if (r < bestm) { bestm = r; best = k; }
  }
  if (na == 0) {                        // Sidon: verify directly
    char in[MAXN];
    for (int v = 1; v <= n; v++) in[v] = !removed[v];
    if (!is_sidon(in, n)) { fprintf(stderr, "BUG: claimed solution at n=%d is not Sidon\n", n); exit(5); }
    return 1;
  }
  if (depth == limit) return 0;
  // lower bound: greedy disjoint packing, smallest removable sets first
  int lb = 0;
  markstamp++;
  for (int pass = 1; pass <= 4 && depth + lb <= limit; pass++)
    for (int t = 0; t < na; t++) {
      if ((alive[t] & 7) != pass) continue;
      int k = alive[t] >> 3, ok = 1;
      for (int i = 0; i < col[k].m; i++) { int v = col[k].e[i]; if (!kept[v] && mark[v] == markstamp) { ok = 0; break; } }
      if (!ok) continue;
      for (int i = 0; i < col[k].m; i++) { int v = col[k].e[i]; if (!kept[v]) mark[v] = markstamp; }
      lb++;
    }
  if (depth + lb > limit) return 0;
  // branch on the collision with the fewest removable elements
  int cand[4], nc = 0;
  for (int i = 0; i < col[best].m; i++) { int v = col[best].e[i]; if (!kept[v]) cand[nc++] = v; }
  int setkept[4], nk = 0, ok = 0;
  for (int i = 0; i < nc && !ok; i++) {
    int v = cand[i];
    removed[v] = 1; remlist[depth] = v;
    if (dfs(depth + 1, limit, n)) ok = 1;
    removed[v] = 0;
    if (!ok) { kept[v] = 1; setkept[nk++] = v; }
  }
  for (int i = 0; i < nk; i++) kept[setkept[i]] = 0;
  return ok;
}

static void bb(int n) {
  // all pair sums x <= y (x = y included), sorted; every two representations of a value collide
  size_t np = (size_t)n * (n + 1) / 2, k = 0;
  pr_t *P = malloc(np * sizeof(pr_t));
  for (int x = 1; x <= n; x++) for (int y = x; y <= n; y++) P[k++] = (pr_t){cube[x] + cube[y], x, y};
  qsort(P, np, sizeof(pr_t), cmp_pr);
  ncol = 0; size_t cap = 64; col = malloc(cap * sizeof(col_t));
  for (size_t i = 0; i < np;) {
    size_t j = i; while (j < np && P[j].s == P[i].s) j++;
    for (size_t p = i; p < j; p++) for (size_t q = p + 1; q < j; q++) {
      int el[4] = {P[p].a, P[p].b, P[q].a, P[q].b}, m = 0, e[4];
      for (int t = 0; t < 4; t++) { int dup = 0; for (int u = 0; u < m; u++) if (e[u] == el[t]) dup = 1; if (!dup) e[m++] = el[t]; }
      if ((size_t)ncol == cap) col = realloc(col, (cap *= 2) * sizeof(col_t));
      col[ncol].m = m; for (int t = 0; t < 4; t++) col[ncol].e[t] = t < m ? e[t] : 0;
      ncol++;
    }
    i = j;
  }
  free(P);
  memset(removed, 0, sizeof removed); memset(kept, 0, sizeof kept);
  order_buf = malloc((size_t)(ncol + 1) * (n + 2) * sizeof(int));
  nodes = 0;
  int tau = -1;
  for (int limit = 0; limit <= n; limit++) if (dfs(0, limit, n)) { tau = limit; break; }
  // removed[] is cleared on the way back; remlist[0..tau-1] is the successful branch path
  printf("%d %d removed=[", n, n - tau);
  for (int i = 0; i < tau; i++) printf(i ? ",%d" : "%d", remlist[i]);
  printf("] collisions=%d nodes=%lld\n", ncol, nodes);
  fflush(stdout);
  free(order_buf); free(col);
}

int main(int argc, char **argv) {
  if (argc < 3) { fprintf(stderr, "usage: oracle_def brute NMAX | oracle_def bb NLO NHI\n"); return 1; }
  for (int i = 0; i < MAXN; i++) cube[i] = (uint64_t)i * i * i;
  mark = calloc(MAXN, sizeof(uint32_t));
  if (!strcmp(argv[1], "brute")) {
    int nmax = atoi(argv[2]);
    if (nmax > 60) { fprintf(stderr, "brute: n <= 60\n"); return 1; }
    for (int n = 1; n <= nmax; n++) {
      brute_n = n; brute_found = 0; brute_checks = 0;
      for (int v = 1; v <= n; v++) inb[v] = 1;
      for (int t = 0; t <= n && !brute_found; t++) brute_rec(1, t, 0);
      fflush(stdout);
    }
  } else {
    int lo = atoi(argv[2]), hi = argc > 3 ? atoi(argv[3]) : lo;
    if (hi >= MAXN) { fprintf(stderr, "bb: n <= %d\n", MAXN - 1); return 1; }
    for (int n = lo; n <= hi; n++) bb(n);
  }
  return 0;
}
