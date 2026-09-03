/*
 * bbmc.c : exact maximum clique for Erdős problem 1109.
 *
 * Vertices: odd squarefree a <= N with a == r (mod 4).  Edge a~b iff a+b is
 * squarefree.  f(N) is the size of the largest clique over r in {1,3} (the
 * loop condition "2a squarefree" is exactly "a odd and squarefree", and
 * a+b == 0 mod 4 forces a single class mod 4).
 *
 * Algorithm: bit-parallel branch and bound with a greedy colouring bound
 * (San Segundo's BBMC, Tomita-style), degeneracy ordering, re-colouring of
 * branching vertices (MCS Re-NUMBER), preceded by residue-pair branching:
 * for each split prime p and each pair {s, p^2-s} of residues mod p^2 the
 * clique cannot meet both classes, so branch on "drop class p^2-s" /
 * "drop class s", pruning with the colouring bound.
 *
 * Modes:
 *   bbmc inc r N_lo N_hi L0 [splits]
 *       incremental: for each odd squarefree N in [N_lo,N_hi] with N%4==r,
 *       decide whether N is in a clique of size L+1 where L is the running
 *       lower bound (starts at L0); if yes L++ and print the witness.
 *       Prints "N L ..." per processed N, "W N: a1 a2 ..." on a jump, "DONE".
 *   bbmc full r N [L0] [splits]
 *       maximum clique of the whole class-r graph at N.  With L0 > 0 the
 *       result is max(L0, omega); a witness is printed only if the search
 *       found a clique larger than L0.
 *
 * splits: comma separated odd primes to branch on before vertex branching,
 * e.g. "3,5,7"; "0" for none.  Default "3,5".
 * Set BBMC_NORENUMBER=1 to disable re-colouring.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

typedef uint64_t u64;

#define MAXDEPTH 1024
#define MAXSPLIT 8

static int W;             /* words per bitset */
static int n;             /* vertex count */
static u64 *adj;          /* n*W */
static int *val;          /* vertex index -> integer */
static int best;          /* incumbent size */
static int target;        /* stop as soon as best >= target (0: never) */
static int *bestset, bestlen;
static int *cur, curlen;
static long long nodes, splitnodes;
static int stopflag;
static int renumber = 1;

static int nsplit = 0;
static int split_p[MAXSPLIT];
static u64 *classbits[MAXSPLIT];  /* classbits[i] + s*W = bitset of vertices == s mod p_i^2 */

static unsigned char *sqf; /* squarefree table up to 2N */

static void sieve(int lim) {
    sqf = malloc(lim + 1);
    memset(sqf, 1, lim + 1);
    sqf[0] = 0;
    for (long p = 2; p * p <= lim; p++) {
        long q = p * p;
        for (long m = q; m <= lim; m += q) sqf[m] = 0;
    }
}

static inline int bs_test(const u64 *b, int i) { return (b[i >> 6] >> (i & 63)) & 1; }
static inline void bs_set(u64 *b, int i) { b[i >> 6] |= (u64)1 << (i & 63); }
static inline void bs_clr(u64 *b, int i) { b[i >> 6] &= ~((u64)1 << (i & 63)); }
static inline int bs_empty(const u64 *b) { for (int i = 0; i < W; i++) if (b[i]) return 0; return 1; }
static inline int bs_first(const u64 *b) { for (int i = 0; i < W; i++) if (b[i]) return i * 64 + __builtin_ctzll(b[i]); return -1; }
static inline int bs_inter_empty(const u64 *a, const u64 *b) { for (int i = 0; i < W; i++) if (a[i] & b[i]) return 0; return 1; }

/* Per-depth scratch: 3 bitsets (Q, R, NP) and two int arrays per depth, plus a
 * lazily allocated pool of colour-class bitsets. */
static u64 *scratch;
static int *ord_buf, *col_buf;
static u64 *clspool[MAXDEPTH];
static int *cntc;

static void save_best(void) {
    best = curlen;
    bestlen = curlen;
    memcpy(bestset, cur, sizeof(int) * curlen);
    if (target && best >= target) stopflag = 1;
}

/*
 * Colour-sort P into ord/col (colours non-decreasing).  Returns count m.
 * Colour class k is the greedy maximal independent set taken from the
 * remaining vertices in index order.  With renumber, a vertex v receiving a
 * colour k > kmin (kmin = best - curlen) is tried for re-insertion into a
 * class k1 <= kmin where it conflicts with a single vertex w, provided w fits
 * into a later class k2 with k1 < k2 <= kmin.
 */
