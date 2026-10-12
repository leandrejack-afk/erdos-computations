/*
 * cand2.c: a second enumeration of the maximal bad intervals (OEIS A388654, A388850; Erdős
 * Problem 382), written separately from cand.c as an independent check of it.
 *
 * P(k) is the largest prime factor of k (P(1) = 1). A square point is an m > 1 with P(m)^2 | m.
 * W(m) is the longest interval containing m on which every k has P(k) <= P(m). The members of
 * A388654 are exactly the integers in some W(m), and the maximal runs of members are the
 * connected components of the union of the W(m) (README.md has the proof). This program
 * collects every W(m) with m <= X and length >= MINLEN and prints the components of their union.
 *
 * Differences from cand.c, on purpose:
 *  - square points m = p^2 s with s <= X/p^2 <= 2^24 come from a table of P(s) (a sieve over
 *    s), and only the small p, where X/p^2 > 2^24, use an enumeration of p-smooth s, done with
 *    an explicit stack and primes taken largest first;
 *  - a neighbour k is tested for P(k) <= p by trial division with the % operator up to
 *    min(p, 1000), then a Baillie-PSW test (strong base 2 plus strong Lucas with Selfridge's
 *    parameters; no composite below 2^64 passes it) and Pollard's rho with Floyd's cycle test;
 *  - runs are read off as the union of the windows (cand.c drops windows nested in others);
 *    any two windows that overlap without one containing the other, or that touch end to end,
 *    are reported as VIOLATION lines, since the proof says that never happens.
 *
 * usage: cand2 X MINLEN THREADS
 *        DUMP=N cand2 X 1 THREADS    (also print every member of A388654 up to N; take X a
 *                                     little above N so that no window near N is missed)
 * output: CANDIDATES (square points m <= X), VIOLATIONS, one RUN line per component of length
 * >= 10 (start end length m p, where m is the square point with the largest p in it), FIRST
 * lines (length, first start, count) for components of length >= MINLEN.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>

typedef unsigned long long ull;
typedef unsigned __int128 uint128;

#define TABLE_BITS 24
#define TRIAL_MAX 1000

static ull X, DUMP; static int MINLEN, NTHREADS;
static uint32_t *ptab;            /* ptab[s] = P(s) for s < 2^TABLE_BITS */
static ull *plist; static long np;  /* primes up to max(sqrt(X), TRIAL_MAX) */
static long ntrial;               /* primes <= TRIAL_MAX */

/* ---------------- modular arithmetic, Montgomery form, odd n < 2^63 ---------------- */
typedef struct { ull n, ni, r1, r2; } mctx;   /* ni = -n^-1 mod 2^64, r1 = 2^64 mod n, r2 = 2^128 mod n */

static void mset(mctx *c, ull n) {
    ull x = 1;                                  /* Hensel lifting: x = n^-1 mod 2^(2^i) */
    for (int i = 0; i < 7; i++) x = x * (2 - n * x);
    c->n = n; c->ni = 0 - x;
    c->r1 = (ull)(((uint128)1 << 64) % n);
    c->r2 = (ull)(((uint128)c->r1 << 64) % n);
}
static inline ull mred(uint128 t, const mctx *c) {
    ull q = (ull)t * c->ni;
    uint128 s = t + (uint128)q * c->n;        /* < 2^128 because t < n^2 < 2^126 */
    ull r = (ull)(s >> 64);
    return r >= c->n ? r - c->n : r;
}
static inline ull mmul(ull a, ull b, const mctx *c) { return mred((uint128)a * b, c); }
static inline ull menter(ull a, const mctx *c) { return mmul(a % c->n, c->r2, c); }
static inline ull madd(ull a, ull b, const mctx *c) { ull s = a + b; return s >= c->n ? s - c->n : s; }
static inline ull msub(ull a, ull b, const mctx *c) { return a >= b ? a - b : a + c->n - b; }
static inline ull mhalf(ull a, const mctx *c) {     /* a / 2 mod n, n odd, a < n < 2^63 */
    return (a & 1) ? (a + c->n) >> 1 : a >> 1;
}

static ull isqrt_ull(ull n) {
    ull r = 0, bit = 1ULL << 62;
    while (bit > n) bit >>= 2;
    while (bit) {
        if (n >= r + bit) { n -= r + bit; r = (r >> 1) + bit; } else r >>= 1;
        bit >>= 2;
    }
    return r;
}

