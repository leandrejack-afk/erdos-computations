/*
 * solver2.c : second, independently written exact solver for Erdős 1109.
 *
 * Written from the problem statement without reference to bbmc.c: plain
 * arrays instead of bitsets, trial-division squarefree test instead of a
 * sieve, vertices kept in increasing numerical order (no degeneracy
 * ordering), sequential greedy colouring (Tomita and Seki's MCQ NUMBER-SORT,
 * no re-colouring), and residue-pair splitting on 3 then 5 with the pairs
 * taken from the largest residue downwards.
 *
 * Graph: vertices are the a <= N with a == r (mod 4) and 2a squarefree;
 * a ~ b iff a+b is squarefree.  A subset with all pairwise sums (including
 * a+a) squarefree is exactly a clique, so f(N) = max over r of the clique
 * number.  The residue split uses: for an odd prime p and 1 <= s < p^2/2 no
 * clique meets both {a == s mod p^2} and {a == p^2-s mod p^2}, because such
 * a pair sums to 0 mod p^2.
 *
 * Usage (same as bbmc):
 *   solver2 inc r N_lo N_hi L0 [splits]
 *   solver2 full r N [L0] [splits]
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static int n;                /* number of vertices */
static int *value;           /* vertex index -> integer */
static unsigned char *adjm;  /* n*n adjacency matrix */
static int best, target, halt;
static int *clique, csize;
static int *bestc, bestsize;
static long long nodes, splitnodes;

static int nsplit, splitp[8];
static int npairs, pair_p[2048], pair_s[2048];

static int squarefree(long m) {
    if (m < 1) return 0;
    for (long d = 2; d * d <= m; d++)
        if (m % (d * d) == 0) return 0;
    return 1;
}

/*
 * Greedy sequential colouring of the candidate array cand[0..m-1] (kept in
 * increasing index order).  On return cand is reordered by non-decreasing
 * colour and col[i] is the colour of cand[i].  Returns the number of colours.
 */
static int number_sort(int *cand, int *col, int m, int *work) {
    /* work: class lists, m*m ints at most; we store class members flat */
    int ncol = 0;
    int *csz = work;                  /* csz[c] = size of class c */
    int *cmem = work + m + 1;         /* cmem[c*m + j] = members of class c */
    for (int c = 0; c <= m; c++) csz[c] = 0;
    for (int i = 0; i < m; i++) {
        int v = cand[i];
        int c = 0;
        for (;;) {
            int ok = 1;
            for (int j = 0; j < csz[c]; j++)
                if (adjm[(size_t)v * n + cmem[c * m + j]]) { ok = 0; break; }
            if (ok) break;
            c++;
        }
        if (c == ncol) ncol++;
        cmem[c * m + csz[c]] = v;
        csz[c]++;
    }
    int k = 0;
    for (int c = 0; c < ncol; c++)
        for (int j = 0; j < csz[c]; j++) { cand[k] = cmem[c * m + j]; col[k] = c + 1; k++; }
    return ncol;
}

static void found_clique(void) {
    best = csize;
    bestsize = csize;
    memcpy(bestc, clique, sizeof(int) * csize);
    if (target && best >= target) halt = 1;
}

/* MCQ expansion on candidate array cand[0..m-1] (increasing order) */
static void expand(int *cand, int m) {
    nodes++;
    int *col = malloc(sizeof(int) * m);
    int *work = malloc(sizeof(int) * (m + 1 + (size_t)m * m));
    number_sort(cand, col, m, work);
    free(work);
    int *next = malloc(sizeof(int) * m);
    for (int i = m - 1; i >= 0; i--) {
        if (csize + col[i] <= best) break;
        int v = cand[i];
        clique[csize++] = v;
        int k = 0;
        for (int j = 0; j < i; j++)
            if (adjm[(size_t)v * n + cand[j]]) next[k++] = cand[j];
        /* next is in the colour-sorted order; restore increasing index order
           so the next colouring is deterministic in vertex order */
        if (k == 0) {
            if (csize > best) found_clique();
        } else {
            /* insertion sort is fine: k is small and nearly sorted */
            for (int a = 1; a < k; a++) {
                int x = next[a], b = a - 1;
                while (b >= 0 && next[b] > x) { next[b + 1] = next[b]; b--; }
                next[b + 1] = x;
            }
            expand(next, k);
        }
        csize--;
        if (halt) break;
    }
    free(next);
    free(col);
}

/* greedy colouring count of a candidate array, used as the split bound */
static int colour_count(const int *cand, int m) {
    int *tmp = malloc(sizeof(int) * m), *col = malloc(sizeof(int) * m);
    int *work = malloc(sizeof(int) * (m + 1 + (size_t)m * m));
    memcpy(tmp, cand, sizeof(int) * m);
    int k = number_sort(tmp, col, m, work);
    free(tmp); free(col); free(work);
    return k;
}

