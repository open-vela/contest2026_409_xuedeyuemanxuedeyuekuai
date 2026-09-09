from pathlib import Path
import struct
p=Path(r'app/watchface_app/assets')
def rle(data, blk=2):
    n=len(data)//blk
    out=bytearray(); i=0
    # pack bytes into 16-bit block values
    vals=[int.from_bytes(data[j*blk:(j+1)*blk],'little') for j in range(n)]
    while i<n:
        run=1
        while i+run<n and vals[i+run]==vals[i] and run<127: run+=1
        if run>=3:
            out.append(run); out += vals[i].to_bytes(blk,'little'); i+=run
        else:
            j=i; lit=[]
            while j<n and len(lit)<127:
                r=1
                while j+r<n and vals[j+r]==vals[j] and r<127: r+=1
                if r>=3: break
                lit.append(vals[j]); j+=1
            out.append(0x80|len(lit))
            for v in lit: out += v.to_bytes(blk,'little')
            i=j
    return bytes(out)

def header(comp,raw,method=1):
    return struct.pack('<III',method,len(comp),len(raw))+comp
for f in sorted(p.glob('*.bin')):
    raw=f.read_bytes(); comp=rle(raw); packed=header(comp,raw)
    print(f'{f.name:20s} raw={len(raw):8d} packed={len(packed):8d} ratio={len(packed)/len(raw):6.1%}')
print('total raw',sum(len(f.read_bytes()) for f in p.glob('*.bin')))
