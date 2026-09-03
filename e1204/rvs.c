/* rvs.c: third computation of S(k) for Erdős problem 1204 by a search over missed residue classes.
 *
 * Same reduction as smin.c (a_1 = 0, a_i = 2 b_i, odd primes p <= k only), different search.
 * Fix for every odd prime p <= k a class r_p != 0 to miss. Any k integers that avoid all the
 * classes r_p form an admissible tuple, and every admissible tuple avoids some choice of classes,
 * so S(k)/2 = min over the choice of the sum of the k smallest survivors of the sieve (an optimal
 * tuple is exactly the k smallest survivors of its own sieve). Depth-first over the primes in
 * increasing order: at each node the sum of the k smallest survivors is a lower bound that only
 * grows with more sieving. A class with no survivor in [0, xlim] costs nothing now or later, so if
 * one exists the prime is settled without branching. Survivors above xlim = incumbent - Sb(k-1)
 * are never needed because the first k-1 elements sum to at least Sb(k-1) (Sb from this same run).
 * The incumbent is the greedy tuple. Ties are kept so every optimal tuple is found.
 *
 * Usage: rvs KMAX [-q]
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#define MAXK 400
#define MAXP 100
#define MAXX 8192
#define MAXOPT 64

static int K, P, xlim;
static int primes[MAXP];
static uint8_t dead[MAXX + 2];      /* dead[x] > 0: x is removed by at least one chosen class */
static long best;
static long Sb[MAXK + 2];
static long long nodes;
static int nopt;
static int opt[MAXOPT][MAXK + 2];
static int quiet = 0;

/* sum of the K smallest survivors; returns -1 if fewer than K survive; fills out if non-null */
static long ksum(int *out) {
    long s = 0;
    int n = 0;
    for (int x = 0; x <= xlim && n < K; x++) {
        if (dead[x]) continue;
        s += x;
        if (out) out[n] = x;
        n++;
    }
    return n < K ? -1 : s;
}

static void record(long sum) {
    static int t[MAXK + 2];
    ksum(t);
    if (sum < best) { best = sum; nopt = 0; }
    for (int i = 0; i < nopt && i < MAXOPT; i++)
        if (!memcmp(opt[i], t, (size_t)K * sizeof(int))) return;
    if (nopt < MAXOPT) memcpy(opt[nopt], t, (size_t)K * sizeof(int));
    nopt++;
}

static void kill_class(int p, int r, int d) {
    for (int x = r; x <= xlim; x += p) dead[x] += (uint8_t)d;
}

static void dfs(int j) {
    nodes++;
    long s = ksum(NULL);
    if (s < 0 || s > best) return;
    if (j == P) { record(s); return; }
    int p = primes[j];
    /* a class with no survivor in [0, xlim] settles this prime */
    for (int r = 1; r < p; r++) {
        int alive = 0;
        for (int x = r; x <= xlim; x += p) if (!dead[x]) { alive = 1; break; }
        if (!alive) { dfs(j + 1); return; }
    }
    /* otherwise branch on the classes, cheapest first */
    long cost[MAXK + 2];
    int order[MAXK + 2], nc = 0;
    for (int r = 1; r < p; r++) {
        kill_class(p, r, +1);
        long c = ksum(NULL);
        kill_class(p, r, -1);
        if (c < 0 || c > best) continue;
        cost[r] = c;
        order[nc++] = r;
    }
    for (int a = 1; a < nc; a++) {
        int r = order[a], b = a;
        while (b > 0 && cost[order[b - 1]] > cost[r]) { order[b] = order[b - 1]; b--; }
        order[b] = r;
    }
    for (int a = 0; a < nc; a++) {
        int r = order[a];
        if (cost[r] > best) break;
        kill_class(p, r, +1);
        dfs(j + 1);
        kill_class(p, r, -1);
    }
}

int main(int argc, char **argv) {
    if (argc < 2) { fprintf(stderr, "usage: rvs KMAX [-q]\n"); return 1; }
    int kmax = atoi(argv[1]);
    if (kmax < 1 || kmax > MAXK) { fprintf(stderr, "KMAX out of range\n"); return 1; }
    if (argc > 2 && !strcmp(argv[2], "-q")) quiet = 1;
    Sb[0] = 0;
    for (K = 1; K <= kmax; K++) {
        clock_t t0 = clock();
        P = 0;
        for (int p = 3; p <= K; p += 2) {
            int isp = 1;
            for (int d = 3; d * d <= p; d += 2) if (p % d == 0) { isp = 0; break; }
            if (isp) primes[P++] = p;
        }
        /* greedy incumbent in b-space: next element is the least x that completes no residue system */
        static int cnt[MAXP];
        static uint8_t cov[MAXP][MAXK + 2];
        for (int j = 0; j < P; j++) { memset(cov[j], 0, (size_t)primes[j]); cov[j][0] = 1; cnt[j] = 1; }
        long g = 0;
        int last = 0;
        for (int n = 1; n < K; n++) {
            int x = last;
            for (;;) {
                x++;
                int ok = 1;
                for (int j = 0; j < P && ok; j++) if (cnt[j] == primes[j] - 1 && !cov[j][x % primes[j]]) ok = 0;
                if (ok) break;
            }
            for (int j = 0; j < P; j++) if (!cov[j][x % primes[j]]) { cov[j][x % primes[j]] = 1; cnt[j]++; }
            g += x; last = x;
        }
        best = g;
        long cap = (K >= 2) ? best - Sb[K - 1] : 0;
        xlim = cap < MAXX ? (int)cap : MAXX;
        if (xlim < 1) xlim = 1;
        memset(dead, 0, sizeof(dead));
        nopt = 0; nodes = 0;
        dfs(0);
        if (nopt == 0) { fprintf(stderr, "internal error: nothing recorded for K=%d\n", K); return 2; }
        Sb[K] = best;
        double secs = (double)(clock() - t0) / CLOCKS_PER_SEC;
        printf("k=%d S=%ld greedy=%ld nopt=%d nodes=%lld xlim=%d secs=%.2f\n", K, 2 * best, 2 * g, nopt, nodes, xlim, secs);
        if (!quiet) for (int t = 0; t < nopt && t < MAXOPT; t++) {
            printf("  tuple:");
            for (int i = 0; i < K; i++) printf(" %d", 2 * opt[t][i]);
            printf("\n");
        }
        fflush(stdout);
    }
    return 0;
}
