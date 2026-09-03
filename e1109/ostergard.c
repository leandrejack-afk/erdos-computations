/*
 * ostergard.c : second, independent exact solver for Erdős problem 1109.
 *
 * Same graph as bbmc.c (odd squarefree a <= N in one class mod 4, edge iff
 * a+b squarefree) but a different algorithm and independently written code:
 *   - squarefreeness by trial division, not a sieve;
 *   - Östergård's algorithm (Cliquer): vertices in a fixed order, c[i] = size
 *     of the largest clique in the suffix {v_i..v_n}, computed for i = n down
 *     to 1, with the pruning size + c[i] <= best;
 *   - vertex order: by residue mod 36 then by value (not degeneracy);
 *   - a static greedy colouring (computed once, in residue order) gives the
 *     vertex order class by class and a second static bound: a clique meets
 *     each colour class at most once;
 *   - residue-pair pre-branching on the primes given (default 3 then 5, pairs
 *     taken from the largest residue downwards), pruned by a greedy matching
 *     bound on the conflict graph and by a greedy colouring count.
 *
 * Modes match bbmc.c:
 *   ostergard inc r N_lo N_hi L0 [splits]
 *   ostergard full r N [L0] [splits]
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

typedef uint64_t u64;

static int nw;                 /* words per bitset */
static int nv;                 /* vertices */
static u64 *nbr;               /* adjacency bitsets */
static int *vv;                /* index -> value */
static int *cval;              /* c[i] */
static int bestsz, tgt, stopnow;
static int *stack, sp;
static int *bestclq, bestn;
static long long visits, splits;

static int nsp = 0, spp[8];
static u64 *rescls[8];         /* rescls[i] + s*nw : vertices == s mod spp[i]^2 */
static int npairs, pr_i[4096], pr_s[4096];
static int ncls;               /* colour classes of the static greedy colouring */
static u64 *clsbits;           /* clsbits + c*nw : vertices of class c (1-based) */
static int *vcol;              /* class of each vertex */

static int is_squarefree(long m) {
    if (m <= 0) return 0;
    for (long d = 2; d * d <= m; d++) {
        if (m % (d * d) == 0) return 0;
        while (m % d == 0) m /= d;
    }
    return 1;
}

static inline void setb(u64 *b, int i) { b[i >> 6] |= (u64)1 << (i & 63); }
static inline void clrb(u64 *b, int i) { b[i >> 6] &= ~((u64)1 << (i & 63)); }
static inline int getb(const u64 *b, int i) { return (b[i >> 6] >> (i & 63)) & 1; }
static int popc(const u64 *b) { int c = 0; for (int i = 0; i < nw; i++) c += __builtin_popcountll(b[i]); return c; }
static int lowest(const u64 *b) { for (int i = 0; i < nw; i++) if (b[i]) return i * 64 + __builtin_ctzll(b[i]); return -1; }
static int disjoint(const u64 *a, const u64 *b) { for (int i = 0; i < nw; i++) if (a[i] & b[i]) return 0; return 1; }

static void record(void) {
    bestsz = sp;
    bestn = sp;
    memcpy(bestclq, stack, sizeof(int) * sp);
    if (tgt && bestsz >= tgt) stopnow = 1;
}

/* Östergård recursion on candidate set U (bitset, owned by caller) */
static int found;
static void clq(u64 *U, int depth) {
    visits++;
    if (popc(U) == 0) {
        if (sp > bestsz) { record(); found = 1; }
        return;
    }
    u64 *U2 = malloc(sizeof(u64) * nw);
    while (1) {
        int cnt = popc(U);
        if (cnt == 0) break;
        if (sp + cnt <= bestsz) break;
        int i = lowest(U);
        if (sp + cval[i] <= bestsz) break;
        /* static colouring bound: a clique meets each colour class at most
           once, and classes are contiguous in index order, so only classes
           from vcol[i] upwards can meet U */
        int hit = 0;
        for (int c = vcol[i]; c <= ncls; c++)
            if (!disjoint(U, clsbits + (size_t)c * nw)) hit++;
        if (sp + hit <= bestsz) break;
        clrb(U, i);
        stack[sp++] = i;
        for (int w = 0; w < nw; w++) U2[w] = U[w] & nbr[(size_t)i * nw + w];
        clq(U2, depth + 1);
        sp--;
        if (found || stopnow) break;
    }
    free(U2);
}

/* Östergård's outer loop restricted to the vertex subset S (bitset).  The
 * suffix structure uses index order; vertices outside S are skipped. */
static void ostergard(const u64 *S) {
    for (int i = nv - 1; i >= 0; i--) {
        if (!getb(S, i)) { cval[i] = (i + 1 < nv) ? cval[i + 1] : 0; continue; }
        found = 0;
        u64 *U = malloc(sizeof(u64) * nw);
        /* U = S ∩ N(v_i) ∩ {v_j : j > i} */
        for (int w = 0; w < nw; w++) U[w] = S[w] & nbr[(size_t)i * nw + w];
        for (int j = 0; j <= i; j++) clrb(U, j);
        sp = 0;
        stack[sp++] = i;
        clq(U, 1);
        sp = 0;
        free(U);
        cval[i] = bestsz;
        if (stopnow) return;
    }
}

