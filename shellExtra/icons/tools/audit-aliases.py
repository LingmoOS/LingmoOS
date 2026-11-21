#!/usr/bin/env python3
from pathlib import Path
import posixpath
import re
root=Path(__file__).resolve().parents[1]
files={}
for tree in ('src','links'):
    for path in (root/tree).rglob('*'):
        if path.is_file(): files[path.relative_to(root/tree).as_posix()]=path
aliases={}
for key,path in files.items():
    if path.stat().st_size>512:continue
    try: text=path.read_text().strip()
    except UnicodeError:continue
    if re.fullmatch(r'[A-Za-z0-9_.+@/ -]+\.(svg|png|xpm)',text):aliases[key]=text
bad=[]
for key,target in aliases.items():
    seen={key};dest=posixpath.normpath(posixpath.join(posixpath.dirname(key),target))
    while dest in aliases and dest not in seen:
        seen.add(dest);dest=posixpath.normpath(posixpath.join(posixpath.dirname(dest),aliases[dest]))
    if dest not in files or dest in seen:bad.append((key,target,dest))
for row in bad:print(*row,sep=' -> ')
print('Audited',len(aliases),'aliases;',len(bad),'broken')
raise SystemExit(bool(bad))
