#!/usr/bin/env python3
"""Re-prove every record boundary with a solver binary in full mode.

For consecutive records (k, a(k)) and (k+1, a(k+1)) the claim f(N) = k on
[a(k), a(k+1)-1] follows from the witness at a(k) (checked separately) plus
"no clique of size k+1 in the class-r graph at N = a(k+1)-1" for r = 1 and 3.
For the last record the proof is run at N_max.  The solver is called as
  solver full r N k splits
and must print "best=k" (meaning nothing larger than k exists).

usage: boundary_check.py records_file N_max solver splits kmin out.tsv [r]
Only boundaries with k >= kmin are run; with r given, only that class.
"""
import subprocess
import sys
import time


def main():
    recs = []
    with open(sys.argv[1]) as fh:
        for line in fh:
            parts = line.rstrip("\n").split("\t")
            recs.append((int(parts[0]), int(parts[1])))
    nmax = int(sys.argv[2])
    solver, splits, kmin, out = sys.argv[3], sys.argv[4], int(sys.argv[5]), sys.argv[6]
    classes = (int(sys.argv[7]),) if len(sys.argv) > 7 else (1, 3)
    fails = 0
    with open(out, "w") as fo:
        fo.write("k\tN\tr\tn\tbest\tnodes\tsecs\tverdict\n")
        for i, (k, a) in enumerate(recs):
            if k < kmin:
                continue
            N = recs[i + 1][1] - 1 if i + 1 < len(recs) else nmax
            for r in classes:
                t0 = time.time()
                res = subprocess.run([solver, "full", str(r), str(N), str(k), splits],
                                     capture_output=True, text=True)
                secs = time.time() - t0
                head = res.stdout.splitlines()[0] if res.stdout else "NO OUTPUT"
                fields = dict(p.split("=") for p in head.split()[1:] if "=" in p)
                best = int(fields.get("best", -1))
                nodes = fields.get("nodes", fields.get("visits", "?"))
                verdict = "OK" if best == k and "DONE" in res.stdout else "FAIL"
                if verdict == "FAIL":
                    fails += 1
                fo.write(f"{k}\t{N}\t{r}\t{fields.get('n', '?')}\t{best}\t{nodes}\t{secs:.1f}\t{verdict}\n")
                fo.flush()
    print(f"boundaries done, failures: {fails}")
    sys.exit(1 if fails else 0)


if __name__ == "__main__":
    main()
