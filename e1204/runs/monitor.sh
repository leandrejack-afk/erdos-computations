#!/bin/sh
# refresh the cross-check record every 15 minutes while the searches run
cd /Users/leandrejack/projects/open-problems/targets/e1204 || exit 1
while true; do
  /Users/leandrejack/projects/open-problems/.venv/bin/python crosscheck.py --table-every 10 --bfile-max 147 > runs/crosscheck.txt 2>&1
  sleep 900
done