static void split(const int *cand, int m, int idx) {
    if (halt) return;
    splitnodes++;
    if (m <= best) return;
    if (colour_count(cand, m) <= best) return;
    if (idx >= npairs) {
        int *c2 = malloc(sizeof(int) * m);
        memcpy(c2, cand, sizeof(int) * m);
        expand(c2, m);
        free(c2);
        return;
    }
    int q = pair_p[idx] * pair_p[idx], s = pair_s[idx], t = q - s;
    int has_s = 0, has_t = 0;
    for (int i = 0; i < m; i++) {
        int rr = value[cand[i]] % q;
        if (rr == s) has_s = 1;
        if (rr == t) has_t = 1;
    }
    if (!has_s || !has_t) { split(cand, m, idx + 1); return; }
    int *sub = malloc(sizeof(int) * m);
    int k = 0;
    for (int i = 0; i < m; i++) if (value[cand[i]] % q != t) sub[k++] = cand[i];
    split(sub, k, idx + 1);
    if (!halt) {
        k = 0;
        for (int i = 0; i < m; i++) if (value[cand[i]] % q != s) sub[k++] = cand[i];
        split(sub, k, idx + 1);
    }
    free(sub);
}

static void build(const int *vals, int cnt) {
    n = cnt;
    value = malloc(sizeof(int) * (n + 1));
    memcpy(value, vals, sizeof(int) * n);
    adjm = calloc((size_t)n * n + 1, 1);
    for (int i = 0; i < n; i++)
        for (int j = i + 1; j < n; j++)
            if (squarefree((long)value[i] + value[j])) adjm[(size_t)i * n + j] = adjm[(size_t)j * n + i] = 1;
    clique = malloc(sizeof(int) * (n + 1));
    bestc = malloc(sizeof(int) * (n + 1));
}

static void release(void) {
    free(value); free(adjm); free(clique); free(bestc);
}

static int run(int L0, int tgt) {
    best = L0; bestsize = 0; csize = 0; target = tgt; halt = 0; nodes = 0; splitnodes = 0;
    int *cand = malloc(sizeof(int) * (n + 1));
    for (int i = 0; i < n; i++) cand[i] = i;
    if (n > 0) split(cand, n, 0);
    free(cand);
    return best;
}

static int cmpint(const void *a, const void *b) { return *(const int *)a - *(const int *)b; }

static void witness(int N, int extra) {
    int len = bestsize + (extra ? 1 : 0);
    int *w = malloc(sizeof(int) * len);
    for (int i = 0; i < bestsize; i++) w[i] = value[bestc[i]];
    if (extra) w[bestsize] = extra;
    qsort(w, len, sizeof(int), cmpint);
    printf("W %d:", N);
    for (int i = 0; i < len; i++) printf(" %d", w[i]);
    printf("\n");
    free(w);
}

static void parse_splits(const char *s) {
    nsplit = 0; npairs = 0;
    if (!s || !strcmp(s, "0")) return;
    char buf[128];
    strncpy(buf, s, 127); buf[127] = 0;
    for (char *t = strtok(buf, ","); t; t = strtok(NULL, ",")) splitp[nsplit++] = atoi(t);
    for (int i = 0; i < nsplit; i++) {
        int q = splitp[i] * splitp[i];
        for (int s2 = q / 2; s2 >= 1; s2--) { pair_p[npairs] = splitp[i]; pair_s[npairs] = s2; npairs++; }
    }
}

int main(int argc, char **argv) {
    if (argc < 4) { fprintf(stderr, "usage: solver2 inc r N_lo N_hi L0 [splits] | solver2 full r N [L0] [splits]\n"); return 2; }
    int r = atoi(argv[2]);
    if (!strcmp(argv[1], "full")) {
        int N = atoi(argv[3]);
        int L0 = argc > 4 ? atoi(argv[4]) : 0;
        parse_splits(argc > 5 ? argv[5] : "3,5");
        int *vals = malloc(sizeof(int) * (N + 1)), cnt = 0;
        for (int a = 1; a <= N; a++) if (a % 4 == r && squarefree(2L * a)) vals[cnt++] = a;
        build(vals, cnt);
        clock_t t0 = clock();
        int res = run(L0, 0);
        printf("FULL N=%d r=%d n=%d best=%d nodes=%lld splitnodes=%lld secs=%.2f\n", N, r, n, res, nodes, splitnodes,
               (double)(clock() - t0) / CLOCKS_PER_SEC);
        if (bestsize > 0) witness(N, 0);
        printf("DONE\n");
        return 0;
    }
    if (strcmp(argv[1], "inc") || argc < 6) { fprintf(stderr, "bad arguments\n"); return 2; }
    int Nlo = atoi(argv[3]), Nhi = atoi(argv[4]), L = atoi(argv[5]);
    parse_splits(argc > 6 ? argv[6] : "3,5");
    int *vals = malloc(sizeof(int) * (Nhi + 1));
    for (int N = Nlo; N <= Nhi; N++) {
        if (N % 4 != r || !squarefree(2L * N)) continue;
        int cnt = 0;
        for (int a = 1; a < N; a++)
            if (a % 4 == r && squarefree(2L * a) && squarefree((long)a + N)) vals[cnt++] = a;
        clock_t t0 = clock();
        int res = -1, built = 0;
        long long nd = 0, sn = 0;
        if (L == 0) { res = 0; bestsize = 0; }
        else if (cnt >= L) {
            build(vals, cnt); built = 1;
            res = run(L - 1, L);
            nd = nodes; sn = splitnodes;
        }
        double secs = (double)(clock() - t0) / CLOCKS_PER_SEC;
        if (res >= L) { L++; witness(N, N); }
        printf("%d %d n=%d nodes=%lld splitnodes=%lld secs=%.2f\n", N, L, cnt, nd, sn, secs);
        fflush(stdout);
        if (built) release();
    }
    printf("DONE\n");
    return 0;
}
