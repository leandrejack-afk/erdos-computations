/* rv_clique.c : reviewer's own exact maximum clique for Erdős 1109.
 * Written independently of bbmc.c / solver2.c for the review.
 * Graph: a <= N, a == r (mod 4), a squarefree; a ~ b iff a+b squarefree.
 * usage: rv_clique r N [L0=0] [split=0|1]
 *   With L0 > 0 the incumbent starts at L0, so the answer is max(L0, omega);
 *   answer == L0 means "no clique of size L0+1 exists" (exhaustive).
 * Algorithm: Tomita-style MCQ. Vertex order = non-increasing degree, ties by
 * numeric value descending. Candidate set as bitset; greedy sequential
 * colouring bound; branch from the highest colour down.
 * split=1: before vertex branching, case-split on the residue pairs
 * {s, q-s} mod q for q = 9, 25 (a clique cannot meet both), pruned by the
 * colour bound. Both modes must agree.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
typedef uint64_t u64;
static int n, W;
static u64 *adj;
static int *label;          /* vertex index -> integer */
static int best;
static int *bestset, bestlen, *stack, top;
static long long nodes;
static unsigned char *sqf;

static inline int tst(const u64 *b, int i){ return (b[i>>6]>>(i&63))&1; }
static inline void clr(u64 *b, int i){ b[i>>6] &= ~((u64)1<<(i&63)); }
static inline void st(u64 *b, int i){ b[i>>6] |= (u64)1<<(i&63); }
static int popc(const u64 *b){ int c=0; for(int i=0;i<W;i++) c+=__builtin_popcountll(b[i]); return c; }

/* greedy colouring of P: fills order[] (vertices) and colour[] non-decreasing; returns count */
static int colour(const u64 *P, int *order, int *colour_of, u64 *tmpQ, u64 *tmpR){
    memcpy(tmpQ, P, 8*W);
    int m=0, c=0;
    while(1){
        int any=0; for(int i=0;i<W;i++) if(tmpQ[i]){any=1;break;}
        if(!any) break;
        c++;
        memcpy(tmpR, tmpQ, 8*W);
        for(int i=0;i<W;i++) while(tmpR[i]){
            int v = i*64 + __builtin_ctzll(tmpR[i]);
            clr(tmpR, v); clr(tmpQ, v);
            order[m]=v; colour_of[m]=c; m++;
            const u64 *av = adj + (size_t)v*W;
            for(int j=0;j<W;j++) tmpR[j] &= ~av[j];
        }
    }
    return m;
}

static u64 *pool;   /* per depth: Q, R, NP */
static int *obuf, *cbuf;

static void expand(u64 *P, int depth){
    nodes++;
    u64 *Q = pool + (size_t)depth*3*W, *R = Q+W, *NP = Q+2*W;
    int *order = obuf + (size_t)depth*n, *col = cbuf + (size_t)depth*n;
    int m = colour(P, order, col, Q, R);
    for(int i=m-1;i>=0;i--){
        if(top + col[i] <= best) return;
        int v = order[i];
        stack[top++] = v;
        const u64 *av = adj + (size_t)v*W;
        int any=0;
        for(int j=0;j<W;j++){ NP[j] = P[j] & av[j]; if(NP[j]) any=1; }
        if(!any){
            if(top > best){ best = top; bestlen = top; memcpy(bestset, stack, 4*top); }
        } else expand(NP, depth+1);
        top--;
        clr(P, v);
    }
}

/* residue-pair split */
static int nq=0, qs[2]={9,25};
static u64 *cls[2];
static int npairs, pq[32], ps[32];
static long long splitnodes;
static u64 *spool;
static void split(u64 *P, int idx, int sd){
    splitnodes++;
    u64 *Q = spool + (size_t)sd*3*W, *R = Q+W, *P1 = Q+2*W;
    int *order = obuf + (size_t)(n-1-sd)*n, *col = cbuf + (size_t)(n-1-sd)*n; /* borrow from the far end */
    int m = colour(P, order, col, Q, R);
    if(m==0 || col[m-1] <= best) return;   /* colour bound */
    if(idx >= npairs){ expand(P, 0); return; }
    int qi = pq[idx], s = ps[idx], q = qs[qi];
    const u64 *Cs = cls[qi] + (size_t)s*W, *Ct = cls[qi] + (size_t)(q-s)*W;
    int hs=0, ht=0;
    for(int i=0;i<W;i++){ if(P[i]&Cs[i]) hs=1; if(P[i]&Ct[i]) ht=1; }
    if(!hs || !ht){ split(P, idx+1, sd); return; }
    for(int i=0;i<W;i++) P1[i] = P[i] & ~Ct[i];
    split(P1, idx+1, sd+1);
    for(int i=0;i<W;i++) P1[i] = P[i] & ~Cs[i];
    split(P1, idx+1, sd+1);
}

