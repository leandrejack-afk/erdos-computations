/* smin.c: exact S(k) = min(a_1 + ... + a_k) over admissible k-tuples 0 <= a_1 < ... < a_k
 * (Erdős problem 1204, B(k) = S(k)/k). Branch and bound.
 *
 * Reduction. Translating a tuple down by a_1 keeps it admissible and lowers the sum, so a_1 = 0.
 * Missing a class mod 2 then forces every a_i to be even: a_i = 2 b_i. Multiplication by 2 permutes
 * the residues mod every odd prime, so (b_i) must miss a class mod every odd prime p <= k (a k-tuple
 * cannot cover all classes of a prime p > k). Hence S(k) = 2 * min sum b_i, and the code works with b.
 *
 * Search. Depth-first over increasing b_{i+1} > b_i, keeping for each odd prime p <= k the set of
 * covered residues. A prime with p-1 covered residues is critical: its missing class is forbidden.
 * With -s NS the missed class of the first NS odd primes is chosen at the top level (pre-sieve), which
 * makes LB1 tight from the root. Every admissible tuple appears in at least one choice; duplicates are
 * removed when optimal tuples are recorded.
 *
 * Bounds at a node with m elements still to place after b_i, where f_1 < f_2 < ... are the
 * non-forbidden integers above b_i and Sb(m) = S(m)/2 was computed earlier in the same run:
 *   LB1  f_1 + ... + f_m           (the remaining elements are distinct non-forbidden integers)
 *   LB2  m * f_1 + Sb(m)           (the remaining elements form an admissible m-tuple on their own)
 *   -A   b_{i+j} >= Ab(i+j) and b_{i+j} >= b_{i+1} + Ab(j), Ab = A008407/2, optional and external.
 * A branch is cut when a bound exceeds the incumbent; ties are kept so every optimal tuple is found.
 * The cap b_k <= best - Sb(k-1) (the first k-1 elements already sum to at least Sb(k-1)) limits x.
 * The incumbent is the better of the greedy tuple and greedy extensions of the previous optima;
 * -slack D loosens it by D (b-space) as a check on the pruning.
 *
 * Single-k mode (parallel double-checking of large k): -k K0 starts at K0 instead of 1 and needs
 * -S table (lines "m S(m)" of exact values, m = 1..mk contiguous) plus -A; for mk < m < K0 the
 * lower bound Sb(m) >= Sb(mk) + Ab(mk+1) + ... + Ab(m) replaces the exact value (the first mk
 * elements form an admissible mk-tuple and b_j >= Ab(j)). -inc file supplies an admissible tuple
 * (a-space, one line) as the starting incumbent; it is re-checked before use.
 *
 * Usage: smin KMAX [-s NS] [-A b008407.txt] [-slack D] [-q] [-k K0 -S table [-inc tuple]]
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#define MAXK 500
#define MAXP 100
#define MAXX 8192
#define MAXOPT 64

static int K, P;
static int primes[MAXP];
static uint8_t covbit[MAXP][MAXK + 1];  /* covbit[j][r] = residue r mod primes[j] is covered */
static int cnt[MAXP];
static int forb[MAXX + 2];
static int xlim;
static int res[MAXX + 2][MAXP];

static long best;
static int tuple[MAXK + 2];
static long long nodes;
static long Sb[MAXK + 2];
static int Ab[MAXK + 2];
static int useA = 0, NS = 0, quiet = 0;
static long slackopt = 0;
static int K0 = 1, mk = 0;
static long Stab[MAXK + 2];
static int inc_tuple[MAXK + 2], inc_len = 0;
static int nopt;
static int opt[MAXOPT][MAXK + 2];
static long long ndup;
static int fbuf[MAXK + 2][MAXX + 2];
static uint8_t chlog[MAXK + 2][MAXP];   /* per depth: 0 untouched, 1 residue added, 2 added and became critical */

static void set_forb(int j, int r, int d) {
    int p = primes[j];
    for (int x = r; x <= xlim; x += p) forb[x] += d;
}

static int missing_residue(int j) {
    int p = primes[j];
    for (int t = 0; t < p; t++) if (!covbit[j][t]) return t;
    return -1;
}

static void add_elem(int x, uint8_t *log) {
    for (int j = 0; j < P; j++) {
        int r = res[x][j];
        if (covbit[j][r]) { log[j] = 0; continue; }
        covbit[j][r] = 1;
        cnt[j]++;
        log[j] = 1;
        if (cnt[j] == primes[j] - 1) {
            log[j] = 2;
            set_forb(j, missing_residue(j), +1);
        }
    }
}

static void remove_elem(int x, const uint8_t *log) {
    for (int j = 0; j < P; j++) {
        if (!log[j]) continue;
        if (log[j] == 2) set_forb(j, missing_residue(j), -1);
        covbit[j][res[x][j]] = 0;
        cnt[j]--;
    }
}

static void reset_state(void) {
    for (int j = 0; j < P; j++) { memset(covbit[j], 0, (size_t)primes[j]); cnt[j] = 0; }
    memset(forb, 0, sizeof(forb));
}