static int jacobi(long long a, ull n) {     /* Jacobi symbol (a/n), n odd positive */
    ull aa; int t = 1;
    if (a < 0) { aa = (ull)(-a) % n; if ((n & 3) == 3) t = -t; } else aa = (ull)a % n;
    while (aa) {
        while (!(aa & 1)) { aa >>= 1; ull r = n & 7; if (r == 3 || r == 5) t = -t; }
        ull tmp = aa; aa = n; n = tmp;
        if ((aa & 3) == 3 && (n & 3) == 3) t = -t;
        aa %= n;
    }
    return n == 1 ? t : 0;
}

/* Baillie-PSW for odd n > 1 with no prime factor below 50 */
static int bpsw(ull n) {
    if (n >= (1ULL << 63)) { fprintf(stderr, "bpsw: %llu too large\n", n); exit(2); }
    mctx c; mset(&c, n);
    ull one = c.r1, mone = n - c.r1;          /* 1 and -1 in Montgomery form */
    /* strong probable prime to base 2 */
    ull d = n - 1; int s = 0;
    while (!(d & 1)) { d >>= 1; s++; }
    ull b = menter(2, &c), x = one;
    for (ull e = d; e; e >>= 1) { if (e & 1) x = mmul(x, b, &c); b = mmul(b, b, &c); }
    if (x != one && x != mone) {
        int ok = 0;
        for (int r = 1; r < s && !ok; r++) { x = mmul(x, x, &c); if (x == mone) ok = 1; }
        if (!ok) return 0;
    }
    /* strong Lucas test with Selfridge's method A: D = 5, -7, 9, -11, ..., P = 1, Q = (1-D)/4 */
    ull rt = isqrt_ull(n);
    if (rt * rt == n) return 0;
    long long D = 5;
    for (;;) {
        int j = jacobi(D, n);
        if (j == -1) break;
        if (j == 0 && (ull)(D < 0 ? -D : D) != n) return 0;
        D = D > 0 ? -(D + 2) : -D + 2;
    }
    long long Q = (1 - D) / 4;
    ull Dm = menter(D >= 0 ? (ull)D : n - ((ull)(-D) % n), &c);
    ull Qm = menter(Q >= 0 ? (ull)Q : n - ((ull)(-Q) % n), &c);
    ull dd = n + 1; int ss = 0;
    while (!(dd & 1)) { dd >>= 1; ss++; }
    /* U_1 = 1, V_1 = P = 1, Q^1; walk the bits of dd below the top one */
    ull U = one, V = one, Qk = Qm;
    int top = 63 - __builtin_clzll(dd);
    for (int i = top - 1; i >= 0; i--) {
        U = mmul(U, V, &c);                                   /* U_2k = U_k V_k */
        V = msub(mmul(V, V, &c), madd(Qk, Qk, &c), &c);       /* V_2k = V_k^2 - 2 Q^k */
        Qk = mmul(Qk, Qk, &c);
        if ((dd >> i) & 1) {                                  /* k -> k + 1 with P = 1 */
            ull U1 = mhalf(madd(U, V, &c), &c);               /* U_{k+1} = (U_k + V_k) / 2 */
            ull V1 = mhalf(madd(mmul(Dm, U, &c), V, &c), &c); /* V_{k+1} = (D U_k + V_k) / 2 */
            U = U1; V = V1; Qk = mmul(Qk, Qm, &c);
        }
    }
    if (U == 0 || V == 0) return 1;
    for (int r = 1; r < ss; r++) {
        V = msub(mmul(V, V, &c), madd(Qk, Qk, &c), &c);
        Qk = mmul(Qk, Qk, &c);
        if (V == 0) return 1;
    }
    return 0;
}

static ull gcd_ull(ull a, ull b) { while (b) { ull t = a % b; a = b; b = t; } return a; }

/* a proper divisor of the odd composite n, which has no prime factor below 50 */
static ull rho_floyd(ull n) {
    ull r = isqrt_ull(n);
    if (r * r == n) return r;
    mctx c; mset(&c, n);
    for (ull k = 1; k < 200; k++) {
        ull cc = menter(k, &c), x = menter(3, &c), y = x, g = 1;
        while (g == 1) {
            ull prod = c.r1, xs = x, ys = y;
            for (int i = 0; i < 64; i++) {
                x = madd(mmul(x, x, &c), cc, &c);
                y = madd(mmul(y, y, &c), cc, &c);
                y = madd(mmul(y, y, &c), cc, &c);
                prod = mmul(prod, x > y ? x - y : y - x, &c);
            }
            g = gcd_ull(prod, n);
            if (g == n) {                     /* overshot: redo the batch one step at a time */
                x = xs; y = ys; g = 1;
                while (g == 1) {
                    x = madd(mmul(x, x, &c), cc, &c);
                    y = madd(mmul(y, y, &c), cc, &c);
                    y = madd(mmul(y, y, &c), cc, &c);
                    g = gcd_ull(x > y ? x - y : y - x, n);
                }
            }
        }
        if (g != n && n % g == 0) return g;
    }
    fprintf(stderr, "rho_floyd: no factor of %llu\n", n); exit(2);
}