static int cmpdeg_deg; static int *gdeg, *gval;
static int cmpv(const void *a, const void *b){
    int i=*(const int*)a, j=*(const int*)b;
    if(gdeg[i]!=gdeg[j]) return gdeg[j]-gdeg[i];
    return gval[j]-gval[i];
}

int main(int argc, char **argv){
    if(argc<3){ fprintf(stderr,"usage: rv_clique r N [L0] [split]\n"); return 2; }
    int r=atoi(argv[1]), N=atoi(argv[2]), L0 = argc>3?atoi(argv[3]):0, dosplit = argc>4?atoi(argv[4]):0;
    sqf = malloc(2*N+3); memset(sqf,1,2*N+3); sqf[0]=0;
    for(long p=2;p*p<=2*N+2;p++) for(long m=p*p;m<=2*N+2;m+=p*p) sqf[m]=0;
    int *vals = malloc(4*(N+1)); int cnt=0;
    for(int a=1;a<=N;a++) if(a%4==r && sqf[a]) vals[cnt++]=a;
    n=cnt; W=(n+63)/64; if(W<1) W=1;
    /* degrees */
    int *deg = calloc(n,4);
    for(int i=0;i<n;i++) for(int j=i+1;j<n;j++) if(sqf[vals[i]+vals[j]]){ deg[i]++; deg[j]++; }
    int *perm = malloc(4*n); for(int i=0;i<n;i++) perm[i]=i;
    gdeg=deg; gval=vals; qsort(perm, n, 4, cmpv);
    label = malloc(4*n); for(int k=0;k<n;k++) label[k]=vals[perm[k]];
    adj = calloc((size_t)n*W, 8);
    long edges=0;
    for(int i=0;i<n;i++) for(int j=0;j<n;j++) if(i!=j && sqf[label[i]+label[j]]){ st(adj+(size_t)i*W, j); edges++; }
    edges/=2;
    pool = malloc(8*(size_t)(n+2)*3*W); spool = malloc(8*(size_t)64*3*W);
    obuf = malloc(4*(size_t)(n+2)*n); cbuf = malloc(4*(size_t)(n+2)*n);
    bestset = malloc(4*(n+1)); stack = malloc(4*(n+1));
    best = L0; bestlen=0; top=0; nodes=0; splitnodes=0;
    u64 *P = calloc(W,8); for(int i=0;i<n;i++) st(P,i);
    clock_t t0=clock();
    if(dosplit){
        nq=2; npairs=0;
        for(int qi=0;qi<2;qi++){ int q=qs[qi]; cls[qi]=calloc((size_t)q*W,8);
            for(int v=0;v<n;v++) st(cls[qi]+(size_t)(label[v]%q)*W, v);
            for(int s=1;s<q-s;s++){ pq[npairs]=qi; ps[npairs]=s; npairs++; } }
        split(P, 0, 0);
    } else expand(P, 0);
    double secs=(double)(clock()-t0)/CLOCKS_PER_SEC;
    printf("RV r=%d N=%d n=%d edges=%ld density=%.3f L0=%d answer=%d nodes=%lld splitnodes=%lld secs=%.2f\n",
           r, N, n, edges, n>1? (double)edges/((double)n*(n-1)/2):0.0, L0, best, nodes, splitnodes, secs);
    if(bestlen>0){
        int *w=malloc(4*bestlen); for(int i=0;i<bestlen;i++) w[i]=label[bestset[i]];
        for(int i=0;i<bestlen;i++) for(int j=i+1;j<bestlen;j++) if(w[j]<w[i]){int t=w[i];w[i]=w[j];w[j]=t;}
        printf("WITNESS:"); for(int i=0;i<bestlen;i++) printf(" %d", w[i]); printf("\n");
        /* self-check the witness */
        int ok=1; for(int i=0;i<bestlen;i++) for(int j=i;j<bestlen;j++) if(!sqf[w[i]+w[j]]) ok=0;
        for(int i=0;i<bestlen;i++) if(w[i]<1||w[i]>N||w[i]%4!=r) ok=0;
        printf("WITNESS_SELFCHECK=%s\n", ok?"ok":"BAD");
    }
    printf("END\n");
    return 0;
}
