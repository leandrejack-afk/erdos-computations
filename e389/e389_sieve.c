/*
 * e389_sieve.c: exhaustive search for Erdős problem 389.
 *
 * Problem: for n >= 1 find the least k >= 1 with
 *     n (n+1) ... (n+k-1)  |  (n+k) (n+k+1) ... (n+2k-1).
 *
 * Write a = n-1 and N = n+k-1 (the last element of the left block). Then
 *     left  = N!/a!,   right = (2N-a)!/N!,
 * and left | right  <=>  (N!)^2 | a! (2N-a)!  <=>  C(2N, a) | C(2N, N).
 * We search N upward; k = N - a.
 *
 * Prime by prime (Kummer): v_p(C(2N,a)) = #borrows of (2N) - a in base p and
 * v_p(C(2N,N)) = #carries of N + N in base p. For a prime p > a+1 at most one
 * of 2N, 2N-1, ..., 2N-a+1 is divisible by p, and a digit analysis shows the
 * condition at p is exactly: for every j with 2j < a (there are h = ceil(a/2)
 * such j) the number m = N - j satisfies
 *     v_p(m) <= carries_p(m/p^{v_p(m)}),          ("m is a p-governor")
 * where carries_p(x) is the carry count of x + x in base p. Divisibility of
 * 2N - i by p for odd i never fails the condition. Consequences used here:
 *   (1) if m has a prime factor p > sqrt(2m) then m is not a p-governor;
 *   (2) m = p q with p not dividing q is a p-governor iff q has a base-p digit
 *       >= (p+1)/2; if p^2 | m and no digit of m/p is >= (p+1)/2 then m is not
 *       a p-governor.
 * The sieve accumulates floor(4 log2 p) per prime-power factor of every m in a
 * segment, withholds the mass of primes at which (1) or (2) proves m is not a
 * governor (only for p >= RULE_PMIN, which exceeds a+1 for every n handled
 * here), and keeps the m whose accumulated mass reaches a threshold that every
 * fully accounted m provably meets. A run of h consecutive kept m ending at N
 * is a candidate; candidates go through an exact definitional check that
 * factors 2N-i for 0 <= i < a and compares Legendre valuations. Both n (even)
 * and n+1 share the same h, so one scan serves the pair.
 *
 * Threshold soundness (LG = 4, SLACK = 36): for odd p the term floor(4 log2 p)
 * loses less than 1 per prime-power factor, and for p = 2 nothing is lost, so a
 * fully accounted m < 2^49 has mass >= 4 log2 m - 30. A withheld or unsieved
 * prime factor costs at least 4 log2 sqrt(2m) >= 41 for m >= 2^20, or
 * floor(4 log2 RULE_PMIN) = 48 for a rule-withheld prime. Thresholds are taken
 * from the smallest m of each 65536-block, which shifts them by at most 1 unit.
 *
 * Usage:
 *   e389_sieve <n even> <N_lo> <N_hi> [-t threads] [-o survivors.txt] [-x nmax]
 *              [-c checkpoint.txt] [--resume] [--no-rules]
 * -c writes "n covered_below N_hi" every minute (all N below covered_below are
 * fully scanned; chunks finish out of order, so this is the minimum in-flight
 * chunk, not the finished count); --resume restarts from that value.
 * Searches N in [N_lo, N_hi) for n and n+1 and prints RESULT lines. With -x,
 * every survivor is also checked exactly for n+2, ..., nmax: the sieve's
 * rejections are necessary conditions for those larger n as well (their
 * windows contain this one and the withheld primes exceed nmax+1), so a
 * survivor list for n is complete for every n' in [n, nmax] over the same range.
 */
#define _GNU_SOURCE
#include <inttypes.h>
#include <math.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define BLOCK_LOG 16
#define BLOCK (1u << BLOCK_LOG)
#define CHUNK_LOG 26
#define CHUNK (1ull << CHUNK_LOG)
#define SLACK 36
#define RULE_PMIN 4096u
#define SIEVE_START (1ull << 20)
#define MAX_H 32
#define OFF_BITS 27
#define OFF_MASK ((1ull << OFF_BITS) - 1)
#define STRIDE_MAX ((1ull << OFF_BITS) - 1)
#define LG_SHIFT (2 * OFF_BITS)

