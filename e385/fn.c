// Erdős problem 385 / 430: F(n) = max_{m<n composite} (m + lpf(m)).
// Lists every n <= N with F(n) <= n. Sieve of least prime factors.
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
int main(int argc, char **argv) {
    uint64_t N = argc > 1 ? strtoull(argv[1], 0, 10) : 100000000ULL;
    uint32_t *lpf = calloc(N + 1, sizeof(uint32_t));
    if (!lpf) { fprintf(stderr, "alloc failed\n"); return 1; }
    uint64_t r = (uint64_t)sqrt((double)N) + 1;
    for (uint64_t p = 2; p <= r; p++) {
        if (lpf[p]) continue;
        for (uint64_t m = p * p; m <= N; m += p) if (!lpf[m]) lpf[m] = (uint32_t)p;
    }
    // F(n) = max over composite m < n of m + lpf(m); running max.
    uint64_t F = 0, count = 0, lastbad = 0;
    for (uint64_t n = 2; n <= N; n++) {
        // include m = n-1 if composite
        uint64_t m = n - 1;
        if (m >= 4 && lpf[m]) { uint64_t v = m + lpf[m]; if (v > F) F = v; }
        if (F <= n) { count++; lastbad = n; if (count <= 200) printf("n=%llu F(n)=%llu\n", (unsigned long long)n, (unsigned long long)F); }
    }
    printf("N=%llu exceptions=%llu last_exception=%llu\n", (unsigned long long)N, (unsigned long long)count, (unsigned long long)lastbad);
    return 0;
}
