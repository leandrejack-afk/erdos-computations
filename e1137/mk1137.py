import re,glob,os
log=open('logs/gaps_1e13.log').read()
rows=[]
for m in re.finditer(r'^X=(\d+) maxgap=(\d+) \(after (\d+)\) maxprod=(\d+) \(=(\d+)\*(\d+) after (\d+)\) ratio=([\d.]+)',log,re.M):
    X,M,Mp,P,g1,g2,Pp,r=m.groups()
    if int(X)>=10**6: rows.append([int(X),int(M),int(Mp),int(P),int(g1),int(g2),int(Pp)])
recs=[(int(P),int(g1),int(g2),int(mid)) for P,g1,g2,mid in re.findall(r'^RECORD PROD (\d+) = (\d+)\*(\d+) \(middle prime (\d+)\)',log,re.M)]
def segs(pattern,n):
    fs=sorted(glob.glob(pattern),key=lambda f:int(re.search(r'_(\d+)\.txt',f).group(1)))
    out=[]
    for f in fs:
        s=open(f).read().strip()
        m=re.match(r'SEG (\d+) (\d+) maxgap=(\d+) after=(\d+) maxprod=(\d+) g1=(\d+) g2=(\d+) mid=(\d+)',s)
        if m: out.append([int(x) for x in m.groups()])
    return out if len(out)==n else None
state=rows[-1][:]  # state at 1e11
segrecs=[]
for pattern,n,X in [('segs/seg12_*.txt',9,10**12),('segs/seg13_*.txt',18,10**13)]:
    S=segs(pattern,n)
    if S is None: break
    X0,M,Mp,P,g1,g2,Pp=state
    for A,B,mg,mgp,mp,sg1,sg2,mid in S:
        if mg>M: M,Mp=mg,mgp
        if mp>P: P,g1,g2,Pp=mp,sg1,sg2,mid; segrecs.append((mp,sg1,sg2,mid,M))
    state=[X,M,Mp,P,g1,g2,Pp]; rows.append(state[:])
out=["| X | M(X) = max gap below X (after prime) | P(X) = max product of consecutive gaps below X | P(X)/M(X)^2 |","|---|---|---|---|"]
for X,M,Mp,P,g1,g2,Pp in rows:
    out.append(f"| 10^{len(str(X))-1} | {M} (after {Mp}) | {P} = {g1}*{g2} (middle prime {Pp}) | {P/(M*M):.3f} |")
out.append("")
out.append("Record products of two consecutive gaps (product = g1*g2, middle prime = the prime between the two gaps): the last 12 of the 54 records below 10^11 from the single scan (the 54 middle primes are exactly the 54 terms of OEIS A120384), then, for the parallel segment scans above 10^11, each segment maximum that beat the running maximum (segment maxima only, so this is not the complete record sequence above 10^11):")
out.append("")
for P,g1,g2,mid in recs[-12:]:
    out.append(f"- {P} = {g1}*{g2}, middle prime {mid}")
for P,g1,g2,mid,M in segrecs:
    out.append(f"- {P} = {g1}*{g2}, middle prime {mid} (segment maximum)")
print("\n".join(out))
