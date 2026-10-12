/*
 * cand.c: maximal bad intervals (OEIS A388654, A388850; Erdős Problem 382) by enumerating
 * the integers m with P(m)^2 | m.
 *
 * P(k) is the largest prime factor of k (P(1) = 1). An interval of integers is bad when its
 * product is divisible by the square of its largest prime factor. For m with P(m)^2 | m let
 * W(m) be the longest interval containing m whose elements all have P(k) <= P(m). Every bad
 * interval lies in W(m) for one such m, and each W(m) is bad, so A388654 is the union of the
 * W(m) and a maximal run of consecutive members equals W(m) for the member m with the
 * largest P (see README.md for the proof). Hence the maximal runs of length >= MINLEN are
 * exactly the windows W(m), m <= X, of length >= MINLEN that lie inside no other such window.
 *
 * Enumeration: m = p^2 * s with p prime, p^2 <= X and P(s) <= p; each m arises once.
 * A neighbour k counts as inside the window when P(k) <= p, decided by trial division by the
 * primes up to min(p, TD) and, for p > TD, a Miller-Rabin test and Pollard-Brent splitting of
 * the cofactor (all of whose prime factors exceed TD).
 *
 * usage: cand X MINLEN THREADS
 * output: CANDIDATES (number of m <= X with P(m)^2 | m), one RUN line per maximal run of
 * length >= MINLEN (start end length m p), FIRST lines (length, first start, count).
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>

typedef uint64_t u64;
typedef unsigned __int128 u128;

#define TD 2048                 /* trial-division bound used when p > TD */

static u64 X;
static int MINLEN, NT;
static u64 *primes;             /* all primes <= sqrt(X) (at least those <= TD) */
static long nprimes;
static u64 *pinv, *plim;        /* q^-1 mod 2^64 and floor((2^64-1)/q), for exact division */
static long ntd;                /* number of primes <= TD */
static int idx_le[TD + 1];      /* index of the largest prime <= v, for v <= TD */

typedef struct { u64 L, R, m, p; } win;

/* ---------- Montgomery arithmetic for odd n < 2^62 ---------- */
typedef struct { u64 n, ninv, one, r2; } mont;

static inline u64 redc(u128 T, const mont *M) {
    u64 m = (u64)T * M->ninv;
    u128 t = (T + (u128)m * M->n) >> 64;
    u64 r = (u64)t;
    return r >= M->n ? r - M->n : r;
}
static inline u64 mmul(u64 a, u64 b, const mont *M) { return redc((u128)a * b, M); }
static void mont_init(mont *M, u64 n) {
    u64 inv = n;                               /* Newton iteration for n^-1 mod 2^64 */
    for (int i = 0; i < 6; i++) inv *= 2 - n * inv;
    M->n = n; M->ninv = (u64)0 - inv;
    M->r2 = (u64)(((u128)1 << 64) % n);        /* R mod n */
    M->one = M->r2;
    M->r2 = (u64)(((u128)M->r2 * M->r2) % n);  /* R^2 mod n */
}
static inline u64 to_m(u64 a, const mont *M) { return mmul(a % M->n, M->r2, M); }

/* deterministic Miller-Rabin with the first k primes as bases (OEIS A014233): k = 7 is exact
   for n < 341550071728321, k = 9 for n < 3825123056546413051, k = 12 for n < 3.18e23 */
static int is_prime(u64 n) {
    static const u64 B[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37};
    if (n < 2) return 0;
    for (int i = 0; i < 12; i++) { if (n == B[i]) return 1; if (n % B[i] == 0) return 0; }
    if (n >= ((u64)1 << 62)) { fprintf(stderr, "is_prime: %llu out of range\n", (unsigned long long)n); exit(2); }
    int nb = n < 341550071728321ULL ? 7 : n < 3825123056546413051ULL ? 9 : 12;
    mont M; mont_init(&M, n);
    u64 d = n - 1; int s = __builtin_ctzll(d); d >>= s;
    u64 mone = to_m(n - 1, &M);
    for (int i = 0; i < nb; i++) {
        u64 a = to_m(B[i], &M), x = M.one, e = d;
        while (e) { if (e & 1) x = mmul(x, a, &M); a = mmul(a, a, &M); e >>= 1; }
        if (x == M.one || x == mone) continue;
        int ok = 0;
        for (int r = 1; r < s; r++) { x = mmul(x, x, &M); if (x == mone) { ok = 1; break; } }
        if (!ok) return 0;
    }
    return 1;
}

