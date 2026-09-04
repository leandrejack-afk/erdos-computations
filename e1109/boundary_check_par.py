#!/usr/bin/env python3
"""Parallel companion to boundary_check.py: re-prove chosen record boundaries
with a solver binary in full mode, several instances at once.

Same claim and same row format as boundary_check.py.  For consecutive records
(k, a(k)) and (k+1, a(k+1)) the boundary proof for k is "no clique of size k+1
in the class-r graph at N = a(k+1)-1"; for the last record it is run at N_max.
The solver is called as  solver full r N k splits  and must print "best=k" and
"DONE".

usage: boundary_check_par.py records_file N_max solver splits out_prefix workers r:k1,k2,... [r:k1,k2,...]

Writes out_prefix_r<r>.tsv per class (rows sorted by k once every instance has
finished) and appends a line to out_prefix.progress as each instance ends.
"""
import subprocess
import sys
import time
from concurrent.futures import ThreadPoolExecutor, as_completed


def run_one(solver, splits, k, N, r):
    t0 = time.time()
    res = subprocess.run([solver, "full", str(r), str(N), str(k), splits],
                         capture_output=True, text=True)
    secs = time.time() - t0
    head = res.stdout.splitlines()[0] if res.stdout else "NO OUTPUT"
    fields = dict(p.split("=") for p in head.split()[1:] if "=" in p)
    best = int(fields.get("best", -1))
    nodes = fields.get("nodes", fields.get("visits", "?"))
    verdict = "OK" if best == k and "DONE" in res.stdout else "FAIL"
    return (k, N, r, fields.get("n", "?"), best, nodes, secs, verdict)


def main():
    recs = []
    with open(sys.argv[1]) as fh:
        for line in fh:
            parts = line.rstrip("\n").split("\t")
            recs.append((int(parts[0]), int(parts[1])))
    nmax = int(sys.argv[2])
    solver, splits, prefix, workers = sys.argv[3], sys.argv[4], sys.argv[5], int(sys.argv[6])
    nxt = {}
    for i, (k, a) in enumerate(recs):
        nxt[k] = recs[i + 1][1] - 1 if i + 1 < len(recs) else nmax
    jobs = []
    for spec in sys.argv[7:]:
        r, ks = spec.split(":")
        for k in ks.split(","):
            jobs.append((int(k), nxt[int(k)], int(r)))
    header = "k\tN\tr\tn\tbest\tnodes\tsecs\tverdict\n"
    rows = {}
    fails = 0
    with open(prefix + ".progress", "a") as prog, ThreadPoolExecutor(max_workers=workers) as pool:
        prog.write(f"start {time.ctime()} jobs={jobs} workers={workers}\n"); prog.flush()
        futs = {pool.submit(run_one, solver, splits, k, N, r): (k, r) for (k, N, r) in jobs}
        for fut in as_completed(futs):
            row = fut.result()
            rows.setdefault(row[2], []).append(row)
            if row[7] == "FAIL":
                fails += 1
            prog.write("\t".join(str(x) if not isinstance(x, float) else f"{x:.1f}" for x in row) + f"\t{time.ctime()}\n")
            prog.flush()
        prog.write(f"end {time.ctime()} failures={fails}\n")
    for r, rs in rows.items():
        with open(f"{prefix}_r{r}.tsv", "w") as fo:
            fo.write(header)
            for row in sorted(rs):
                fo.write("\t".join(str(x) if not isinstance(x, float) else f"{x:.1f}" for x in row) + "\n")
    print(f"boundaries done, failures: {fails}")
    sys.exit(1 if fails else 0)


if __name__ == "__main__":
    main()