/* greedy matching in the conflict graph restricted to S: every matched edge
 * removes at least one vertex from any clique, so |S| - matching bounds the
 * clique number */
static int match_bound(const u64 *S) {
    u64 *T = malloc(sizeof(u64) * nw);
    memcpy(T, S, sizeof(u64) * nw);
    int m = 0, total = popc(S);
    while (1) {
        int i = lowest(T);
        if (i < 0) break;
        clrb(T, i);
        /* find a vertex in T not adjacent to i (a conflict) */
        int j = -1;
        for (int w = 0; w < nw && j < 0; w++) {
            u64 x = T[w] & ~nbr[(size_t)i * nw + w];
            if (x) j = w * 64 + __builtin_ctzll(x);
        }
        if (j >= 0) { clrb(T, j); m++; }
    }
    free(T);
    return total - m;
}

/* greedy colouring count of G[S], scanning vertices in index order */
static int colour_count(const u64 *S) {
    u64 *T = malloc(sizeof(u64) * nw), *R = malloc(sizeof(u64) * nw);
    memcpy(T, S, sizeof(u64) * nw);
    int k = 0;
    while (popc(T) > 0) {
        k++;
        memcpy(R, T, sizeof(u64) * nw);
        while (1) {
            int i = lowest(R);
            if (i < 0) break;
            clrb(R, i); clrb(T, i);
            for (int w = 0; w < nw; w++) R[w] &= ~nbr[(size_t)i * nw + w];
        }
    }
    free(T); free(R);
    return k;
}

static void presplit(u64 *S, int idx) {
    if (stopnow) return;
    splits++;
    if (match_bound(S) <= bestsz) return;
    if (colour_count(S) <= bestsz) return;
    if (idx >= npairs) { ostergard(S); return; }
    int pi = pr_i[idx], s = pr_s[idx], q = spp[pi] * spp[pi];
    const u64 *A = rescls[pi] + (size_t)s * nw;
    const u64 *B = rescls[pi] + (size_t)(q - s) * nw;
    if (disjoint(S, A) || disjoint(S, B)) { presplit(S, idx + 1); return; }
    u64 *S2 = malloc(sizeof(u64) * nw);
    for (int w = 0; w < nw; w++) S2[w] = S[w] & ~A[w];
    presplit(S2, idx + 1);
    if (!stopnow) {
        for (int w = 0; w < nw; w++) S2[w] = S[w] & ~B[w];
        presplit(S2, idx + 1);
    }
    free(S2);
}

static int cmp_res(const void *a, const void *b) {
    int x = *(const int *)a, y = *(const int *)b;
    if (x % 36 != y % 36) return x % 36 - y % 36;
    return x - y;
}

/* greedy colouring of the values in their current order (residue mod 36,
 * then value); vertices are then reordered class by class, so that every
 * suffix {v_i..v_n} meets few colour classes and c[i] stays tight */
static void colour_order(int *vals, int cnt) {
    int *col = malloc(sizeof(int) * cnt);
    int ncol = 0;
    for (int i = 0; i < cnt; i++) col[i] = 0;
    for (int i = 0; i < cnt; i++) {
        int c = 1;
        for (;;) {
            int ok = 1;
            for (int j = 0; j < i; j++)
                if (col[j] == c && is_squarefree((long)vals[i] + vals[j])) { ok = 0; break; }
            if (ok) break;
            c++;
        }
        col[i] = c;
        if (c > ncol) ncol = c;
    }
    int *out = malloc(sizeof(int) * cnt), k = 0;
    for (int c = 1; c <= ncol; c++)
        for (int i = 0; i < cnt; i++) if (col[i] == c) out[k++] = vals[i];
    memcpy(vals, out, sizeof(int) * cnt);
    free(out); free(col);
}

static void make_graph(int *vals, int cnt) {
    nv = cnt;
    nw = (nv + 63) / 64; if (nw == 0) nw = 1;
    qsort(vals, cnt, sizeof(int), cmp_res);
    colour_order(vals, cnt);
    vv = malloc(sizeof(int) * nv);
    memcpy(vv, vals, sizeof(int) * nv);
    nbr = calloc((size_t)nv * nw, sizeof(u64));
    for (int i = 0; i < nv; i++)
        for (int j = i + 1; j < nv; j++)
            if (is_squarefree(vv[i] + vv[j])) { setb(nbr + (size_t)i * nw, j); setb(nbr + (size_t)j * nw, i); }
    /* recover the colour classes of the (already class-sorted) order: a
       vertex starts a new class when it is adjacent to some vertex of the
       current class */
    vcol = malloc(sizeof(int) * (nv + 1));
    ncls = 0;
    int cstart = 0;
    for (int i = 0; i < nv; i++) {
        int newc = (i == 0);
        for (int j = cstart; j < i && !newc; j++) if (getb(nbr + (size_t)i * nw, j)) newc = 1;
        if (newc) { ncls++; cstart = i; }
        vcol[i] = ncls;
    }
    clsbits = calloc((size_t)(ncls + 1) * nw, sizeof(u64));
    for (int i = 0; i < nv; i++) setb(clsbits + (size_t)vcol[i] * nw, i);
    cval = calloc(nv + 1, sizeof(int));
    stack = malloc(sizeof(int) * (nv + 1));
    bestclq = malloc(sizeof(int) * (nv + 1));
    for (int k = 0; k < nsp; k++) {
        int q = spp[k] * spp[k];
        rescls[k] = calloc((size_t)q * nw, sizeof(u64));
        for (int i = 0; i < nv; i++) setb(rescls[k] + (size_t)(vv[i] % q) * nw, i);
    }
}