static u64 gcd64(u64 a, u64 b) { while (b) { u64 t = a % b; a = b; b = t; } return a; }

static u64 isqrt64(u64 n) {
    u64 r = 0;
    for (u64 b = (u64)1 << 31; b; b >>= 1) { u64 t = r | b; if (t * t <= n) r = t; }
    return r;
}

/* a nontrivial factor of the odd composite n (Pollard-Brent in Montgomery form) */
static u64 rho(u64 n) {
    u64 r0 = isqrt64(n);
    if (r0 * r0 == n) return r0;
    mont M; mont_init(&M, n);
    for (u64 c = 1; c < 1000; c++) {
        u64 cm = to_m(c, &M), y = to_m(2, &M), x = y, ys = y, q = M.one, g = 1;
        u64 r = 1, m = 64;
        while (g == 1) {
            x = y;
            for (u64 i = 0; i < r; i++) y = mmul(y, y, &M) + cm, y = y >= n ? y - n : y;
            for (u64 k = 0; k < r && g == 1; k += m) {
                ys = y;
                u64 lim = (r - k < m) ? r - k : m;
                for (u64 i = 0; i < lim; i++) {
                    y = mmul(y, y, &M) + cm; y = y >= n ? y - n : y;
                    q = mmul(q, x > y ? x - y : y - x, &M);
                }
                g = gcd64(q, n);
            }
            r <<= 1;
        }
        if (g == n) {
            do {
                ys = mmul(ys, ys, &M) + cm; ys = ys >= n ? ys - n : ys;
                g = gcd64(x > ys ? x - ys : ys - x, n);
            } while (g == 1);
        }
        if (g != n && g != 1) return g;
    }
    fprintf(stderr, "rho failed on %llu\n", (unsigned long long)n); exit(2);
}

/* n > 1 with every prime factor > TD: are all its prime factors <= p? */
static int big_smooth(u64 n, u64 p) {
    if (n <= p) return 1;
    if (n < (u64)TD * TD || is_prime(n)) return 0;   /* n is a prime > p */
    u64 d = rho(n);
    return big_smooth(d, p) && big_smooth(n / d, p);
}

/* 1 iff every prime factor of k (k >= 1) is <= p (p prime) */
static int smooth(u64 k, u64 p) {
    k >>= __builtin_ctzll(k);
    if (k <= p) return 1;
    long last = (p <= TD) ? idx_le[p] : ntd - 1;
    for (long i = 1; i <= last; i++) {
        u64 t = k * pinv[i];
        if (t <= plim[i]) {
            do { k = t; t = k * pinv[i]; } while (t <= plim[i]);
            if (k <= p) return 1;
        }
        if (primes[i] * primes[i] > k) return 0;     /* k > p is prime */
    }
    if (p <= TD) return 0;                            /* a prime factor > p remains */
    return big_smooth(k, p);
}

/* ---------- enumeration ---------- */
typedef struct {
    u64 cands;
    win *w; long nw, cap;
    u64 lencount[256];          /* windows (not only maximal ones) by length, capped at 255 */
} tstate;

static long next_prime_idx = 0;
static pthread_mutex_t mu = PTHREAD_MUTEX_INITIALIZER;

static void handle(u64 m, u64 p, tstate *T) {
    T->cands++;
    u64 L = m, R = m;
    while (L > 1 && smooth(L - 1, p)) L--;
    while (smooth(R + 1, p)) R++;
    u64 len = R - L + 1;
    T->lencount[len < 255 ? len : 255]++;
    if (len >= (u64)MINLEN) {
        if (T->nw == T->cap) { T->cap = T->cap ? 2 * T->cap : 1024; T->w = realloc(T->w, T->cap * sizeof(win)); }
        T->w[T->nw++] = (win){L, R, m, p};
    }
}

static void dfs(u64 s, long idx, u64 smax, u64 p, u64 p2, tstate *T) {
    handle(p2 * s, p, T);
    for (long i = 0; i <= idx; i++) {
        u64 q = primes[i];
        if (s > smax / q) break;
        dfs(s * q, i, smax, p, p2, T);
    }
}