#define PA (27u * 25u * 7u * 11u * 13u)      /* 675675: 3^3 5^2 7 11 13 */
#define PB (17u * 19u * 23u * 29u)           /* 215441 */

/* ------------------------------------------------------------------ primes */
static uint32_t *P;      /* all primes up to pmax */
static uint8_t *PL;      /* floor(4 log2 p) */
static size_t NP;
static uint32_t pmax;
static size_t first_bucket_prime;  /* index of first prime > BLOCK */

static int bitlen128(unsigned __int128 x)
{
    uint64_t hi = (uint64_t)(x >> 64), lo = (uint64_t)x;
    if (hi) return 128 - __builtin_clzll(hi);
    if (lo) return 64 - __builtin_clzll(lo);
    return 0;
}

static uint8_t lg4(uint64_t p) /* floor(4 log2 p), exact, p < 2^32 */
{
    unsigned __int128 x = (unsigned __int128)p * p;
    x = x * x;
    return (uint8_t)(bitlen128(x) - 1);
}

static void gen_primes(uint32_t limit)
{
    uint8_t *comp = calloc(limit + 1, 1);
    if (!comp) { perror("calloc"); exit(1); }
    size_t cap = 1024;
    P = malloc(cap * sizeof *P);
    PL = malloc(cap);
    NP = 0;
    for (uint64_t i = 2; i <= limit; i++) {
        if (comp[i]) continue;
        if (NP == cap) { cap *= 2; P = realloc(P, cap * sizeof *P); PL = realloc(PL, cap); }
        P[NP] = (uint32_t)i; PL[NP] = lg4(i); NP++;
        for (uint64_t j = i * i; j <= limit; j += i) comp[j] = 1;
    }
    free(comp);
    pmax = limit;
    first_bucket_prime = NP;
    for (size_t i = 0; i < NP; i++) if (P[i] > BLOCK) { first_bucket_prime = i; break; }
}

/* ------------------------------------------------------ exact arithmetic */
static uint64_t mulmod(uint64_t a, uint64_t b, uint64_t m)
{
    return (uint64_t)((unsigned __int128)a * b % m);
}

static uint64_t powmod(uint64_t a, uint64_t e, uint64_t m)
{
    uint64_t r = 1;
    a %= m;
    while (e) { if (e & 1) r = mulmod(r, a, m); a = mulmod(a, a, m); e >>= 1; }
    return r;
}

static int is_prime64(uint64_t n)
{
    if (n < 2) return 0;
    static const uint64_t small[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37};
    for (int i = 0; i < 12; i++) { if (n == small[i]) return 1; if (n % small[i] == 0) return 0; }
    uint64_t d = n - 1; int s = 0;
    while (!(d & 1)) { d >>= 1; s++; }
    static const uint64_t bases[] = {2, 325, 9375, 28178, 450775, 9780504, 1795265022};
    for (int i = 0; i < 7; i++) {
        uint64_t a = bases[i] % n;
        if (a == 0) continue;
        uint64_t x = powmod(a, d, n);
        if (x == 1 || x == n - 1) continue;
        int ok = 0;
        for (int r = 1; r < s; r++) { x = mulmod(x, x, n); if (x == n - 1) { ok = 1; break; } }
        if (!ok) return 0;
    }
    return 1;
}

static uint64_t gcd64(uint64_t a, uint64_t b)
{
    while (b) { uint64_t t = a % b; a = b; b = t; }
    return a;
}

static uint64_t pollard_brent(uint64_t n)
{
    if (!(n & 1)) return 2;
    for (uint64_t c = 1;; c++) {
        uint64_t y = 2, x = 2, q = 1, g = 1, ys = 0;
        uint64_t r = 1;
        while (g == 1) {
            x = y;
            for (uint64_t i = 0; i < r; i++) y = (mulmod(y, y, n) + c) % n;
            uint64_t k = 0;
            while (k < r && g == 1) {
                ys = y;
                uint64_t lim = (r - k < 128) ? r - k : 128;
                for (uint64_t i = 0; i < lim; i++) {
                    y = (mulmod(y, y, n) + c) % n;
                    uint64_t diff = x > y ? x - y : y - x;
                    q = mulmod(q, diff, n);
                }
                g = gcd64(q, n);
                k += lim;
            }
            r *= 2;
        }
        if (g == n) {
            do {
                ys = (mulmod(ys, ys, n) + c) % n;
                uint64_t diff = x > ys ? x - ys : ys - x;
                g = gcd64(diff, n);
            } while (g == 1);
        }
        if (g != n) return g;
    }
}