static void record(long sum) {
    if (sum < best) { best = sum; nopt = 0; }
    if (sum > best) return;
    for (int t = 0; t < nopt && t < MAXOPT; t++)
        if (!memcmp(opt[t], tuple, (size_t)K * sizeof(int))) { ndup++; return; }
    if (nopt < MAXOPT) memcpy(opt[nopt], tuple, (size_t)K * sizeof(int));
    nopt++;
}

static void dfs(int i, long sum) {
    nodes++;
    int m = K - i;
    if (m == 0) { record(sum); return; }
    int last = tuple[i - 1];
    int *f = fbuf[i];
    long slack = best - sum;
    if (slack < 0) return;
    long win = 0;
    int n = 0, broke = 0;
    for (int x = last + 1; x <= xlim; x++) {
        if (forb[x]) continue;
        f[n++] = x;
        win += x;
        if (n > m) win -= f[n - 1 - m];
        if (n >= m && win > slack) { broke = 1; break; }
    }
    if (n < m) return;
    int tmax = broke ? n - m - 1 : n - m;
    long cap2 = (slack - Sb[m]) / m;   /* LB2: sum + m*x + Sb(m) <= best */
    for (int t = 0; t <= tmax; t++) {
        int x = f[t];
        if (x > cap2) break;
        if (useA) {
            long lb = sum + x;
            for (int j = 1; j < m; j++) {
                int v = f[t + j];
                if (Ab[i + 1 + j] > v) v = Ab[i + 1 + j];
                if (x + Ab[j + 1] > v) v = x + Ab[j + 1];
                lb += v;
            }
            if (lb > best) continue;
        }
        add_elem(x, chlog[i]);
        tuple[i] = x;
        dfs(i + 1, sum + x);
        remove_elem(x, chlog[i]);
    }
}

/* Greedy completion of tuple[0..len-1] (already added to the state) to K elements; returns the sum
 * or -1 if the cap xlim is hit. Leaves the state as it found it. */
static long greedy_complete(int len, long sum, int *out) {
    static uint8_t glog[MAXK + 2][MAXP];
    int last = tuple[len - 1], n = len;
    for (int i = 0; i < len; i++) out[i] = tuple[i];
    while (n < K) {
        int x = last + 1;
        while (x <= xlim && forb[x]) x++;
        if (x > xlim) { sum = -1; break; }
        add_elem(x, glog[n]);
        out[n] = x; sum += x; last = x; n++;
    }
    for (int i = n - 1; i >= len; i--) remove_elem(out[i], glog[i]);
    return sum;
}