/* every prime factor of n is > TRIAL_MAX; is every prime factor <= p? */
static int rough_part_ok(ull n, ull p) {
    if (n <= p) return 1;
    if (n < (ull)TRIAL_MAX * TRIAL_MAX) return 0;          /* then n is a prime above p */
    if (bpsw(n)) return 0;
    ull f = rho_floyd(n);
    return rough_part_ok(f, p) && rough_part_ok(n / f, p);
}

/* is every prime factor of k (k >= 1) at most p, for p prime? */
static int all_factors_le(ull k, ull p) {
    if (k <= p) return 1;
    for (long i = 0; i < ntrial; i++) {
        ull q = plist[i];
        if (q > p) return 0;                  /* k > p and no factor <= p remains */
        if (k % q == 0) {
            do k /= q; while (k % q == 0);
            if (k <= p) return 1;
        }
        if (q * q > k) return 0;              /* k > p is a prime */
    }
    return rough_part_ok(k, p);
}

/* ---------------- windows ---------------- */
typedef struct { ull lo, hi, m, p; } window;
typedef struct { ull cands; window *w; long nw, cap; } wstate;

static void window_at(ull m, ull p, wstate *S) {
    S->cands++;
    ull lo = m, hi = m;
    while (lo > 1 && all_factors_le(lo - 1, p)) lo--;
    while (all_factors_le(hi + 1, p)) hi++;
    if (hi - lo + 1 >= (ull)MINLEN) {
        if (S->nw == S->cap) { S->cap = S->cap ? S->cap * 2 : 4096; S->w = realloc(S->w, S->cap * sizeof(window)); }
        S->w[S->nw].lo = lo; S->w[S->nw].hi = hi; S->w[S->nw].m = m; S->w[S->nw].p = p; S->nw++;
    }
}

static long next_job = 0, njobs;
static pthread_mutex_t job_lock = PTHREAD_MUTEX_INITIALIZER;

typedef struct { ull v; long j; } frame;

static void do_prime(long pi, wstate *S, frame **stk, long *stkcap) {
    ull p = plist[pi], p2 = p * p;
    if (p2 > X) return;
    ull smax = X / p2;
    if (smax < (1ULL << TABLE_BITS)) {
        for (ull s = 1; s <= smax; s++) if (ptab[s] <= p) window_at(p2 * s, p, S);
        return;
    }
    /* p-smooth s <= smax: each s = q_1 q_2 ... with q_1 >= q_2 >= ..., built largest prime first */
    long top = 0;
    (*stk)[top].v = 1; (*stk)[top].j = pi; top++;
    while (top) {
        frame f = (*stk)[--top];
        window_at(p2 * f.v, p, S);
        ull room = smax / f.v;                /* a further prime q fits when q <= room */
        if (room < 2) continue;
        long hi = f.j;
        if (plist[hi] > room) {               /* largest index with plist[index] <= room */
            long a = 0, b = hi;               /* plist[a] = 2 <= room < plist[b] */
            while (b - a > 1) { long mid = (a + b) / 2; if (plist[mid] <= room) a = mid; else b = mid; }
            hi = a;
        }
        for (long i = hi; i >= 0; i--) {
            if (top == *stkcap) { *stkcap *= 2; *stk = realloc(*stk, *stkcap * sizeof(frame)); }
            (*stk)[top].v = f.v * plist[i]; (*stk)[top].j = i; top++;
        }
    }
}

static void *thread_main(void *arg) {
    wstate *S = arg;
    long cap = 1 << 16; frame *stk = malloc(cap * sizeof(frame));
    for (;;) {
        pthread_mutex_lock(&job_lock);
        long j0 = next_job; next_job += 8;
        pthread_mutex_unlock(&job_lock);
        if (j0 >= njobs) break;
        long j1 = j0 + 8 < njobs ? j0 + 8 : njobs;
        for (long j = j0; j < j1; j++) do_prime(j, S, &stk, &cap);
    }
    free(stk);
    return NULL;
}

static int by_lo(const void *a, const void *b) {
    const window *x = a, *y = b;
    if (x->lo != y->lo) return x->lo < y->lo ? -1 : 1;
    if (x->hi != y->hi) return x->hi < y->hi ? 1 : -1;
    return 0;
}