/* Append the distinct prime factors of x to out (unsorted, may repeat across calls). */
static void factor_into(uint64_t x, uint64_t *out, int *cnt)
{
    for (size_t i = 0; i < NP && P[i] <= 65536; i++) {
        uint64_t p = P[i];
        if (p * p > x) break;
        if (x % p == 0) { out[(*cnt)++] = p; do x /= p; while (x % p == 0); }
    }
    if (x == 1) return;
    if (x < 65536ull * 65536ull || is_prime64(x)) { out[(*cnt)++] = x; return; }
    uint64_t stack[64]; int sp = 0;
    stack[sp++] = x;
    while (sp) {
        uint64_t y = stack[--sp];
        if (y == 1) continue;
        if (is_prime64(y)) { out[(*cnt)++] = y; continue; }
        uint64_t d = pollard_brent(y);
        stack[sp++] = d; stack[sp++] = y / d;
    }
}

static uint64_t vfact(uint64_t x, uint64_t p) /* v_p(x!) by Legendre */
{
    uint64_t s = 0;
    while (x) { x /= p; s += x; }
    return s;
}

/* Definitional check: does C(2N, a) divide C(2N, N)?  Sets *bad to a failing prime. */
static int exact389(uint64_t N, uint64_t a, uint64_t *bad)
{
    uint64_t fac[64 * 40]; int cnt = 0;
    for (uint64_t i = 0; i < a; i++) factor_into(2 * N - i, fac, &cnt);
    for (int i = 0; i < cnt; i++) {
        uint64_t p = fac[i];
        int dup = 0;
        for (int j = 0; j < i; j++) if (fac[j] == p) { dup = 1; break; }
        if (dup) continue;
        uint64_t top = vfact(2 * N, p);
        uint64_t v1 = top - vfact(a, p) - vfact(2 * N - a, p);
        uint64_t v2 = top - 2 * vfact(N, p);
        if (v1 > v2) { *bad = p; return 0; }
    }
    *bad = 0;
    return 1;
}

/* ------------------------------------------------------------- sieve data */
typedef struct { uint32_t q, p; uint8_t lg, rule; } strider_t;
typedef struct { uint64_t q; uint8_t lg; } bigpow_t;

static strider_t *SD; static size_t NSD;
static bigpow_t *BP; static size_t NBP;
static uint8_t *patA, *patB, *patZ;   /* presieve patterns, each period + BLOCK long */

#define MAX_AL 16
static uint64_t g_al[MAX_AL];    /* a = n'-1 for each n' checked exactly */
static int g_nal;
static _Atomic uint64_t g_best[MAX_AL];
static uint32_t g_h;             /* window length */
static int g_rules = 1;
static uint64_t g_Nhi;
static uint64_t g_L0;            /* first chunk base */
static uint64_t g_nchunks;
static uint32_t g_chunk_w;       /* CHUNK + h - 1 */
static uint32_t g_nblocks;
static _Atomic uint64_t g_next_chunk;
static _Atomic uint64_t g_cands, g_done_chunks;
#define MAX_THREADS 256
static _Atomic uint64_t g_inflight[MAX_THREADS];   /* chunk id being processed, UINT64_MAX if none */
static int g_nthreads;
static uint64_t g_n_even;
static char g_ckpt[512];                           /* checkpoint file path */

/* every N below this value has been fully scanned (all lower chunks finished) */
static uint64_t covered_below(void)
{
    uint64_t safe = atomic_load(&g_next_chunk);
    if (safe > g_nchunks) safe = g_nchunks;
    for (int i = 0; i < g_nthreads; i++) {
        uint64_t c = atomic_load(&g_inflight[i]);
        if (c < safe) safe = c;
    }
    return g_L0 + safe * CHUNK + g_h - 1;
}

