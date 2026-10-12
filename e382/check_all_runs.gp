\\ check_all_runs.gp: checks every run of length >= 10 listed by cand.c below 2*10^14 (runs_2e14.gp,
\\ made from logs/cand_2e14_min10.txt by the awk line in README.md) from the definition alone:
\\ the members near each run are the union of the bad intervals of length <= 64, each tested
\\ literally, and the run through the listed start must end exactly at the listed end.
P(k) = if(k == 1, 1, vecmax(factor(k)[, 1]));
\\ definition-only: members near [L,R] = union of bad intervals of length <= B; return the run through L
chk(L, R, B) = {
  my(D = 40, lo = L - D - B, n = (R - L) + 2*D + 2*B + 1, PP = vector(n), mem = vector(n));
  for (i = 1, n, PP[i] = P(lo + i - 1));
  for (iu = 1, n - B,
    my(p = 1, ex = 0);
    for (iv = iu, iu + B - 1,
      my(q = PP[iv]);
      if (q > p, p = q; ex = sum(j = iu, iv, valuation(lo + j - 1, p)),
        if (q == p && p > 1, ex += valuation(lo + iv - 1, p)));
      if (p > 1 && ex >= 2, for (j = iu, iv, mem[j] = 1))));
  my(s = L - lo + 1, e = s);
  if (!mem[s] || mem[s - 1], return(0));
  while (mem[e + 1], e++);
  lo + e - 1;
}
read("runs_2e14.gp");
{
  my(bad = 0, hist = vector(20));
  for (i = 1, #R, my(L = R[i][1], E = R[i][2], len = R[i][3], got = chk(L, E, 64));
    if (got != E, bad++; print("MISMATCH ", R[i], " definition gives end ", got), hist[len]++));
  print("runs checked: ", #R, ", mismatches: ", bad, ", lengths 10..16: ", vector(7, j, hist[j + 9]));
}
quit;
