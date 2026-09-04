#include <primesieve.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
int main(int argc,char**argv){
  uint64_t X = strtoull(argv[1],0,10);
  primesieve_iterator it; primesieve_init(&it);
  uint64_t prev=0, prevgap=0, maxgap=0, maxgap_p=0, maxprod=0, maxprod_p=0, mp_g1=0, mp_g2=0;
  uint64_t nextpow=10; 
  uint64_t p;
  while((p=primesieve_next_prime(&it))<=X){
    while(p>nextpow){ /* report state for all primes < nextpow */
      printf("X=%" PRIu64 " maxgap=%" PRIu64 " (after %" PRIu64 ") maxprod=%" PRIu64 " (=%" PRIu64 "*%" PRIu64 " after %" PRIu64 ") ratio=%.6f\n",
        nextpow,maxgap,maxgap_p,maxprod,mp_g1,mp_g2,maxprod_p,(double)maxprod/((double)maxgap*(double)maxgap));
      fflush(stdout); nextpow*=10; }
    if(prev){
      uint64_t gap=p-prev;
      if(gap>maxgap){maxgap=gap;maxgap_p=prev; printf("RECORD GAP %" PRIu64 " after prime %" PRIu64 "\n",gap,prev);}
      if(prevgap){ uint64_t prod=gap*prevgap;
        if(prod>maxprod){maxprod=prod;maxprod_p=prev;mp_g1=prevgap;mp_g2=gap; printf("RECORD PROD %" PRIu64 " = %" PRIu64 "*%" PRIu64 " (middle prime %" PRIu64 ") ratio=%.6f\n",prod,prevgap,gap,prev,(double)prod/((double)maxgap*(double)maxgap));}
      }
      prevgap=gap;
    }
    prev=p;
  }
  printf("X=%" PRIu64 " maxgap=%" PRIu64 " (after %" PRIu64 ") maxprod=%" PRIu64 " (=%" PRIu64 "*%" PRIu64 " after %" PRIu64 ") ratio=%.6f\n",
        X,maxgap,maxgap_p,maxprod,mp_g1,mp_g2,maxprod_p,(double)maxprod/((double)maxgap*(double)maxgap));
  primesieve_free_iterator(&it);
  return 0;
}