static void write_checkpoint(void)
{
    if (!g_ckpt[0]) return;
    char tmp[600];
    snprintf(tmp, sizeof tmp, "%s.tmp", g_ckpt);
    FILE *f = fopen(tmp, "w");
    if (!f) return;
    fprintf(f, "%" PRIu64 " %" PRIu64 " %" PRIu64 "\n", g_n_even, covered_below(), g_Nhi);
    fclose(f);
    rename(tmp, g_ckpt);
}
static FILE *g_surv;
static pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;

static int cmp_bigpow(const void *x, const void *y)
{
    uint64_t a = ((const bigpow_t *)x)->q, b = ((const bigpow_t *)y)->q;
    return a < b ? -1 : a > b;
}

static void build_tables(uint64_t Nhi)
{
    size_t lenA = PA + BLOCK, lenB = PB + BLOCK, lenZ = 65536 + BLOCK;
    patA = calloc(lenA, 1); patB = calloc(lenB, 1); patZ = calloc(lenZ, 1);
    const uint32_t pa[5] = {3, 5, 7, 11, 13}, capa[5] = {3, 2, 1, 1, 1};
    for (int t = 0; t < 5; t++) {
        uint8_t lg = lg4(pa[t]);
        uint64_t q = pa[t];
        for (uint32_t e = 1; e <= capa[t]; e++, q *= pa[t])
            for (size_t j = 0; j < lenA; j += q) patA[j] += lg;
    }
    const uint32_t pb[4] = {17, 19, 23, 29};
    for (int t = 0; t < 4; t++) {
        uint8_t lg = lg4(pb[t]);
        for (size_t j = 0; j < lenB; j += pb[t]) patB[j] += lg;
    }
    for (size_t i = 1; i < lenZ; i++) {
        uint32_t r = (uint32_t)(i & 65535);
        patZ[i] = r ? (uint8_t)(4 * __builtin_ctz(r)) : 0;
    }
    /* striders: odd prime powers q <= BLOCK not covered by the patterns */
    size_t capS = 16384; SD = malloc(capS * sizeof *SD); NSD = 0;
    size_t capB = 65536; BP = malloc(capB * sizeof *BP); NBP = 0;
    for (size_t i = 1; i < NP; i++) {
        uint32_t p = P[i];
        if ((uint64_t)p * p > Nhi && p > BLOCK) break;
        uint64_t q = p; uint32_t e = 1;
        while (q <= BLOCK) {
            int covered = 0;
            if (p == 3 && e <= 3) covered = 1;
            if (p == 5 && e <= 2) covered = 1;
            if ((p == 7 || p == 11 || p == 13 || p == 17 || p == 19 || p == 23 || p == 29) && e == 1) covered = 1;
            if (!covered) {
                if (NSD == capS) { capS *= 2; SD = realloc(SD, capS * sizeof *SD); }
                SD[NSD++] = (strider_t){(uint32_t)q, p, PL[i], (uint8_t)(e == 1 && p >= RULE_PMIN)};
            }
            q *= p; e++;
        }
        while (q <= Nhi) {
            if (NBP == capB) { capB *= 2; BP = realloc(BP, capB * sizeof *BP); }
            BP[NBP++] = (bigpow_t){q, PL[i]};
            if (q > Nhi / p) break;
            q *= p;
        }
    }
    qsort(BP, NBP, sizeof *BP, cmp_bigpow);
}

/* ------------------------------------------------------------ per thread */
typedef struct { uint64_t *d; uint32_t n, cap; } bucket_t;

static inline void bpush(bucket_t *b, uint64_t item)
{
    if (b->n == b->cap) {
        b->cap = b->cap ? 2 * b->cap : 4096;
        b->d = realloc(b->d, b->cap * sizeof(uint64_t));
        if (!b->d) { perror("realloc"); exit(1); }
    }
    b->d[b->n++] = item;
}

typedef struct { uint32_t q, off; uint8_t lg; } act_t;  /* active strider in this chunk */

typedef struct {
    int id;
    uint8_t *acc;          /* MAX_H + BLOCK bytes */
    act_t *act;
    bucket_t *bk;          /* g_nblocks + 1 buckets */
    uint64_t cands;
} thread_t;

static inline int digits_small(uint64_t c, uint32_t p, uint32_t half)
{
    while (c) { if (c % p > half) return 0; c /= p; }
    return 1;
}

static uint64_t isqrt64(uint64_t x)
{
    uint64_t r = (uint64_t)sqrtl((long double)x);
    while (r * r > x) r--;
    while ((r + 1) * (r + 1) <= x) r++;
    return r;
}