static int colour_sort(const u64 *P, int *ord, int *col, u64 *cls, int depth) {
    u64 *Q = scratch + (size_t)depth * 3 * W;
    u64 *R = Q + W;
    memcpy(Q, P, sizeof(u64) * W);
    int k = 0, m = 0;
    int kmin = best - curlen;
    if (kmin < 0) kmin = 0;
    while (!bs_empty(Q)) {
        k++;
        memcpy(R, Q, sizeof(u64) * W);
        u64 *C = cls + (size_t)k * W;
        memset(C, 0, sizeof(u64) * W);
        while (!bs_empty(R)) {
            int v = bs_first(R);
            bs_clr(R, v);
            bs_clr(Q, v);
            const u64 *av = adj + (size_t)v * W;
            for (int i = 0; i < W; i++) R[i] &= ~av[i];
            int placed = 0;
            if (renumber && k > kmin && kmin >= 1) {
                for (int k1 = 1; k1 <= kmin && !placed; k1++) {
                    u64 *C1 = cls + (size_t)k1 * W;
                    int cnt = 0, w = -1;
                    for (int i = 0; i < W && cnt < 2; i++) {
                        u64 x = C1[i] & av[i];
                        while (x) {
                            cnt++;
                            if (cnt >= 2) break;
                            w = i * 64 + __builtin_ctzll(x);
                            x &= x - 1;
                        }
                    }
                    if (cnt == 0) {
                        bs_set(C1, v);
                        col[m] = k1; ord[m] = v; m++;
                        placed = 1;
                    } else if (cnt == 1) {
                        const u64 *aw = adj + (size_t)w * W;
                        for (int k2 = k1 + 1; k2 <= kmin; k2++) {
                            u64 *C2 = cls + (size_t)k2 * W;
                            if (bs_inter_empty(C2, aw)) {
                                bs_clr(C1, w); bs_set(C2, w); bs_set(C1, v);
                                for (int j = 0; j < m; j++) if (ord[j] == w) { col[j] = k2; break; }
                                col[m] = k1; ord[m] = v; m++;
                                placed = 1;
                                break;
                            }
                        }
                    }
                }
            }
            if (!placed) {
                bs_set(C, v);
                col[m] = k; ord[m] = v; m++;
            }
        }
    }
    if (renumber) {
        /* restore non-decreasing colours along ord (stable counting sort),
           borrowing the next depth's buffers as temporaries */
        int *o2 = ord_buf + (size_t)(depth + 1) * n;
        int *c2 = col_buf + (size_t)(depth + 1) * n;
        for (int i = 0; i <= k + 1; i++) cntc[i] = 0;
        for (int j = 0; j < m; j++) cntc[col[j]]++;
        int acc = 0;
        for (int i = 0; i <= k; i++) { int t = cntc[i]; cntc[i] = acc; acc += t; }
        for (int j = 0; j < m; j++) { int p = cntc[col[j]]++; o2[p] = ord[j]; c2[p] = col[j]; }
        memcpy(ord, o2, sizeof(int) * m);
        memcpy(col, c2, sizeof(int) * m);
    }
    return m;
}

/* number of greedy colour classes of P (plain, no renumber): an upper bound
 * on the clique number of G[P] */
static int colour_bound(const u64 *P, int depth) {
    u64 *Q = scratch + (size_t)depth * 3 * W;
    u64 *R = Q + W;
    memcpy(Q, P, sizeof(u64) * W);
    int k = 0;
    while (!bs_empty(Q)) {
        k++;
        memcpy(R, Q, sizeof(u64) * W);
        while (!bs_empty(R)) {
            int v = bs_first(R);
            bs_clr(R, v);
            bs_clr(Q, v);
            const u64 *av = adj + (size_t)v * W;
            for (int i = 0; i < W; i++) R[i] &= ~av[i];
        }
    }
    return k;
}

static void expand(u64 *P, int depth) {
    nodes++;
    if (depth + 1 >= MAXDEPTH) { fprintf(stderr, "depth overflow\n"); exit(3); }
    int *ord = ord_buf + (size_t)depth * n;
    int *col = col_buf + (size_t)depth * n;
    if (!clspool[depth]) clspool[depth] = malloc(sizeof(u64) * (size_t)(n + 2) * W);
    u64 *classes = clspool[depth];
    u64 *NP = scratch + (size_t)depth * 3 * W + 2 * W;
    int m = colour_sort(P, ord, col, classes, depth);
    for (int i = m - 1; i >= 0; i--) {
        if (curlen + col[i] <= best) break;
        int v = ord[i];
        cur[curlen++] = v;
        const u64 *av = adj + (size_t)v * W;
        int empty = 1;
        for (int j = 0; j < W; j++) { NP[j] = P[j] & av[j]; if (NP[j]) empty = 0; }
        if (empty) {
            if (curlen > best) save_best();
        } else {
            expand(NP, depth + 1);
        }
        curlen--;
        bs_clr(P, v);
        if (stopflag) break;
    }
}

