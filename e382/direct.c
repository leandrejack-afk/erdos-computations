/* direct.c: reviewer's independent check of A388654 / A388850 from the literal definition.
 * An interval [u,v] of naturals is bad iff the largest prime p dividing u*(u+1)*...*v has
 * total exponent >= 2 in that product. Members = union of bad intervals. Every interval with
 * v - u < LMAX is tested (no square-point enumeration, no window lemma). The scan from u stops
 * early only when no longer interval from u can be bad: p >= LMAX, so p divides at most one
 * member; its exponent is still < 2; and no k ahead within reach has P(k)^2 | k with P(k) > p,
 * which is the only way a longer interval from u could have a new largest prime appearing
 * twice. A FLAG is printed if a scan reaches length LMAX while a longer bad interval is still
 * possible. P(k) by a segmented sieve using exact division by q^-1 mod 2^64, recomputed from
 * scratch for every segment (no state carried between segments).
 * usage: direct N THREADS [DUMPMAX]   -> runs starting in [1, N]
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>

typedef uint64_t u64;
#define LMAX 128
#define G (3 * LMAX)
#define SEG (1u << 18)

static u64 N, DUMPMAX; static int NT;
static u64 *primes, *qinv, *qmax; static long np;
static u64 nseg, nextseg; static pthread_mutex_t mu = PTHREAD_MUTEX_INITIALIZER;

typedef struct { u64 start, len; } run_t;
typedef struct {
    u64 first[LMAX + 2], cnt[LMAX + 2], sq, flags;
    run_t *runs; long nr, cap;
    u64 *rem, *P; uint8_t *e; int *ns, *d; uint8_t *mem;
    char *dumpbuf; size_t dumplen, dumpcap;
} st_t;

static void seg(st_t *T, u64 A) {
    long len = SEG + 2 * G;
    long long base = (long long)A - G;          /* index i <-> k = base + i */
    u64 *rem = T->rem, *P = T->P; uint8_t *e = T->e;
    for (long i = 0; i < len; i++) {
        long long k = base + i;
        rem[i] = k >= 1 ? (u64)k : 0; P[i] = k >= 1 ? 1 : 0; e[i] = 0;
    }
    u64 hi = (u64)(base + len);                 /* exclusive */
    u64 lo = base >= 1 ? (u64)base : 1;
    for (long i = (base & 1) ? 1 : 0; i < len; i += 2) {
        if (!rem[i]) continue;
        int tz = __builtin_ctzll(rem[i]); rem[i] >>= tz; P[i] = 2; e[i] = (uint8_t)tz;
    }
    for (long j = 1; j < np; j++) {
        u64 q = primes[j];
        if (q * q >= hi) break;
        u64 f = lo % q ? lo + (q - lo % q) : lo;
        u64 iq = qinv[j], mq = qmax[j];
        for (u64 k = f; k < hi; k += q) {
            long i = (long)((long long)k - base);
            u64 x = rem[i] * iq; int c = 1;          /* q | x exactly: x * q^-1 is the quotient */
            for (;;) { u64 y = x * iq; if (y > mq) break; x = y; c++; }
            rem[i] = x; P[i] = q; e[i] = (uint8_t)c;
        }
    }
    for (long i = 0; i < len; i++) if (rem[i] > 1) { P[i] = rem[i]; e[i] = 1; }
    /* ns[i] = smallest index >= i with P(k)^2 | k (k >= 2) */
    int *ns = T->ns; ns[len] = 1 << 30;
    for (long i = len - 1; i >= 0; i--) ns[i] = (P[i] >= 2 && e[i] >= 2) ? (int)i : ns[i + 1];
    int *d = T->d; memset(d, 0, (len + 1) * sizeof(int));
    for (long iu = G - LMAX; iu < G + SEG + LMAX; iu++) {
        if (base + iu < 1) continue;
        u64 p = 0; int ex = 0; long maxv = -1;
        for (long iv = iu; iv < iu + LMAX; iv++) {
            u64 q = P[iv];
            if (q > p) { p = q; ex = e[iv]; }
            else if (q == p) ex += e[iv];
            if (ex >= 2 && p >= 2) maxv = iv;
            /* stop when no longer interval from u can be bad: p >= LMAX divides one member only,
               so a later bad [u,v'] needs some j in (v, u+LMAX) with P(j)^2 | j and P(j) > p */
            if (p >= LMAX && ex < 2) {
                long j = ns[iv + 1];
                while (j < iu + LMAX && P[j] <= p) j = ns[j + 1];
                if (j >= iu + LMAX) break;
            }
            /* for u <= LMAX + 1 no bad interval is longer than LMAX (Bertrand: H <= N), so a
               flag there is not a real possibility; above that, p < LMAX cannot happen here
               (Sylvester-Schur), so only ex >= 2 can flag */
            if (iv == iu + LMAX - 1 && (ex >= 2 || p < LMAX) && base + iu > LMAX + 1) {
                T->flags++;
                fprintf(stderr, "FLAG u=%lld\n", base + iu);
            }
        }
        if (maxv >= 0) { d[iu]++; d[maxv + 1]--; }
    }
    uint8_t *mem = T->mem; int acc = 0;
    for (long i = 0; i < len; i++) { acc += d[i]; mem[i] = acc > 0; }
    for (long i = G; i < G + SEG; i++) {
        u64 k = (u64)(base + i);
        if (k > N) break;
        if (P[i] >= 2 && e[i] >= 2) T->sq++;
        if (!mem[i]) continue;
        if (DUMPMAX && k <= DUMPMAX) {
            if (T->dumplen + 32 > T->dumpcap) { T->dumpcap = T->dumpcap ? 2 * T->dumpcap : 1 << 20; T->dumpbuf = realloc(T->dumpbuf, T->dumpcap); }
            T->dumplen += sprintf(T->dumpbuf + T->dumplen, "%llu\n", (unsigned long long)k);
        }
        if (mem[i - 1]) continue;
        long j = i;
        while (mem[j]) { j++; if (j >= G + SEG + LMAX) { T->flags++; fprintf(stderr, "FLAG run at %llu too long\n", (unsigned long long)k); break; } }
        u64 L = (u64)(j - i);
        if (L > LMAX) L = LMAX + 1;
        if (!T->cnt[L] || k < T->first[L]) T->first[L] = k;
        T->cnt[L]++;
        if (L >= 10) {
            if (T->nr == T->cap) { T->cap = T->cap ? 2 * T->cap : 64; T->runs = realloc(T->runs, T->cap * sizeof(run_t)); }
            T->runs[T->nr++] = (run_t){k, L};
        }
    }
}