static uint64_t stop_point(void)
{
    uint64_t s = 0;
    for (int i = 0; i < g_nal; i++) { uint64_t b = atomic_load(&g_best[i]); if (b > s) s = b; }
    return s;
}

static void check_all(uint64_t N, int *ok, uint64_t *bad)
{
    for (int i = 0; i < g_nal; i++) {
        ok[i] = -1; bad[i] = 0;
        if (N <= g_al[i] || N >= atomic_load(&g_best[i])) continue;
        ok[i] = exact389(N, g_al[i], &bad[i]);
        if (ok[i]) {
            uint64_t cur = atomic_load(&g_best[i]);
            while (N < cur && !atomic_compare_exchange_weak(&g_best[i], &cur, N)) {}
        }
    }
}

static void report_candidate(thread_t *t, uint64_t N)
{
    if (N >= g_Nhi) return;
    t->cands++;
    int ok[MAX_AL]; uint64_t bad[MAX_AL];
    check_all(N, ok, bad);
    if (g_surv) {
        int hit = 0;
        pthread_mutex_lock(&g_lock);
        fprintf(g_surv, "%" PRIu64, N);
        for (int i = 0; i < g_nal; i++) { fprintf(g_surv, " %d:%" PRIu64, ok[i], bad[i]); if (ok[i] == 1) hit = 1; }
        fputc('\n', g_surv);
        if (hit) fflush(g_surv);
        pthread_mutex_unlock(&g_lock);
    }
}

#ifdef PROF
static _Atomic uint64_t prof_ns[6];
static inline uint64_t pnow(void) { struct timespec ts; clock_gettime(CLOCK_MONOTONIC_RAW, &ts); return (uint64_t)ts.tv_sec * 1000000000ull + ts.tv_nsec; }
#define PT(i, stmt) do { uint64_t _t0 = pnow(); stmt; atomic_fetch_add(&prof_ns[i], pnow() - _t0); } while (0)
#else
#define PT(i, stmt) do { stmt; } while (0)
#endif

