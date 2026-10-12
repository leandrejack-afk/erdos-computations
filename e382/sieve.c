/*
 * sieve.c: maximal bad intervals (OEIS A388654, A388850; Erdős Problem 382) by a segmented
 * sieve that factors every integer in a range.
 *
 * For each k the sieve divides out every prime q <= sqrt(range end) and records the largest
 * such q with its exponent; the cofactor left over is 1 or a prime. So P(k), the largest prime
 * factor, is known for every k, and so is whether P(k)^2 | k. For every m with P(m)^2 | m the
 * window W(m) (the longest interval around m on which P(k) <= P(m)) is marked; a marked
 * integer is a member of A388654 (every bad interval lies in some W(m), and each W(m) is bad;
 * see README.md). Runs are then read off the marks: a run starts at k when k is marked and
 * k-1 is not, and its length is the number of consecutive marked integers from k.
 *
 * Windows and runs are assumed shorter than MARGIN; any window or run that reaches the
 * margin stops the program with an error, so a successful run proves the assumption.
 *
 * usage: sieve LO HI THREADS [SEGBITS]     (runs starting in [LO, HI))
 *        DUMP=1 sieve LO HI 1              (also print every member of A388654 in [LO, HI))
 * output: CANDIDATES (k in [LO, HI) with P(k)^2 | k), one RUN line per run of length >= 10
 * (start end length), FIRST lines (length, first start, count).
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>

typedef uint64_t u64;
typedef int64_t i64;

#define MARGIN 64
#define MAXLEN MARGIN

static u64 LO, HI; static int NT, SEGBITS = 18; static int DUMP = 0;
static u64 S;                     /* segment length */
static u64 CHUNK;                 /* segments per work unit */
static u64 *pr, *pinv, *plim; static long npr;
static u64 nseg, next_seg = 0;
static pthread_mutex_t mu = PTHREAD_MUTEX_INITIALIZER;

typedef struct { u64 start, len; } run;
typedef struct {
    u64 cands, first[MAXLEN + 1], cnt[MAXLEN + 1];
    run *runs; long nr, cap;
    u64 *rem; uint32_t *lpf; uint8_t *mem; u64 *nm; long *cl;
} tstate;

static void fail(const char *what, u64 k) {
    fprintf(stderr, "error: %s near %llu (raise MARGIN)\n", what, (unsigned long long)k); exit(3);
}

static void do_segment(tstate *T, u64 A, u64 B, int fresh) {
    /* array index i <-> k = base + i, k from A - 3*MARGIN to B + 3*MARGIN - 1 */
    i64 base = (i64)A - 3 * MARGIN;
    u64 len = (B - A) + 6 * MARGIN;
    u64 *rem = T->rem; uint32_t *lpf = T->lpf; uint8_t *mem = T->mem;
    for (u64 i = 0; i < len; i++) {
        i64 k = base + (i64)i;
        if (k >= 1) { rem[i] = (u64)k; lpf[i] = 2; }       /* (1 << 1) | 0: no prime yet */
        else { rem[i] = 0; lpf[i] = 0; }
        mem[i] = 0;
    }
    /* q = 2 */
    for (u64 i = (base & 1) ? 1 : 0; i < len; i += 2) {
        if (!rem[i]) continue;
        int tz = __builtin_ctzll(rem[i]);
        rem[i] >>= tz; lpf[i] = (2u << 1) | (tz >= 2);
    }
    i64 nb = base + (i64)S;           /* base of the next segment in this work unit */
    /* every prime up to sqrt(HI + S + 3*MARGIN) is used in every segment, so the cofactor
       left in rem[] is 1 or a prime larger than every prime that was divided out */
    for (long j = 1; j < npr; j++) {
        u64 q = pr[j];
        u64 inv = pinv[j], lim = plim[j];
        u64 f;
        if (fresh) {
            u64 k0 = base >= 1 ? (u64)base : 1;
            u64 r = k0 % q;
            f = r ? k0 + (q - r) : k0;
        } else f = T->nm[j];
        u64 i = f - (u64)base;
        for (; i < len; i += q) {
            u64 t = rem[i] * inv, e = 1, u = t * inv;
            while (u <= lim) { t = u; u = t * inv; e++; }
            rem[i] = t; lpf[i] = ((uint32_t)q << 1) | (e >= 2);
        }
        /* first multiple of q that is >= nb, for the next segment of this work unit */
        u64 kn = (u64)base + i;                              /* first multiple past the array */
        u64 d = kn - (u64)nb;
        T->nm[j] = (q > 6 * MARGIN) ? (d >= q ? kn - q : kn) : kn - (d / q) * q;
    }
    /* P(k) in rem[], candidates listed */
    long nc = 0;
    u64 clo = MARGIN, chi = len - MARGIN;                   /* candidates in [A-2M, B+2M) */
    for (u64 i = 0; i < len; i++) {
        if (!lpf[i]) { rem[i] = UINT64_MAX; continue; }    /* k <= 0 blocks every window */
        if (rem[i] == 1) {
            if ((lpf[i] & 1) && i >= clo && i < chi) T->cl[nc++] = (long)i;
            rem[i] = lpf[i] >> 1;
        }
    }
    for (long c = 0; c < nc; c++) {
        u64 i = (u64)T->cl[c], p = rem[i], l = i, r = i;
        while (l > 0 && rem[l - 1] <= p) l--;
        while (r + 1 < len && rem[r + 1] <= p) r++;
        if (l == 0 || r == len - 1 || r - l + 1 >= MARGIN) fail("window as long as the margin", (u64)(base + (i64)i));
        memset(mem + l, 1, r - l + 1);
        u64 k = (u64)(base + (i64)i);
        if (k >= A && k < B) T->cands++;
    }
    /* runs starting in [A, B) */
    for (u64 i = 3 * MARGIN; i < 3 * MARGIN + (B - A); i++) {
        if (!mem[i]) continue;
        u64 k = (u64)(base + (i64)i);
        if (DUMP) printf("M %llu\n", (unsigned long long)k);
        if (mem[i - 1]) continue;
        u64 j = i;
        while (mem[j]) { j++; if (j >= len - 2 * MARGIN) fail("run reaches the margin", k); }
        u64 L = j - i;
        if (!T->cnt[L]) T->first[L] = k;
        T->cnt[L]++;
        if (L >= 10) {
            if (T->nr == T->cap) { T->cap = T->cap ? 2 * T->cap : 256; T->runs = realloc(T->runs, T->cap * sizeof(run)); }
            T->runs[T->nr++] = (run){k, L};
        }
    }
}

