#!/bin/zsh
# Regression: reproduce A375071 for problem n = 2..23 (pairs share one run).
# Expected N = n + k - 1 for the pair (n even, n+1).
typeset -A EXPN
EXPN=(2 "6 6" 4 "210 210" 6 "2480 990" 8 "8178 8178" 10 "45153 45153" 12 "3648841 3648841" 14 "7979090 7979090" 16 "58068877 58068877" 18 "255278312 255278312" 20 "1019547844 1019547844" 22 "17609764994 17609764994")
for n in 2 4 6 8 10 12 14 16 18 20 22; do
  set -- ${=EXPN[$n]}
  N1=$1; N2=$2
  NHI=$(( (N1 > N2 ? N1 : N2) * 2 + 1000 ))
  out=$(./e389_sieve $n $n $NHI -t 10 -o surv_$n.txt 2>/dev/null)
  got1=$(echo "$out" | grep "RESULT n=$n " | sed -E 's/.*N=([0-9]+).*/\1/')
  got2=$(echo "$out" | grep "RESULT n=$((n+1)) " | sed -E 's/.*N=([0-9]+).*/\1/')
  t=$(echo "$out" | grep TIME)
  st="OK"; [[ "$got1" == "$N1" && "$got2" == "$N2" ]] || st="MISMATCH"
  echo "n=$n,$((n+1)): got N=$got1,$got2 expected $N1,$N2 $st ($t, survivors $(wc -l < surv_$n.txt))"
done