static void *worker(void *arg) {
    tstate *T = arg;
    for (;;) {
        pthread_mutex_lock(&mu);
        long i = next_prime_idx++;
        pthread_mutex_unlock(&mu);
        if (i >= nprimes) break;
        u64 p = primes[i], p2 = p * p;
        if (p2 > X) continue;
        u64 smax = X / p2;
        if (smax <= p) { for (u64 s = 1; s <= smax; s++) handle(p2 * s, p, T); }
        else dfs(1, i, smax, p, p2, T);
    }
    return NULL;
}

static int cmpw(const void *a, const void *b) {
    const win *x = a, *y = b;
    if (x->L != y->L) return x->L < y->L ? -1 : 1;
    if (x->R != y->R) return x->R > y->R ? -1 : 1;
    return 0;
}

int main(int argc, char **argv) {
    if (argc < 4) { fprintf(stderr, "usage: cand X MINLEN THREADS\n"); return 1; }
    X = strtoull(argv[1], 0, 10); MINLEN = atoi(argv[2]); NT = atoi(argv[3]);
    u64 sq = isqrt64(X);
    u64 lim = sq > TD ? sq : TD;
    char *comp = calloc(lim + 1, 1);
    for (u64 i = 2; i * i <= lim; i++) if (!comp[i]) for (u64 j = i * i; j <= lim; j += i) comp[j] = 1;
    primes = malloc((lim + 1) * sizeof(u64) / 2 + 64 * sizeof(u64));
    nprimes = 0;
    for (u64 i = 2; i <= lim; i++) if (!comp[i]) primes[nprimes++] = i;
    free(comp);
    pinv = malloc(nprimes * sizeof(u64)); plim = malloc(nprimes * sizeof(u64));
    ntd = 0;
    for (long i = 0; i < nprimes; i++) {
        u64 q = primes[i], inv = q;
        for (int k = 0; k < 6; k++) inv *= 2 - q * inv;
        pinv[i] = inv; plim[i] = UINT64_MAX / q;
        if (q <= TD) ntd = i + 1;
    }
    for (int v = 0, j = -1; v <= TD; v++) { while (j + 1 < nprimes && primes[j + 1] <= (u64)v) j++; idx_le[v] = j; }
    /* primes beyond sqrt(X) are only used for trial division; the enumeration stops at p^2 > X */

    pthread_t th[256]; tstate *TS = calloc(NT, sizeof(tstate));
    for (int t = 0; t < NT; t++) pthread_create(&th[t], 0, worker, &TS[t]);
    for (int t = 0; t < NT; t++) pthread_join(th[t], 0);

    u64 cands = 0; long nw = 0; u64 lc[256] = {0};
    for (int t = 0; t < NT; t++) { cands += TS[t].cands; nw += TS[t].nw; for (int j = 0; j < 256; j++) lc[j] += TS[t].lencount[j]; }
    win *W = malloc((nw + 1) * sizeof(win)); long k = 0;
    for (int t = 0; t < NT; t++) { memcpy(W + k, TS[t].w, TS[t].nw * sizeof(win)); k += TS[t].nw; }
    qsort(W, nw, sizeof(win), cmpw);

    printf("X %llu MINLEN %d\n", (unsigned long long)X, MINLEN);
    printf("CANDIDATES %llu\n", (unsigned long long)cands);
    for (int j = 1; j < 256; j++) if (lc[j]) printf("WINDOWS len %d%s count %llu\n", j, j == 255 ? "+" : "", (unsigned long long)lc[j]);
    u64 first[256] = {0}, cnt[256] = {0}; u64 maxR = 0;
    for (long i = 0; i < nw; i++) {
        if (i > 0 && W[i].R <= maxR) continue;      /* inside an earlier (longer) window */
        maxR = W[i].R;
        u64 len = W[i].R - W[i].L + 1;
        int j = len < 255 ? (int)len : 255;
        if (!cnt[j]) first[j] = W[i].L;
        cnt[j]++;
        if (len >= 10)
            printf("RUN %llu %llu len %llu m %llu p %llu\n", (unsigned long long)W[i].L, (unsigned long long)W[i].R,
                   (unsigned long long)len, (unsigned long long)W[i].m, (unsigned long long)W[i].p);
    }
    for (int j = 1; j < 256; j++) if (cnt[j]) printf("FIRST len %d start %llu count %llu\n", j, (unsigned long long)first[j], (unsigned long long)cnt[j]);
    return 0;
}
