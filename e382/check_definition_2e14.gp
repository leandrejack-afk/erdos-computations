\\ check_definition_2e14.gp: definition-only local checks for the search to 2*10^14.
\\ An interval [u,v] is bad iff the largest prime p dividing prod_{k=u..v} k has
\\ sum_k valuation(k,p) >= 2. Every interval [u,v] with v-u <= B and u near the target is
\\ tested literally; members are the union of the bad ones.
P(k) = if(k == 1, 1, vecmax(factor(k)[, 1]));
direct(L, R, D, B) = {
  my(lo = L - D - B, hi = R + D + B, n = hi - lo + 1, PP = vector(n), mem = vector(n), maxlen = 0);
  for (i = 1, n, PP[i] = P(lo + i - 1));
  for (iu = B + 1, n - 2*B,
    my(p = 1, ex = 0);
    for (iv = iu, iu + B,
      my(q = PP[iv]);
      if (q > p, p = q; ex = sum(j = iu, iv, valuation(lo + j - 1, p)),
        if (q == p && p > 1, ex += valuation(lo + iv - 1, p)));
      if (p > 1 && ex >= 2,
        for (j = iu, iv, mem[j] = 1);
        maxlen = max(maxlen, iv - iu + 1))));
  my(s = L - lo + 1, e = s);
  if (!mem[s], return([0, "start not member"]));
  if (mem[s - 1], return([0, "L-1 is a member"]));
  while (mem[e + 1], e++);
  [1, L, lo + e - 1, e - s + 1, "max bad length seen", maxlen];
}
\\ the run of length 16 and the seven runs of length 14 below 2*10^14
runs = [[52629641927627, 52629641927642], [17952448643078, 17952448643091], [37815743549294, 37815743549307], [43924829911549, 43924829911562], [48939424551930, 48939424551943], [116248526623312, 116248526623325], [145404841742439, 145404841742452], [192495617769831, 192495617769844]];
{
  for (r = 1, #runs, print(direct(runs[r][1], runs[r][2], 60, 64)));
  my(m = 52629641927641); print("m = ", m, " = ", factor(m), "; P of each member: ", vector(16, i, P(52629641927626 + i)));
}
\\ boundary of the 2*10^14 search: every run that starts in [2e14-80, 2e14] (definition only)
{
  my(X = 200000000000000, lo = X - 300, n = 700, PP = vector(n), mem = vector(n), B = 64);
  for (i = 1, n, PP[i] = P(lo + i - 1));
  for (iu = 1, n - B,
    my(p = 1, ex = 0);
    for (iv = iu, iu + B,
      my(q = PP[iv]);
      if (q > p, p = q; ex = sum(j = iu, iv, valuation(lo + j - 1, p)),
        if (q == p && p > 1, ex += valuation(lo + iv - 1, p)));
      if (p > 1 && ex >= 2, for (j = iu, iv, mem[j] = 1))));
  my(cnt = 0, best = 0);
  for (i = 2, n - 2*B, if (mem[i] && !mem[i-1], my(e = i); while (mem[e+1], e++); cnt++; if (e-i+1 > best, best = e-i+1); if (lo+i-1 >= X-80 && lo+i-1 <= X, print("run near 2e14: ", lo+i-1, "..", lo+e-1, " len ", e-i+1))));
  print("runs in [2e14-299, 2e14+270]: ", cnt, ", longest ", best, "; 2e14 = ", factor(X));
}
quit;