static void *worker(void *arg) {
    tstate *T = arg;
    for (;;) {
        pthread_mutex_lock(&mu);
        u64 s0 = next_seg; next_seg += CHUNK;
        pthread_mutex_unlock(&mu);
        if (s0 >= nseg) break;
        u64 s1 = s0 + CHUNK < nseg ? s0 + CHUNK : nseg;
        for (u64 s = s0; s < s1; s++) {
            u64 A = LO + s * S, B = A + S < HI ? A + S : HI;
            do_segment(T, A, B, s == s0);
        }
    }
    return NULL;
}

static int cmpr(const void *a, const void *b) {
    const run *x = a, *y = b;
    return x->start < y->start ? -1 : x->start > y->start;
}

int main(int argc, char **argv) {
    if (argc < 4) { fprintf(stderr, "usage: sieve LO HI THREADS [SEGBITS]\n"); return 1; }
    LO = strtoull(argv[1], 0, 10); HI = strtoull(argv[2], 0, 10); NT = atoi(argv[3]);
    if (argc > 4) SEGBITS = atoi(argv[4]);
    if (getenv("DUMP")) DUMP = 1;
    if (LO < 1) LO = 1;
    S = (u64)1 << SEGBITS; CHUNK = 64;
    nseg = (HI - LO + S - 1) / S;
    u64 top = HI + 3 * MARGIN + S, sq = 1;
    while ((sq + 1) * (sq + 1) <= top) sq++;
    char *comp = calloc(sq + 2, 1);
    for (u64 i = 2; i * i <= sq; i++) if (!comp[i]) for (u64 j = i * i; j <= sq; j += i) comp[j] = 1;
    pr = malloc((sq / 2 + 16) * sizeof(u64)); npr = 0;
    for (u64 i = 2; i <= sq; i++) if (!comp[i]) pr[npr++] = i;
    free(comp);
    pinv = malloc(npr * sizeof(u64)); plim = malloc(npr * sizeof(u64));
    for (long j = 0; j < npr; j++) {
        u64 q = pr[j], inv = q;
        for (int k = 0; k < 6; k++) inv *= 2 - q * inv;
        pinv[j] = inv; plim[j] = UINT64_MAX / q;
    }
    tstate *TS = calloc(NT, sizeof(tstate));
    u64 alen = S + 6 * MARGIN;
    for (int t = 0; t < NT; t++) {
        TS[t].rem = malloc(alen * sizeof(u64)); TS[t].lpf = malloc(alen * sizeof(uint32_t));
        TS[t].mem = malloc(alen); TS[t].nm = malloc(npr * sizeof(u64)); TS[t].cl = malloc(alen * sizeof(long));
    }
    pthread_t th[256];
    for (int t = 0; t < NT; t++) pthread_create(&th[t], 0, worker, &TS[t]);
    for (int t = 0; t < NT; t++) pthread_join(th[t], 0);

    u64 cands = 0, first[MAXLEN + 1] = {0}, cnt[MAXLEN + 1] = {0}; long nr = 0;
    for (int t = 0; t < NT; t++) {
        cands += TS[t].cands; nr += TS[t].nr;
        for (int L = 1; L <= MAXLEN; L++) if (TS[t].cnt[L]) {
            if (!cnt[L] || TS[t].first[L] < first[L]) first[L] = TS[t].first[L];
            cnt[L] += TS[t].cnt[L];
        }
    }
    run *R = malloc((nr + 1) * sizeof(run)); long k = 0;
    for (int t = 0; t < NT; t++) { memcpy(R + k, TS[t].runs, TS[t].nr * sizeof(run)); k += TS[t].nr; }
    qsort(R, nr, sizeof(run), cmpr);
    printf("RANGE %llu %llu\n", (unsigned long long)LO, (unsigned long long)HI);
    printf("CANDIDATES %llu\n", (unsigned long long)cands);
    for (long i = 0; i < nr; i++)
        printf("RUN %llu %llu len %llu\n", (unsigned long long)R[i].start,
               (unsigned long long)(R[i].start + R[i].len - 1), (unsigned long long)R[i].len);
    for (int L = 1; L <= MAXLEN; L++) if (cnt[L])
        printf("FIRST len %d start %llu count %llu\n", L, (unsigned long long)first[L], (unsigned long long)cnt[L]);
    return 0;
}