int main(int argc, char **argv) {
    if (argc < 2) { fprintf(stderr, "usage: smin KMAX [-s NS] [-A b008407.txt] [-slack D] [-q]\n"); return 1; }
    int kmax = atoi(argv[1]);
    if (kmax < 1 || kmax > MAXK) { fprintf(stderr, "KMAX out of range\n"); return 1; }
    for (int a = 2; a < argc; a++) {
        if (!strcmp(argv[a], "-s") && a + 1 < argc) NS = atoi(argv[++a]);
        else if (!strcmp(argv[a], "-slack") && a + 1 < argc) slackopt = atol(argv[++a]);
        else if (!strcmp(argv[a], "-A") && a + 1 < argc) {
            FILE *fp = fopen(argv[++a], "r");
            if (!fp) { fprintf(stderr, "cannot open %s\n", argv[a]); return 1; }
            int n; long v;
            while (fscanf(fp, "%d %ld", &n, &v) == 2) if (n >= 1 && n <= MAXK) Ab[n] = (int)(v / 2);
            fclose(fp);
            useA = 1;
        } else if (!strcmp(argv[a], "-q")) quiet = 1;
        else if (!strcmp(argv[a], "-k") && a + 1 < argc) K0 = atoi(argv[++a]);
        else if (!strcmp(argv[a], "-S") && a + 1 < argc) {
            FILE *fp = fopen(argv[++a], "r");
            if (!fp) { fprintf(stderr, "cannot open %s\n", argv[a]); return 1; }
            int n; long v;
            while (fscanf(fp, "%d %ld", &n, &v) == 2) if (n >= 1 && n <= MAXK && n == mk + 1) { Stab[n] = v; mk = n; }
            fclose(fp);
        } else if (!strcmp(argv[a], "-inc") && a + 1 < argc) {
            FILE *fp = fopen(argv[++a], "r");
            if (!fp) { fprintf(stderr, "cannot open %s\n", argv[a]); return 1; }
            long v;
            while (inc_len < MAXK && fscanf(fp, "%ld", &v) == 1) inc_tuple[inc_len++] = (int)v;
            fclose(fp);
        }
        else { fprintf(stderr, "bad argument %s\n", argv[a]); return 1; }
    }
    if (K0 > 1) {
        if (mk < 1 || !useA) { fprintf(stderr, "-k needs -S and -A\n"); return 1; }
        for (int m = 1; m <= mk; m++) Sb[m] = Stab[m] / 2;
        for (int m = mk + 1; m < K0; m++) Sb[m] = Sb[m - 1] + Ab[m];
        if (mk >= K0) fprintf(stderr, "note: table already covers K0=%d, the run re-derives it\n", K0);
    }
    if (useA) for (int n = 2; n <= kmax; n++) if (Ab[n] == 0) { fprintf(stderr, "A008407 missing n=%d\n", n); return 1; }

    static int prevopt[MAXOPT][MAXK + 2];
    static int greedyt[MAXK + 2], cand[MAXK + 2];
    static uint8_t plog[MAXK + 2][MAXP];
    int nprev = 0;
    Sb[0] = 0;
    for (K = K0; K <= kmax; K++) {
        clock_t t0 = clock();
        P = 0;
        for (int p = 3; p <= K; p += 2) {
            int isp = 1;
            for (int d = 3; d * d <= p; d += 2) if (p % d == 0) { isp = 0; break; }
            if (isp) primes[P++] = p;
        }
        if (P > MAXP) { fprintf(stderr, "too many primes\n"); return 1; }
        xlim = MAXX;
        for (int x = 0; x <= xlim; x++) for (int j = 0; j < P; j++) res[x][j] = x % primes[j];
        /* incumbent: greedy tuple, and greedy extensions of the previous optima */
        reset_state();
        add_elem(0, plog[0]);
        tuple[0] = 0;
        long greedy_sum = greedy_complete(1, 0, greedyt);
        if (greedy_sum < 0) { fprintf(stderr, "greedy exceeds MAXX at K=%d\n", K); return 1; }
        long inc_sum = greedy_sum;
        for (int t = 0; t < nprev; t++) {
            reset_state();
            long s = 0;
            for (int i = 0; i < K - 1; i++) { tuple[i] = prevopt[t][i]; add_elem(tuple[i], plog[i]); s += tuple[i]; }
            long v = greedy_complete(K - 1, s, cand);
            if (v >= 0 && v < inc_sum) inc_sum = v;
        }
        if (K == K0 && inc_len > 0) {
            /* external incumbent: must be K increasing even integers starting at 0 and admissible */
            if (inc_len != K || inc_tuple[0] != 0) { fprintf(stderr, "bad incumbent length or start\n"); return 1; }
            reset_state();
            long s = 0;
            for (int i = 0; i < K; i++) {
                if (inc_tuple[i] % 2 || (i > 0 && inc_tuple[i] <= inc_tuple[i - 1])) { fprintf(stderr, "bad incumbent element\n"); return 1; }
                int x = inc_tuple[i] / 2;
                if (x > xlim || forb[x]) { fprintf(stderr, "incumbent is not admissible at element %d\n", i); return 1; }
                add_elem(x, plog[i]);
                s += x;
            }
            if (s < inc_sum) inc_sum = s;
        }
        best = inc_sum + slackopt;
        nopt = 0; ndup = 0; nodes = 0;
        long cap = (K >= 2) ? best - Sb[K - 1] : 0;
        xlim = cap < MAXX ? (int)cap : MAXX;
        if (xlim < 1) xlim = 1;
        int ns = NS < P ? NS : P;
        int r[MAXP];
        for (int j = 0; j < ns; j++) r[j] = 1;
        for (;;) {
            reset_state();
            for (int j = 0; j < ns; j++) {
                int p = primes[j];
                for (int t = 0; t < p; t++) covbit[j][t] = (t != r[j]);
                cnt[j] = p - 1;
                set_forb(j, r[j], +1);
            }
            add_elem(0, plog[0]);
            tuple[0] = 0;
            dfs(1, 0);
            int j = 0;
            while (j < ns) { if (++r[j] < primes[j]) break; r[j] = 1; j++; }
            if (j == ns) break;
        }
        if (nopt == 0) { fprintf(stderr, "internal error: no optimal tuple recorded for K=%d\n", K); return 2; }
        Sb[K] = best;
        int isgreedy = 0;
        for (int t = 0; t < nopt && t < MAXOPT; t++) if (!memcmp(opt[t], greedyt, (size_t)K * sizeof(int))) isgreedy = 1;
        double secs = (double)(clock() - t0) / CLOCKS_PER_SEC;
        printf("k=%d S=%ld greedy=%ld isgreedy=%d nopt=%d nodes=%lld xlim=%d secs=%.2f",
               K, 2 * best, 2 * greedy_sum, isgreedy, nopt, nodes, xlim, secs);
        if (useA) printf(" A=%d", 2 * Ab[K]);
        printf("\n");
        if (!quiet) {
            for (int t = 0; t < nopt && t < MAXOPT; t++) {
                printf("  tuple:");
                for (int i = 0; i < K; i++) printf(" %d", 2 * opt[t][i]);
                printf("\n");
            }
        }
        fflush(stdout);
        nprev = nopt < MAXOPT ? nopt : MAXOPT;
        for (int t = 0; t < nprev; t++) memcpy(prevopt[t], opt[t], sizeof(prevopt[t]));
    }
    return 0;
}