/*
 * Residue-pair branching.  pair index runs over (prime i, residue s) with
 * 1 <= s < p_i^2/2; the two classes are s and p_i^2 - s.  Depth counter for
 * scratch use runs from the top of the scratch pool downwards.
 */
static int total_pairs;
static int pair_prime[4096], pair_s[4096];

static void split(u64 *P, int idx, int sdepth) {
    if (stopflag) return;
    splitnodes++;
    /* prune with the colouring bound (curlen == 0 here) */
    int ub = colour_bound(P, MAXDEPTH - 1 - sdepth);
    if (ub <= best) return;
    if (idx >= total_pairs) { expand(P, 0); return; }
    int pi = pair_prime[idx], s = pair_s[idx];
    int q = split_p[pi] * split_p[pi];
    const u64 *Cs = classbits[pi] + (size_t)s * W;
    const u64 *Ct = classbits[pi] + (size_t)(q - s) * W;
    int has_s = !bs_inter_empty(P, Cs), has_t = !bs_inter_empty(P, Ct);
    if (!has_s || !has_t) { split(P, idx + 1, sdepth); return; }
    u64 *P1 = scratch + (size_t)(MAXDEPTH - 1 - sdepth) * 3 * W + 2 * W;
    for (int i = 0; i < W; i++) P1[i] = P[i] & ~Ct[i];
    split(P1, idx + 1, sdepth + 1);
    if (stopflag) return;
    for (int i = 0; i < W; i++) P1[i] = P[i] & ~Cs[i];
    split(P1, idx + 1, sdepth + 1);
}

/* Build the graph on the given integer values with degeneracy ordering:
 * position n-1 gets the global min-degree vertex, then the min-degree vertex
 * of the remainder, and so on (MCS "minimum width" order). */
static void build_graph(const int *vals, int cnt) {
    n = cnt;
    W = (n + 63) / 64;
    if (W == 0) W = 1;
    u64 *tadj = calloc((size_t)n * W, sizeof(u64));
    int *deg = calloc(n, sizeof(int));
    for (int i = 0; i < n; i++)
        for (int j = i + 1; j < n; j++)
            if (sqf[vals[i] + vals[j]]) {
                bs_set(tadj + (size_t)i * W, j);
                bs_set(tadj + (size_t)j * W, i);
                deg[i]++; deg[j]++;
            }
    int *pos = malloc(sizeof(int) * n);
    int *removed = calloc(n, sizeof(int));
    int *d = malloc(sizeof(int) * n);
    memcpy(d, deg, sizeof(int) * n);
    for (int p = n - 1; p >= 0; p--) {
        int bi = -1, bd = 1 << 30;
        for (int i = 0; i < n; i++) if (!removed[i] && d[i] < bd) { bd = d[i]; bi = i; }
        removed[bi] = 1;
        pos[bi] = p;
        const u64 *ai = tadj + (size_t)bi * W;
        for (int j = 0; j < n; j++) if (!removed[j] && bs_test(ai, j)) d[j]--;
    }
    adj = calloc((size_t)n * W, sizeof(u64));
    val = malloc(sizeof(int) * n);
    for (int i = 0; i < n; i++) val[pos[i]] = vals[i];
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            if (bs_test(tadj + (size_t)i * W, j)) bs_set(adj + (size_t)pos[i] * W, pos[j]);
    free(tadj); free(deg); free(pos); free(removed); free(d);
    scratch = malloc(sizeof(u64) * (size_t)MAXDEPTH * 3 * W);
    ord_buf = malloc(sizeof(int) * (size_t)(n + 2) * n);
    col_buf = malloc(sizeof(int) * (size_t)(n + 2) * n);
    cntc = malloc(sizeof(int) * (n + 3));
    bestset = malloc(sizeof(int) * (n + 1));
    cur = malloc(sizeof(int) * (n + 1));
    for (int i = 0; i < MAXDEPTH; i++) clspool[i] = NULL;
    for (int i = 0; i < nsplit; i++) {
        int q = split_p[i] * split_p[i];
        classbits[i] = calloc((size_t)q * W, sizeof(u64));
        for (int v = 0; v < n; v++) bs_set(classbits[i] + (size_t)(val[v] % q) * W, v);
    }
}

static void free_graph(void) {
    free(adj); free(val); free(scratch); free(ord_buf); free(col_buf); free(cntc);
    free(bestset); free(cur);
    for (int i = 0; i < MAXDEPTH; i++) if (clspool[i]) { free(clspool[i]); clspool[i] = NULL; }
    for (int i = 0; i < nsplit; i++) { free(classbits[i]); classbits[i] = NULL; }
    adj = NULL;
}

/* Search the whole vertex set with incumbent L0; stop early once a clique of
 * size >= tgt is found (tgt = 0: exhaustive).  Returns max(L0, size found). */
