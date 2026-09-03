import functools, sys, time
sys.path.insert(0, "/Users/leandrejack/projects/open-problems/targets/e1204")
import smin_short
smin_short.a = functools.lru_cache(None)(smin_short.a)
for n in range(1, 400):
    t0 = time.time()
    v = smin_short.a(n)
    print(f"k={n} S={v} secs={time.time() - t0:.1f}", flush=True)