int main(int argc, char **argv) {
    if (argc < 4) { fprintf(stderr, "usage: cand2 X MINLEN THREADS\n"); return 1; }
    X = strtoull(argv[1], NULL, 10); MINLEN = atoi(argv[2]); NTHREADS = atoi(argv[3]);
    DUMP = getenv("DUMP") ? strtoull(getenv("DUMP"), NULL, 10) : 0;
    if (X >= (1ULL << 62)) { fprintf(stderr, "X too large\n"); return 1; }

    /* P(s) for s < 2^TABLE_BITS: the last prime to touch s is its largest prime factor */
    ull T = 1ULL << TABLE_BITS;
    ptab = calloc(T, sizeof(uint32_t));
    ptab[1] = 1;
    for (ull q = 2; q < T; q++) {
        if (ptab[q]) continue;                /* composite: already marked by a smaller prime */
        for (ull k = q; k < T; k += q) ptab[k] = (uint32_t)q;
    }
    /* primes up to max(sqrt(X), TRIAL_MAX) by an odd-only bit sieve */
    ull lim = isqrt_ull(X); if (lim < TRIAL_MAX) lim = TRIAL_MAX;
    ull nb = lim / 2 + 1;
    uint8_t *bits = calloc(nb / 8 + 1, 1);    /* bit i set: 2i+1 is composite */
    for (ull i = 1; (2 * i + 1) * (2 * i + 1) <= lim; i++)
        if (!(bits[i >> 3] & (1 << (i & 7))))
            for (ull j = (2 * i + 1) * (2 * i + 1) / 2; j < nb; j += 2 * i + 1) bits[j >> 3] |= (uint8_t)(1 << (j & 7));
    plist = malloc((lim / 2 + 8) * sizeof(ull)); np = 0;
    plist[np++] = 2;
    for (ull i = 1; 2 * i + 1 <= lim; i++) if (!(bits[i >> 3] & (1 << (i & 7)))) plist[np++] = 2 * i + 1;
    free(bits);
    ntrial = 0; while (ntrial < np && plist[ntrial] <= TRIAL_MAX) ntrial++;
    njobs = np;

    wstate *S = calloc(NTHREADS, sizeof(wstate));
    pthread_t th[256];
    for (int t = 0; t < NTHREADS; t++) pthread_create(&th[t], NULL, thread_main, &S[t]);
    for (int t = 0; t < NTHREADS; t++) pthread_join(th[t], NULL);

    ull cands = 0; long nw = 0;
    for (int t = 0; t < NTHREADS; t++) { cands += S[t].cands; nw += S[t].nw; }
    window *W = malloc((nw + 1) * sizeof(window)); long k = 0;
    for (int t = 0; t < NTHREADS; t++) { memcpy(W + k, S[t].w, S[t].nw * sizeof(window)); k += S[t].nw; }
    qsort(W, nw, sizeof(window), by_lo);

    /* components of the union */
    typedef struct { ull lo, hi, m, p; } comp;
    comp *C = malloc((nw + 1) * sizeof(comp)); long nc = 0; long viol = 0;
    for (long i = 0; i < nw; i++) {
        if (nc && W[i].lo <= C[nc - 1].hi + 1) {
            comp *c = &C[nc - 1];
            int nested = W[i].lo >= c->lo && W[i].hi <= c->hi;
            if (!nested) {
                viol++;
                printf("VIOLATION window %llu %llu (m %llu) meets component %llu %llu\n", W[i].lo, W[i].hi, W[i].m, c->lo, c->hi);
                if (W[i].hi > c->hi) c->hi = W[i].hi;
            }
            if (W[i].p > c->p) { c->p = W[i].p; c->m = W[i].m; }
        } else {
            C[nc].lo = W[i].lo; C[nc].hi = W[i].hi; C[nc].m = W[i].m; C[nc].p = W[i].p; nc++;
        }
    }
    printf("X %llu MINLEN %d\n", X, MINLEN);
    printf("CANDIDATES %llu\n", cands);
    printf("VIOLATIONS %ld\n", viol);
    ull firstlo[65] = {0}, count[65] = {0};
    for (long i = 0; i < nc; i++) {
        ull len = C[i].hi - C[i].lo + 1, b = len < 64 ? len : 64;
        if (!count[b]) firstlo[b] = C[i].lo;
        count[b]++;
        if (len >= 10) printf("RUN %llu %llu len %llu m %llu p %llu\n", C[i].lo, C[i].hi, len, C[i].m, C[i].p);
        if (DUMP) for (ull v = C[i].lo; v <= C[i].hi && v <= DUMP; v++) printf("M %llu\n", v);
    }
    for (int b = 1; b <= 64; b++) if (count[b]) printf("FIRST len %d%s start %llu count %llu\n", b, b == 64 ? "+" : "", firstlo[b], count[b]);
    return 0;
}