static void process_chunk(thread_t *t, uint64_t L)
{
    const uint32_t h = g_h;
    const uint64_t W = g_chunk_w;
    const uint64_t R = L + W - 1;
    const uint64_t Pc = isqrt64(2 * R) + 1;
    uint8_t *acc = t->acc;
    act_t *act = t->act;
    bucket_t *bk = t->bk;

    /* striders: compact list of the prime powers with a hit in this chunk; a
       base prime p >= RULE_PMIN is dropped for the whole chunk when every
       multiple in the chunk is provably not a p-governor */
    size_t na = 0;
#ifdef PROF
    uint64_t _ts = pnow();
#endif
    for (size_t i = 0; i < NSD; i++) {
        const strider_t *s = &SD[i];
        if (s->p > Pc) continue;
        uint64_t m0 = ((L + s->q - 1) / s->q) * s->q;
        if (m0 > R) continue;
        if (g_rules && s->rule) {
            uint64_t p = s->p, qlo = m0 / p, qhi = R / p, c = qhi / p;
            if (qlo >= c * p && qhi - c * p <= (p - 1) / 2 && digits_small(c, (uint32_t)p, (uint32_t)((p - 1) / 2)))
                continue;
        }
        act[na++] = (act_t){s->q, (uint32_t)(m0 - L), s->lg};
    }
    /* bucket primes in (BLOCK, Pc] */
    for (size_t i = first_bucket_prime; i < NP; i++) {
        uint64_t p = P[i];
        if (p > Pc) break;
        uint64_t qlo = (L + p - 1) / p, qhi = R / p;
        if (qlo > qhi) continue;
        if (g_rules) {
            uint64_t c = qhi / p;
            if (qlo >= c * p && qhi - c * p <= (p - 1) / 2 && digits_small(c, (uint32_t)p, (uint32_t)((p - 1) / 2)))
                continue;
        }
        uint64_t off = qlo * p - L;
        bpush(&bk[off >> BLOCK_LOG], off | (p << OFF_BITS) | ((uint64_t)PL[i] << LG_SHIFT));
    }
    /* big prime powers */
    for (size_t i = 0; i < NBP; i++) {
        uint64_t q = BP[i].q;
        if (q > R) break;
        uint64_t m = ((L + q - 1) / q) * q;
        if (m > R) continue;
        uint64_t lgf = (uint64_t)BP[i].lg << LG_SHIFT;
        if (q <= STRIDE_MAX) {
            uint64_t off = m - L;
            bpush(&bk[off >> BLOCK_LOG], off | (q << OFF_BITS) | lgf);
        } else {
            for (; m <= R; m += q) {
                uint64_t off = m - L;
                bpush(&bk[off >> BLOCK_LOG], off | (STRIDE_MAX << OFF_BITS) | lgf);
            }
        }
    }

#ifdef PROF
    atomic_fetch_add(&prof_ns[0], pnow() - _ts);
#endif
    memset(acc, 0, h);  /* windows ending before L + h - 1 belong to the previous chunk */
    uint8_t *buf = acc + (h - 1);
    for (uint32_t b = 0; b < g_nblocks; b++) {
        uint64_t bL = L + (uint64_t)b * BLOCK;
        uint32_t size = (uint32_t)((W - (uint64_t)b * BLOCK) < BLOCK ? (W - (uint64_t)b * BLOCK) : BLOCK);
        if (size == 0) break;
        const uint8_t *pz = patZ + (bL & 65535);
        const uint8_t *pa = patA + (bL % PA);
        const uint8_t *pb = patB + (bL % PB);
        PT(1, {
        for (uint32_t i = 0; i < size; i++) buf[i] = (uint8_t)(pz[i] + pa[i] + pb[i]);
        for (uint64_t m = (bL + 65535) & ~65535ull; m < bL + size; m += 65536)
            buf[m - bL] += (uint8_t)(4 * __builtin_ctzll(m));
        });

        PT(2, {
        for (size_t i = 0; i < na; i++) {
            act_t *a = &act[i];
            uint32_t j = a->off;
            if (j < size) {
                const uint32_t q = a->q; const uint8_t lg = a->lg;
                for (; j < size; j += q) buf[j] += lg;
            }
            a->off = j - size;
        }
        });

        PT(3, {
        bucket_t *cur = &bk[b];
        for (uint32_t i = 0; i < cur->n; i++) {
            uint64_t item = cur->d[i];
            uint32_t off = (uint32_t)(item & OFF_MASK);
            buf[off & (BLOCK - 1)] += (uint8_t)(item >> LG_SHIFT);
            uint64_t next = off + ((item >> OFF_BITS) & OFF_MASK);
            if (next < W) bpush(&bk[next >> BLOCK_LOG], (item & ~OFF_MASK) | next);
        }
        cur->n = 0;
        });

        int T = (int)floor(4.0 * log2((double)bL)) - SLACK;
        if (T < 0) T = 0;
        uint8_t Tb = (uint8_t)T;
        /* windows of h consecutive bytes >= Tb ending at buffer index i, i in [h-1, h-1+size) */
        PT(4, {
        uint32_t i = h - 1;
        uint32_t end = h - 1 + size;
        while (i < end) {
            if (acc[i] < Tb) { i += h; continue; }
            uint32_t tb = 1;
            while (tb < h && acc[i - tb] >= Tb) tb++;
            if (tb == h) { report_candidate(t, bL + i - (h - 1)); i++; }
            else i = i - tb + h;
        }
        if (size >= h - 1) memmove(acc, buf + size - (h - 1), h - 1);
        else { memmove(acc, acc + size, h - 1); }
        });
    }
}

static void *worker(void *arg)
{
    thread_t *t = arg;
    for (;;) {
        /* claim a conservative lower bound before fetching, so covered_below()
           never counts a chunk this thread is about to take as finished */
        atomic_store(&g_inflight[t->id], atomic_load(&g_next_chunk));
        uint64_t id = atomic_fetch_add(&g_next_chunk, 1);
        if (id >= g_nchunks) { atomic_store(&g_inflight[t->id], UINT64_MAX); break; }
        atomic_store(&g_inflight[t->id], id);
        uint64_t L = g_L0 + id * CHUNK;
        if (L > stop_point()) break;   /* chunk id stays marked: it was not scanned */
        process_chunk(t, L);
        atomic_store(&g_inflight[t->id], UINT64_MAX);
        atomic_fetch_add(&g_done_chunks, 1);
    }
    atomic_fetch_add(&g_cands, t->cands);
    return NULL;
}