static void *worker(void *a) {
    st_t *T = a;
    for (;;) {
        pthread_mutex_lock(&mu); u64 s = nextseg++; pthread_mutex_unlock(&mu);
        if (s >= nseg) break;
        seg(T, 1 + s * (u64)SEG);
    }
    return 0;
}

static int cmpu(const void *a, const void *b) { u64 x = *(const u64 *)a, y = *(const u64 *)b; return x < y ? -1 : x > y; }

static int cmpr(const void *a, const void *b) {
    const run_t *x = a, *y = b; return x->start < y->start ? -1 : x->start > y->start;
}

int main(int argc, char **argv) {
    if (argc < 3) { fprintf(stderr, "usage: direct N THREADS [DUMPMAX]\n"); return 1; }
    N = strtoull(argv[1], 0, 10); NT = atoi(argv[2]); DUMPMAX = argc > 3 ? strtoull(argv[3], 0, 10) : 0;
    nseg = (N + SEG - 1) / SEG;
    u64 top = N + SEG + 2 * G + 2, r = 1;
    while ((r + 1) * (r + 1) <= top) r++;
    r += 2;
    char *c = calloc(r + 1, 1); primes = malloc((r + 1) * sizeof(u64)); np = 0;
    for (u64 i = 2; i <= r; i++) { if (c[i]) continue; primes[np++] = i; for (u64 j = i * i; j <= r; j += i) c[j] = 1; }
    qinv = malloc(np * sizeof(u64)); qmax = malloc(np * sizeof(u64));
    for (long j = 0; j < np; j++) {
        u64 q = primes[j], v = 1;
        if (q == 2) { qinv[j] = 0; qmax[j] = 0; continue; }
        for (int t = 0; t < 7; t++) v *= 2 - q * v;   /* Newton: v = q^-1 mod 2^64 */
        if (v * q != 1) { fprintf(stderr, "inverse failed for %llu\n", (unsigned long long)q); return 2; }
        qinv[j] = v; qmax[j] = UINT64_MAX / q;
    }
    st_t *TS = calloc(NT, sizeof(st_t));
    long len = SEG + 2 * G;
    for (int t = 0; t < NT; t++) {
        TS[t].rem = malloc(len * 8); TS[t].P = malloc(len * 8); TS[t].e = malloc(len);
        TS[t].ns = malloc((len + 1) * sizeof(int)); TS[t].d = malloc((len + 1) * sizeof(int)); TS[t].mem = malloc(len);
    }
    pthread_t th[64];
    for (int t = 0; t < NT; t++) pthread_create(&th[t], 0, worker, &TS[t]);
    for (int t = 0; t < NT; t++) pthread_join(th[t], 0);
    u64 first[LMAX + 2] = {0}, cnt[LMAX + 2] = {0}, sq = 0, flags = 0; long nr = 0;
    for (int t = 0; t < NT; t++) {
        sq += TS[t].sq; flags += TS[t].flags; nr += TS[t].nr;
        for (int L = 1; L <= LMAX + 1; L++) if (TS[t].cnt[L]) {
            if (!cnt[L] || TS[t].first[L] < first[L]) first[L] = TS[t].first[L];
            cnt[L] += TS[t].cnt[L];
        }
    }
    run_t *R = malloc((nr + 1) * sizeof(run_t)); long k = 0;
    for (int t = 0; t < NT; t++) { memcpy(R + k, TS[t].runs, TS[t].nr * sizeof(run_t)); k += TS[t].nr; }
    qsort(R, nr, sizeof(run_t), cmpr);
    if (DUMPMAX) {
        /* members in order: segments were split across threads, so gather and sort */
        size_t tot = 0; for (int t = 0; t < NT; t++) tot += TS[t].dumplen;
        u64 *m = malloc((tot / 2 + 1) * sizeof(u64)); long nm = 0;
        for (int t = 0; t < NT; t++) { char *p = TS[t].dumpbuf, *end = p + TS[t].dumplen; while (p < end) { m[nm++] = strtoull(p, &p, 10); p++; } }
        qsort(m, nm, sizeof(u64), cmpu);
        FILE *f = fopen("members.txt", "w"); for (long i = 0; i < nm; i++) fprintf(f, "%llu\n", (unsigned long long)m[i]); fclose(f);
    }
    printf("N %llu LMAX %d\nSQUAREPOINTS %llu\nFLAGS %llu\n", (unsigned long long)N, LMAX, (unsigned long long)sq, (unsigned long long)flags);
    for (long i = 0; i < nr; i++) printf("RUN %llu %llu len %llu\n", (unsigned long long)R[i].start, (unsigned long long)(R[i].start + R[i].len - 1), (unsigned long long)R[i].len);
    for (int L = 1; L <= LMAX + 1; L++) if (cnt[L]) printf("FIRST len %d start %llu count %llu\n", L, (unsigned long long)first[L], (unsigned long long)cnt[L]);
    return 0;
}
