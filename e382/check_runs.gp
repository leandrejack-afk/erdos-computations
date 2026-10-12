\\ check_runs.gp: local check, with PARI/GP's factor(), that each claimed run is a maximal
\\ run of consecutive members of A388654. For every k within D of the run it computes the
\\ largest prime factor P(k), finds every m with P(m)^2 | m, marks the window around each
\\ such m on which P stays <= P(m) (stopping with an error if a window reaches the edge),
\\ and reads off the run of marked integers that contains the claimed start.
P(k) = if(k == 1, 1, vecmax(factor(k)[, 1]));
checkrun(L, R, D) = {
  my(lo = max(1, L - D), hi = R + D, n = hi - lo + 1, PP = vector(n), mark = vector(n), a, b, s, e);
  for (i = 1, n, PP[i] = P(lo + i - 1));
  for (i = 1, n,
    my(k = lo + i - 1, p = PP[i]);
    if (k > 1 && k % p^2 == 0,
      a = i; while (a > 1 && PP[a - 1] <= p, a--);
      b = i; while (b < n && PP[b + 1] <= p, b++);
      if ((a == 1 && lo > 1) || b == n, error("window reaches the edge at ", k));
      for (j = a, b, mark[j] = 1)));
  s = L - lo + 1;
  if (!mark[s], return([0, "start not marked"]));
  e = s; while (e < n && mark[e + 1], e++);
  if (s > 1 && mark[s - 1], return([0, "member before start"]));
  [1, [L, lo + e - 1, e - s + 1]];
}
runs = [[4928180396, 4928180405], [40533366231, 40533366241], [111460496439, 111460496450], [436502026483, 436502026495], [17952448643078, 17952448643091]];
{
  for (r = 1, #runs,
    my(L = runs[r][1], R = runs[r][2], res = checkrun(L, R, 200), m, p);
    p = 0; for (k = L, R, my(q = P(k)); if (q > p, p = q; m = k));
    printf("run %d..%d: %s  largest prime %d at %d = %d^2 * %d; P(L-1) = %d, P(R+1) = %d\n",
      L, R, res, p, m, p, m / p^2, P(L - 1), P(R + 1)));
}
quit;
