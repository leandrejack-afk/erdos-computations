#!/bin/zsh
# check_small.sh: the small-range checks quoted in README.md, written to logs/small_checks.txt.
#  - first run of each length 1..10 below 4928180500 from cand and cand2 (a(0)..a(9)), and cand
#    below 40533366300 for comparison with direct.c;
#  - members of A388654 up to 1014049 from sieve and cand2, compared with the b-file.
cd "${0:A:h}" || exit 1
out=logs/small_checks.txt
B=$(mktemp -d)
{
  echo "== ./cand 4928180500 1 2 (first start and count of maximal runs by length)"
  ./cand 4928180500 1 2 | grep -E '^(CANDIDATES|FIRST)'
  echo "== ./cand2 4928180500 1 2"
  ./cand2 4928180500 1 2 | grep -E '^(CANDIDATES|VIOLATIONS|FIRST)'
  echo "== ./cand 40533366300 1 2 (to compare with logs/direct_40533366300.txt)"
  ./cand 40533366300 1 2 | grep -E '^(CANDIDATES|RUN|FIRST)'
  awk '!/^#/ && NF == 2 {print $2}' oeis/b388654.txt > $B/b.txt
  DUMP=1 ./sieve 1 1014050 1 | awk '$1 == "M" {print $2}' > $B/s.txt
  DUMP=1014049 ./cand2 1100000 1 1 | awk '$1 == "M" {print $2}' > $B/c.txt
  echo "== members of A388654 up to 1014049: b-file $(wc -l < $B/b.txt | tr -d ' '), sieve $(wc -l < $B/s.txt | tr -d ' '), cand2 $(wc -l < $B/c.txt | tr -d ' ')"
  cmp $B/s.txt $B/b.txt && echo "sieve members identical to oeis/b388654.txt"
  cmp $B/c.txt $B/b.txt && echo "cand2 members identical to oeis/b388654.txt"
} > $out 2>&1
rm -rf "${B:?}"
cat $out