/* brute force for small N: every N in [lo, hi) through the exact check */
typedef struct { uint64_t lo, hi; int id, nt; } bf_t;
static void *bf_worker(void *arg)
{
    bf_t *b = arg;
    int ok[MAX_AL]; uint64_t bad[MAX_AL];
    for (uint64_t N = b->lo + b->id; N < b->hi; N += b->nt) check_all(N, ok, bad);
    return NULL;
}

static double now(void)
{
    struct timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + 1e-9 * ts.tv_nsec;
}

static _Atomic int g_finished;
static double g_t0;

static void *progress_worker(void *arg)
{
    (void)arg;
    double tl = now(); uint64_t last = 0;
    while (!atomic_load(&g_finished)) {
        struct timespec w = {1, 0}; nanosleep(&w, 0);
        double tn = now();
        if (tn - tl < 60) continue;
        uint64_t done = atomic_load(&g_done_chunks);
        fprintf(stderr, "progress: all N < %" PRIu64 " covered (%.0f M/s, %.1f min, chunks %" PRIu64 "/%" PRIu64 ")\n",
                covered_below(), (double)(done - last) * CHUNK / (tn - tl) / 1e6, (tn - g_t0) / 60, done, g_nchunks);
        write_checkpoint();
        tl = tn; last = done;
    }
    return NULL;
}

