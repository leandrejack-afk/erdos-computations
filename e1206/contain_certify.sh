#!/bin/zsh
# For each n given and the value v = c(n) from the c-file: "some Sidon subset of the first n cubes
# contains n^3 and has v + 1 elements" must be UNSAT (kissat, DRAT proof checked by drat-trim), and
# the same with v elements must be SAT with a model that check_model.py accepts from the definition
# and that contains n. The CNF is review/make_cnf.py's plus the unit clause "n is in A".
# usage: contain_certify.sh WORKDIR DRAT_TRIM C_FILE n1 n2 ...   (one line per n on stdout)
W=$1; DT=$2; CF=$3; shift 3
HERE=${0:A:h}
unit() { # append the unit clause [n] to a DIMACS file and fix its header
  local f=$1 n=$2
  awk -v n=$n 'NR==2 { $4 = $4 + 1 } { print } END { print n " 0" }' $f > $f.tmp && mv $f.tmp $f
}
for n in "$@"; do
  v=$(awk -v n=$n '$1==n{print $2}' $CF)
  K=$((v + 1))
  python3 $HERE/review/make_cnf.py $n $K $W/cu$n.cnf; unit $W/cu$n.cnf $n
  t0=$(date +%s)
  kissat -q $W/cu$n.cnf $W/cu$n.drat > $W/cu$n.out; ks=$?
  t1=$(date +%s)
  if [[ $ks == 20 ]]; then
    p=$($DT $W/cu$n.cnf $W/cu$n.drat -t 100000 2>&1 | tr '\r' '\n' | grep -E '^s ' | tr '\n' ' ')
  else
    p="(no proof check: kissat exit $ks)"
  fi
  t2=$(date +%s)
  python3 $HERE/review/make_cnf.py $n $v $W/cs$n.cnf; unit $W/cs$n.cnf $n
  kissat -q $W/cs$n.cnf > $W/cs$n.out; ss=$?
  m=$(python3 $HERE/review/check_model.py $n $v $W/cs$n.out)
  has=$(grep -E '^v ' $W/cs$n.out | tr ' ' '\n' | grep -qx "$n" && echo "contains n" || echo "MISSING n")
  rm -f $W/cu$n.drat $W/cs$n.cnf
  echo "n=$n c=$v | K=$K kissat_exit=$ks drat-trim: $p solve=$((t1-t0))s check=$((t2-t1))s | K=$v kissat_exit=$ss $m $has"
done
