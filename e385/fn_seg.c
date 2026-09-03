// Segmented version: reports n in [L,R] with F(n) <= n, where F(n) = max_{m<n composite} m + lpf(m).
// Only composites m >= n - sqrt(n) - 1 can have m + lpf(m) >= n, so sieving from L - W with W >= sqrt(R)+2 suffices.
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
int main(int argc, char **argv) {
    uint64_t L = strtoull(argv[1], 0, 10), R = strtoull(argv[2], 0, 10);
    uint64_t W = (uint64_t)sqrt((double)R) + 1000;
    uint64_t lo = L > W ? L - W : 2;
    uint64_t sq = (uint64_t)sqrt((double)R) + 2;
    // primes up to sq
    uint8_t *comp = calloc(sq + 1, 1);
    uint32_t *primes = malloc(sizeof(uint32_t) * (sq / 2 + 100)); size_t np = 0;
    for (uint64_t i = 2; i <= sq; i++) { if (comp[i]) continue; primes[np++] = (uint32_t)i; for (uint64_t j = i * i; j <= sq; j += i) comp[j] = 1; }
    const uint64_t S = 1u << 26; // segment length
    uint32_t *lpf = malloc(sizeof(uint32_t) * S);
    uint64_t F = 0, count = 0, last = 0;
    for (uint64_t base = lo; base <= R; base += S) {
        uint64_t hi = base + S - 1; if (hi > R) hi = R;
        uint64_t len = hi - base + 1;
        memset(lpf, 0, sizeof(uint32_t) * len);
        for (size_t k = 0; k < np; k++) {
            uint64_t p = primes[k]; if (p * p > hi) break;
            uint64_t start = ((base + p - 1) / p) * p; if (start < p * p) start = p * p;
            for (uint64_t m = start; m <= hi; m += p) if (!lpf[m - base]) lpf[m - base] = (uint32_t)p;
        }
        for (uint64_t m = base; m <= hi; m++) {
            uint64_t n = m + 1;               // test n = m+1 after including composite m
            if (m >= 4 && lpf[m - base]) { uint64_t v = m + lpf[m - base]; if (v > F) F = v; }
            if (n >= L && n <= R && F <= n) { count++; last = n; printf("n=%llu F(n)=%llu\n", (unsigned long long)n, (unsigned long long)F); fflush(stdout); }
        }
    }
    printf("range [%llu,%llu] exceptions=%llu last=%llu\n", (unsigned long long)L, (unsigned long long)R, (unsigned long long)count, (unsigned long long)last);
    return 0;
}