int main(int argc, char **argv)
{
    if (argc < 4) {
        fprintf(stderr, "usage: %s <n even> <N_lo> <N_hi> [-t threads] [-o survivors] [--no-rules]\n", argv[0]);
        return 2;
    }
    uint64_t n = strtoull(argv[1], 0, 10);
    uint64_t Nlo = strtoull(argv[2], 0, 10), Nhi = strtoull(argv[3], 0, 10);
    int nt = 1, resume = 0;
    uint64_t nmax = n + 1;
    const char *survpath = NULL;
    for (int i = 4; i < argc; i++) {
        if (!strcmp(argv[i], "-t") && i + 1 < argc) nt = atoi(argv[++i]);
        else if (!strcmp(argv[i], "-o") && i + 1 < argc) survpath = argv[++i];
        else if (!strcmp(argv[i], "-x") && i + 1 < argc) nmax = strtoull(argv[++i], 0, 10);
        else if (!strcmp(argv[i], "-c") && i + 1 < argc) snprintf(g_ckpt, sizeof g_ckpt, "%s", argv[++i]);
        else if (!strcmp(argv[i], "--resume")) resume = 1;
        else if (!strcmp(argv[i], "--no-rules")) g_rules = 0;
        else { fprintf(stderr, "bad arg %s\n", argv[i]); return 2; }
    }
    if (nt < 1 || nt > MAX_THREADS) { fprintf(stderr, "bad -t\n"); return 2; }
    g_nthreads = nt; g_n_even = n;
    if (resume) {
        FILE *f = g_ckpt[0] ? fopen(g_ckpt, "r") : NULL;
        uint64_t cn = 0, cov = 0, chi = 0;
        if (f && fscanf(f, "%" SCNu64 " %" SCNu64 " %" SCNu64, &cn, &cov, &chi) == 3 && cn == n && chi == Nhi && cov > Nlo) {
            fprintf(stderr, "resume: checkpoint says all N < %" PRIu64 " covered, starting there\n", cov);
            Nlo = cov;
        } else fprintf(stderr, "resume: no usable checkpoint, starting from N_lo\n");
        if (f) fclose(f);
    }
    if (n < 2 || (n & 1)) { fprintf(stderr, "n must be even and >= 2\n"); return 2; }
    if (Nhi >= (1ull << 49)) { fprintf(stderr, "N_hi too large for the u8 accumulator analysis\n"); return 2; }
    if (nmax < n + 1 || nmax - n + 1 > MAX_AL || nmax + 1 >= RULE_PMIN) { fprintf(stderr, "bad -x\n"); return 2; }
    g_h = (uint32_t)(n / 2);
    if (g_h > MAX_H) { fprintf(stderr, "h too large\n"); return 2; }
    g_nal = 0;
    for (uint64_t m = n; m <= nmax; m++) { g_al[g_nal] = m - 1; atomic_store(&g_best[g_nal], UINT64_MAX); g_nal++; }
    g_Nhi = Nhi;
    if (Nlo < n) Nlo = n;

    double t0 = now();
    uint32_t plim = (uint32_t)(isqrt64(2 * (Nhi + CHUNK + MAX_H)) + 2);
    if (plim < 200000) plim = 200000;
    if (plim >= (1u << OFF_BITS)) { fprintf(stderr, "prime bound too large for item packing\n"); return 2; }
    gen_primes(plim);
    build_tables(Nhi + CHUNK + MAX_H);
    fprintf(stderr, "n=%" PRIu64 "..%" PRIu64 " h=%u primes<=%u: %zu striders=%zu bigpows=%zu rules=%d threads=%d (%.1fs setup)\n",
            n, nmax, g_h, plim, NP, NSD, NBP, g_rules, nt, now() - t0);

    if (survpath) { g_surv = fopen(survpath, "w"); if (!g_surv) { perror(survpath); return 1; } }

    /* brute force part */
    uint64_t bf_hi = Nhi < SIEVE_START ? Nhi : SIEVE_START;
    if (Nlo < bf_hi) {
        pthread_t th[256]; bf_t args[256];
        for (int i = 0; i < nt; i++) { args[i] = (bf_t){Nlo, bf_hi, i, nt}; pthread_create(&th[i], 0, bf_worker, &args[i]); }
        for (int i = 0; i < nt; i++) pthread_join(th[i], 0);
        fprintf(stderr, "brute force [%" PRIu64 ", %" PRIu64 ") done (%.1fs)\n", Nlo, bf_hi, now() - t0);
    }

    /* sieve part */
    uint64_t Nstart = Nlo > SIEVE_START ? Nlo : SIEVE_START;
    if (Nstart < Nhi) {
        g_chunk_w = (uint32_t)(CHUNK + g_h - 1);
        g_nblocks = (g_chunk_w + BLOCK - 1) / BLOCK;
        g_L0 = Nstart - (g_h - 1);
        g_nchunks = (Nhi - g_L0 + CHUNK - 1) / CHUNK;
        atomic_store(&g_next_chunk, 0);
        for (int i = 0; i < MAX_THREADS; i++) atomic_store(&g_inflight[i], UINT64_MAX);
        thread_t *ts = calloc(nt, sizeof *ts);
        pthread_t th[256];
        for (int i = 0; i < nt; i++) {
            ts[i].id = i;
            ts[i].acc = calloc(MAX_H + BLOCK + 64, 1);
            ts[i].act = calloc(NSD, sizeof(act_t));
            ts[i].bk = calloc(g_nblocks + 2, sizeof(bucket_t));
            pthread_create(&th[i], 0, worker, &ts[i]);
        }
        pthread_t prog;
        g_t0 = t0;
        pthread_create(&prog, 0, progress_worker, 0);
        for (int i = 0; i < nt; i++) pthread_join(th[i], 0);
        atomic_store(&g_finished, 1);
        pthread_join(prog, 0);
        write_checkpoint();
        fprintf(stderr, "sieve done: chunks=%" PRIu64 " candidates=%" PRIu64 " (%.1fs)\n",
                atomic_load(&g_done_chunks), atomic_load(&g_cands), now() - t0);
#ifdef PROF
        fprintf(stderr, "prof (s): setup %.2f init %.2f striders %.2f buckets %.2f scan %.2f\n",
                prof_ns[0] / 1e9, prof_ns[1] / 1e9, prof_ns[2] / 1e9, prof_ns[3] / 1e9, prof_ns[4] / 1e9);
#endif
    }
    if (g_surv) fclose(g_surv);

    for (int i = 0; i < g_nal; i++) {
        uint64_t b = atomic_load(&g_best[i]), a = g_al[i];
        if (b != UINT64_MAX) printf("RESULT n=%" PRIu64 " N=%" PRIu64 " k=%" PRIu64 "\n", a + 1, b, b - a);
        else printf("RESULT n=%" PRIu64 " NONE in [%" PRIu64 ", %" PRIu64 ")\n", a + 1, Nlo, Nhi);
    }
    printf("TIME %.1f s\n", now() - t0);
    return 0;
}
