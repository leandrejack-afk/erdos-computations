#!/bin/zsh
# run_sieve_chunks.sh: resumable sieve run over (5e11, 2e13] in chunks of 2.5e11.
# Each chunk writes logs/sieve-chunks/<LO>.txt; a chunk whose log already ends with a
# FIRST line is complete and is skipped, so the script can be restarted after a crash.
# usage: ./run_sieve_chunks.sh [THREADS]
cd "${0:A:h}" || exit 1
T=${1:-8}
START=500000000001
END=20000000000001
STEP=250000000000
lo=$START
while (( lo < END )); do
  hi=$(( lo + STEP < END ? lo + STEP : END ))
  f=logs/sieve-chunks/$lo.txt
  if [[ -s $f ]] && tail -1 "$f" | grep -q '^FIRST'; then lo=$hi; continue; fi
  t0=$(date +%s)
  if ./sieve $lo $hi $T 18 > "$f.part" 2> "$f.err"; then
    mv "$f.part" "$f"; rm -f "$f.err"
    echo "$(date '+%F %T') chunk $lo $hi done in $(( $(date +%s) - t0 ))s" >> logs/sieve-chunks/progress.log
  else
    echo "$(date '+%F %T') chunk $lo $hi FAILED, see $f.err" >> logs/sieve-chunks/progress.log
    exit 1
  fi
  lo=$hi
done
echo "$(date '+%F %T') all chunks done" >> logs/sieve-chunks/progress.log
