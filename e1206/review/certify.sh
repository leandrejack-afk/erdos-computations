#!/bin/zsh
# For each n given: K = b(n) + 1 must be UNSAT (kissat, DRAT proof checked by drat-trim), and
# K = b(n) must be SAT with a model that check_model.py accepts from the definition.
# usage: certify.sh WORKDIR DRAT_TRIM n1 n2 ...   (one line per n on stdout)
W=$1; DT=$2; shift 2
HERE=${0:A:h}
B=$HERE/../b1206.txt
for n in "$@"; do
  b=$(awk -v n=$n '$1==n{print $2}' $B)
  K=$((b + 1))
  python3 $HERE/make_cnf.py $n $K $W/u$n.cnf
  t0=$(date +%s)
  kissat -q $W/u$n.cnf $W/u$n.drat > $W/u$n.out; ks=$?
  t1=$(date +%s)
  if [[ $ks == 20 ]]; then
    v=$($DT $W/u$n.cnf $W/u$n.drat -t 100000 2>&1 | tr '\r' '\n' | grep -E '^s ' | tr '\n' ' ')
  else
    v="(no proof check: kissat exit $ks)"
  fi
  t2=$(date +%s)
  python3 $HERE/make_cnf.py $n $b $W/s$n.cnf
  kissat -q $W/s$n.cnf > $W/s$n.out; ss=$?
  m=$(python3 $HERE/check_model.py $n $b $W/s$n.out)
  rm -f $W/u$n.drat $W/s$n.cnf
  echo "n=$n b=$b | K=$K kissat_exit=$ks drat-trim: $v solve=$((t1-t0))s check=$((t2-t1))s | K=$b kissat_exit=$ss $m"
done
