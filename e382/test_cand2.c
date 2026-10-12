/* test_cand2.c: unit checks of cand2.c's primality test, factoring and smoothness test.
   build: clang -O2 -o test_cand2 test_cand2.c -lpthread */
#define main cand2_main
#include "cand2.c"
#undef main

static ull mulmod_t(ull a, ull b, ull n) { return (ull)((uint128)a * b % n); }
static ull powmod_t(ull a, ull e, ull n) { ull r = 1; a %= n; while (e) { if (e & 1) r = mulmod_t(r, a, n); a = mulmod_t(a, a, n); e >>= 1; } return r; }
/* reference: Miller-Rabin with the first 12 primes, exact below 3.18e23 */
static int mr_ref(ull n) {
    static const ull B[] = {2,3,5,7,11,13,17,19,23,29,31,37};
    if (n < 2) return 0;
    for (int i = 0; i < 12; i++) { if (n % B[i] == 0) return n == B[i]; }
    ull d = n - 1; int s = 0; while (!(d & 1)) { d >>= 1; s++; }
    for (int i = 0; i < 12; i++) {
        ull x = powmod_t(B[i], d, n);
        if (x == 1 || x == n - 1) continue;
        int ok = 0; for (int r = 1; r < s; r++) { x = mulmod_t(x, x, n); if (x == n - 1) { ok = 1; break; } }
        if (!ok) return 0;
    }
    return 1;
}
static ull rng = 88172645463325252ULL;
static ull xr(void) { rng ^= rng << 13; rng ^= rng >> 7; rng ^= rng << 17; return rng; }
static int has_small(ull n) { for (ull q = 2; q < 50; q++) if (n % q == 0) return 1; return 0; }
/* reference smoothness by full trial division (small k only) */
static int smooth_ref(ull k, ull p) { for (ull q = 2; q * q <= k; q++) while (k % q == 0) { if (q > p) return 0; k /= q; } return k <= p; }

int main(void) {
    long bad = 0, tested = 0;
    /* 1. bpsw against a sieve on [3, 3e7] for n with no prime factor below 50 */
    ull N = 30000000; char *c = calloc(N + 1, 1);
    for (ull i = 2; i * i <= N; i++) if (!c[i]) for (ull j = i * i; j <= N; j += i) c[j] = 1;
    for (ull n = 53; n <= N; n += 2) { if (has_small(n)) continue; tested++; if (bpsw(n) != !c[n]) { bad++; if (bad < 5) printf("bpsw wrong at %llu\n", n); } }
    printf("1. bpsw vs sieve to %llu: %ld tested, %ld wrong\n", N, tested, bad);
    /* 2. random odd n up to 2^62 with no small factor, against Miller-Rabin with 12 bases */
    bad = 0; tested = 0; long primes = 0;
    for (long t = 0; t < 3000000; t++) {
        int bitsz = 21 + (int)(xr() % 42);
        ull n = (xr() >> (64 - bitsz)) | 1 | (1ULL << (bitsz - 1));
        if (has_small(n)) continue;
        tested++; int a = bpsw(n), b = mr_ref(n); primes += b;
        if (a != b) { bad++; if (bad < 5) printf("bpsw/mr disagree at %llu\n", n); }
    }
    printf("2. bpsw vs MR on random n < 2^62: %ld tested (%ld prime), %ld disagree\n", tested, primes, bad);
    /* 3. composites that fool weaker tests must be rejected: strong pseudoprimes to base 2,
       strong Lucas pseudoprimes, and other Fermat pseudoprimes and Carmichael numbers. bpsw()
       is only ever called on numbers with no prime factor below 50, so only those are used. */
    static const ull psp[] = {3215031751ULL, 2152302898747ULL, 3474749660383ULL, 341550071728321ULL,
        3825123056546413051ULL, 5459, 5777, 10877, 16109, 18971, 22499,
        24569, 25199, 40309, 58519, 75077, 97439, 100127, 113573, 115639, 130139, 155819, 158399,
        161027, 162133, 176399, 176471, 189419, 192509, 197801, 224369, 230691, 231703, 243629,
        253259, 268349, 288919, 313499, 324899, 1373653, 25326001, 4759123141ULL, 1122004669633ULL,
        2047, 3277, 4033, 4681, 8321, 15841, 29341, 42799, 49141, 52633, 65281, 74665, 80581,
        85489, 88357, 90751, 104653, 130561, 196093, 220729, 233017, 252601, 253241, 256999,
        271951, 280601, 314821, 357761, 390937, 458989, 476971, 486737, 1050985, 1078003};
    long rej = 0, cnt = 0;
    for (unsigned i = 0; i < sizeof psp / sizeof psp[0]; i++) {
        ull n = psp[i]; if (n < 53 || has_small(n)) continue;
        if (mr_ref(n)) continue;           /* skip anything prime */
        cnt++; if (!bpsw(n)) rej++; else printf("pseudoprime %llu passed bpsw\n", n);
    }
    printf("3. composites that fool weaker tests, rejected: %ld of %ld\n", rej, cnt);
    /* 4. rho_floyd on random semiprimes of two primes above 1000 */
    bad = 0; tested = 0;
    for (long t = 0; t < 200000; t++) {
        ull a, b;
        do a = 1001 + xr() % 30000000; while (!mr_ref(a));
        do b = 1001 + xr() % 30000000; while (!mr_ref(b));
        ull n = a * b, f = rho_floyd(n); tested++;
        if (!(f == a || f == b)) { bad++; if (bad < 5) printf("rho wrong %llu = %llu * %llu got %llu\n", n, a, b, f); }
    }
    printf("4. rho_floyd on semiprimes: %ld tested, %ld wrong\n", tested, bad);
    /* 5. all_factors_le against full trial division for k < 10^12 and assorted p */
    X = 1000000000000ULL; MINLEN = 1; NTHREADS = 1;
    ull lim = 1000000; char *cc = calloc(lim + 1, 1);
    plist = malloc(lim * sizeof(ull)); np = 0;
    for (ull i = 2; i <= lim; i++) { if (!cc[i]) { plist[np++] = i; for (ull j = i * i; j <= lim; j += i) cc[j] = 1; } }
    ntrial = 0; while (ntrial < np && plist[ntrial] <= TRIAL_MAX) ntrial++;
    bad = 0; tested = 0;
    for (long t = 0; t < 400000; t++) {
        ull k = 1 + xr() % 1000000000000ULL;
        ull p = plist[xr() % 78498];     /* a prime below 10^6 */
        if (t % 4 == 0) {                /* make k smooth-ish: product of primes <= p */
            k = 1; while (1) { ull q = plist[xr() % (np < 2000 ? np : 2000)]; if (q > p) continue; if (k > 1000000000000ULL / q) break; k *= q; }
        }
        tested++;
        if (all_factors_le(k, p) != smooth_ref(k, p)) { bad++; if (bad < 5) printf("smooth wrong k=%llu p=%llu\n", k, p); }
    }
    printf("5. all_factors_le vs trial division: %ld tested, %ld wrong\n", tested, bad);
    return 0;
}