static void drop_graph(void) {
    free(vv); free(nbr); free(cval); free(stack); free(bestclq); free(vcol); free(clsbits);
    for (int k = 0; k < nsp; k++) free(rescls[k]);
}

static int run(int L0, int target) {
    bestsz = L0; bestn = 0; tgt = target; stopnow = 0; visits = 0; splits = 0; sp = 0;
    u64 *S = calloc(nw, sizeof(u64));
    for (int i = 0; i < nv; i++) setb(S, i);
    if (nv > 0) presplit(S, 0);
    free(S);
    return bestsz;
}

static int cmpi(const void *a, const void *b) { return *(const int *)a - *(const int *)b; }

static void show(int N, int extra) {
    int len = bestn + (extra ? 1 : 0);
    int *w = malloc(sizeof(int) * len);
    for (int i = 0; i < bestn; i++) w[i] = vv[bestclq[i]];
    if (extra) w[bestn] = extra;
    qsort(w, len, sizeof(int), cmpi);
    printf("W %d:", N);
    for (int i = 0; i < len; i++) printf(" %d", w[i]);
    printf("\n");
    free(w);
}

static void set_splits(const char *s) {
    nsp = 0; npairs = 0;
    if (!s || !strcmp(s, "0")) return;
    char buf[128]; strncpy(buf, s, 127); buf[127] = 0;
    for (char *t = strtok(buf, ","); t; t = strtok(NULL, ",")) spp[nsp++] = atoi(t);
    for (int k = 0; k < nsp; k++) {
        int q = spp[k] * spp[k];
        for (int s2 = q / 2; s2 >= 1; s2--) { pr_i[npairs] = k; pr_s[npairs] = s2; npairs++; }
    }
}

int main(int argc, char **argv) {
    if (argc < 4) { fprintf(stderr, "usage: ostergard inc r N_lo N_hi L0 [splits] | ostergard full r N [L0] [splits]\n"); return 2; }
    int r = atoi(argv[2]);
    if (!strcmp(argv[1], "full")) {
        int N = atoi(argv[3]);
        int L0 = argc > 4 ? atoi(argv[4]) : 0;
        set_splits(argc > 5 ? argv[5] : "3,5");
        int *vals = malloc(sizeof(int) * (N + 1)), cnt = 0;
        for (int a = 1; a <= N; a++) if (a % 4 == r && is_squarefree(2L * a)) vals[cnt++] = a;
        make_graph(vals, cnt);
        clock_t t0 = clock();
        int res = run(L0, 0);
        printf("FULL N=%d r=%d n=%d best=%d visits=%lld splits=%lld secs=%.2f\n", N, r, nv, res, visits, splits,
               (double)(clock() - t0) / CLOCKS_PER_SEC);
        if (bestn > 0) show(N, 0);
        printf("DONE\n");
        return 0;
    }
    if (strcmp(argv[1], "inc") || argc < 6) { fprintf(stderr, "bad arguments\n"); return 2; }
    int Nlo = atoi(argv[3]), Nhi = atoi(argv[4]), L = atoi(argv[5]);
    set_splits(argc > 6 ? argv[6] : "3,5");
    int *vals = malloc(sizeof(int) * (Nhi + 1));
    for (int N = Nlo; N <= Nhi; N++) {
        if (N % 4 != r || !is_squarefree(2L * N)) continue;
        int cnt = 0;
        for (int a = 1; a < N; a++)
            if (a % 4 == r && is_squarefree(2L * a) && is_squarefree((long)a + N)) vals[cnt++] = a;
        clock_t t0 = clock();
        int res = -1, built = 0;
        long long vis = 0, spl = 0;
        if (L == 0) { res = 0; bestn = 0; }
        else if (cnt >= L) {
            make_graph(vals, cnt); built = 1;
            res = run(L - 1, L);
            vis = visits; spl = splits;
        }
        double secs = (double)(clock() - t0) / CLOCKS_PER_SEC;
        if (res >= L) { L++; show(N, N); }
        printf("%d %d n=%d visits=%lld splits=%lld secs=%.2f\n", N, L, cnt, vis, spl, secs);
        fflush(stdout);
        if (built) drop_graph();
    }
    printf("DONE\n");
    return 0;
}
