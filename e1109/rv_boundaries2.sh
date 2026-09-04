#!/bin/zsh
S=/Users/leandrejack/projects/math-contributions/workspace/evidence
OUT=$S/rv_boundaries2.tsv
echo "k\tN\tr\tn\tomega\tnodes\tsplitnodes\tsecs\tverdict" > $OUT
A=(1103 1255 1277 1293 1335 1407 1509 1535 1595 1707 1735 1779 1835 1919 1991 1999 2071 2141 2193 2287 2355 2427 2455 2627 2729 2741 2993)
for i in {14..27}; do
  k=$((38+i))
  if [ $i -lt 27 ]; then N=$(( ${A[$((i+1))]} - 1 )); else N=3000; fi
  for r in 1 3; do
    line=$(/opt/homebrew/bin/gtimeout 1500 $S/rv_clique $r $N 0 1 | head -1)
    if [ -z "$line" ]; then echo "$k\t$N\t$r\t?\tTIMEOUT\t?\t?\t1500\tTIMEOUT" >> $OUT; continue; fi
    n=$(echo $line | sed -E 's/.* n=([0-9]+).*/\1/'); om=$(echo $line | sed -E 's/.* answer=([0-9]+).*/\1/')
    nd=$(echo $line | sed -E 's/.* nodes=([0-9]+).*/\1/'); sn=$(echo $line | sed -E 's/.* splitnodes=([0-9]+).*/\1/'); sc=$(echo $line | sed -E 's/.* secs=([0-9.]+).*/\1/')
    if [ "$om" -le "$k" ]; then v=LE_k; else v=EXCEEDS_k; fi
    echo "$k\t$N\t$r\t$n\t$om\t$nd\t$sn\t$sc\t$v" >> $OUT
  done
done
echo ALLDONE >> $OUT