static int solve(int L0, int tgt) {
    best = L0; bestlen = 0; curlen = 0; nodes = 0; splitnodes = 0; stopflag = 0; target = tgt;
    u64 *P = calloc(W, sizeof(u64));
    for (int i = 0; i < n; i++) bs_set(P, i);
    if (n > 0) split(P, 0, 0);
    free(P);
    return best;
}

static int cmp_int(const void *a, const void *b) { return *(const int *)a - *(const int *)b; }

static void print_witness(int N, int extra) {
    int len = bestlen + (extra > 0 ? 1 : 0);
    int *w = malloc(sizeof(int) * len);
    for (int i = 0; i < bestlen; i++) w[i] = val[bestset[i]];
    if (extra > 0) w[bestlen] = extra;
    qsort(w, len, sizeof(int), cmp_int);
    printf("W %d:", N);
    for (int i = 0; i < len; i++) printf(" %d", w[i]);
    printf("\n");
    free(w);
}

static void parse_splits(const char *s) {
    nsplit = 0;
    total_pairs = 0;
    if (!s || strcmp(s, "0") == 0) return;
    char buf[256];
    strncpy(buf, s, 255); buf[255] = 0;
    for (char *tok = strtok(buf, ","); tok; tok = strtok(NULL, ",")) {
        int p = atoi(tok);
        if (p < 3 || p % 2 == 0 || nsplit >= MAXSPLIT) { fprintf(stderr, "bad split prime %s\n", tok); exit(2); }
        split_p[nsplit++] = p;
    }
    for (int i = 0; i < nsplit; i++) {
        int q = split_p[i] * split_p[i];
        for (int s2 = 1; s2 < q - s2; s2++) {
            if (total_pairs >= 4096) { fprintf(stderr, "too many pairs\n"); exit(2); }
            pair_prime[total_pairs] = i; pair_s[total_pairs] = s2; total_pairs++;
        }
    }
}

int main(int argc, char **argv) {
    if (argc < 4) {
        fprintf(stderr, "usage: bbmc inc r N_lo N_hi L0 [splits] | bbmc full r N [L0] [splits]\n");
        return 2;
    }
    const char *mode = argv[1];
    int r = atoi(argv[2]);
    if (getenv("BBMC_NORENUMBER")) renumber = 0;
    if (strcmp(mode, "full") == 0) {
        int N = atoi(argv[3]);
        int L0 = argc > 4 ? atoi(argv[4]) : 0;
        parse_splits(argc > 5 ? argv[5] : "3,5");
        sieve(2 * N + 2);
        int *vals = malloc(sizeof(int) * (N + 1));
        int cnt = 0;
        for (int a = 1; a <= N; a++)
            if (a % 4 == r && sqf[a]) vals[cnt++] = a;
        build_graph(vals, cnt);
        clock_t t0 = clock();
        int res = solve(L0, 0);
        double secs = (double)(clock() - t0) / CLOCKS_PER_SEC;
        printf("FULL N=%d r=%d n=%d best=%d nodes=%lld splitnodes=%lld secs=%.2f\n", N, r, n, res, nodes, splitnodes, secs);
        if (bestlen > 0) print_witness(N, 0);
        printf("DONE\n");
        return 0;
    }
    if (strcmp(mode, "inc") != 0 || argc < 6) { fprintf(stderr, "bad arguments\n"); return 2; }
    int Nlo = atoi(argv[3]), Nhi = atoi(argv[4]);
    int L = atoi(argv[5]);
    parse_splits(argc > 6 ? argv[6] : "3,5");
    sieve(2 * Nhi + 2);
    int *vals = malloc(sizeof(int) * (Nhi + 1));
    for (int N = Nlo; N <= Nhi; N++) {
        if (N % 4 != r || !sqf[N]) continue;
        int cnt = 0;
        for (int a = 1; a < N; a++)
            if (a % 4 == r && sqf[a] && sqf[a + N]) vals[cnt++] = a;
        clock_t t0 = clock();
        int res = -1, built = 0;
        long long nd = 0, sn = 0;
        if (L == 0) {
            res = 0;           /* {N} itself is a clique of size 1 */
            bestlen = 0;
        } else if (cnt >= L) {
            build_graph(vals, cnt);
            built = 1;
            res = solve(L - 1, L);   /* clique of size >= L inside the neighbourhood? */
            nd = nodes; sn = splitnodes;
        }
        double secs = (double)(clock() - t0) / CLOCKS_PER_SEC;
        if (res >= L) {
            L = L + 1;
            print_witness(N, N);
        }
        printf("%d %d n=%d nodes=%lld splitnodes=%lld secs=%.2f\n", N, L, cnt, nd, sn, secs);
        fflush(stdout);
        if (built) free_graph();
    }
    printf("DONE\n");
    return 0;
}
