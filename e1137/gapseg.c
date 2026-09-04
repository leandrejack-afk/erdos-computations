#include <primesieve.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
/* Count every gap (prev,p) whose lower prime prev lies in [A,B], and every consecutive pair whose middle prime lies in [A,B].
   Iterate from A-5000 to B+5000 so both boundary gaps and the pair before A are available (all gaps here are < 5000). */
int main(int argc,char**argv){
  uint64_t A = strtoull(argv[1],0,10), B = strtoull(argv[2],0,10);
  uint64_t start = A>5000 ? A-5000 : 0, stop = B+5000;
  primesieve_iterator it; primesieve_init(&it); primesieve_jump_to(&it,start,stop);
  uint64_t prev=0, prevgap=0, maxgap=0, maxgap_p=0, maxprod=0, maxprod_p=0, g1=0,g2=0, p;
  while((p=primesieve_next_prime(&it))<=stop){
    if(prev){
      uint64_t gap=p-prev;
      if(prev>=A && prev<=B){
        if(gap>maxgap){maxgap=gap;maxgap_p=prev;}
        if(prevgap){ uint64_t prod=gap*prevgap; if(prod>maxprod){maxprod=prod;maxprod_p=prev;g1=prevgap;g2=gap;} }
      }
      prevgap=gap;
    }
    prev=p;
  }
  printf("SEG %" PRIu64 " %" PRIu64 " maxgap=%" PRIu64 " after=%" PRIu64 " maxprod=%" PRIu64 " g1=%" PRIu64 " g2=%" PRIu64 " mid=%" PRIu64 "\n",A,B,maxgap,maxgap_p,maxprod,g1,g2,maxprod_p);
  primesieve_free_iterator(&it);
  return 0;
}
