#!/usr/bin/env python3
"""Draw schematic_preview.png from the EAGLE schematic (and report overlapping symbols).

    python3 preview_schematic.py cloudline_controller.sch schematic_preview.png
"""
import sys, math
from lxml import etree
import matplotlib; matplotlib.use("Agg")
import matplotlib.pyplot as plt
root = etree.parse(sys.argv[1]).getroot()
lib = root.find(".//schematic/libraries/library")
syms = {s.get("name"): s for s in lib.iter("symbol")}
dsym = {d.get("name"): d.find("gates/gate").get("symbol") for d in lib.iter("deviceset")}
parts = {p.get("name"): (p.get("deviceset"), p.get("value")) for p in root.iter("part")}
fig, ax = plt.subplots(figsize=(22, 14))
boxes = []
for i in root.iter("instance"):
    ix, iy = float(i.get("x")), float(i.get("y")); s = syms[dsym[parts[i.get("part")][0]]]
    xs, ys = [], []
    for w in s.iter("wire"):
        x1, y1, x2, y2 = (float(w.get(k)) for k in ("x1", "y1", "x2", "y2"))
        ax.plot([ix+x1, ix+x2], [iy+y1, iy+y2], color="#8b0000", lw=1); xs += [x1, x2]; ys += [y1, y2]
    for c in s.iter("circle"): pass
    for p in s.iter("pin"):
        x, y = float(p.get("x")), float(p.get("y")); d = -1 if p.get("rot") == "R180" else 1
        ax.plot([ix+x, ix+x+2.54*d], [iy+y, iy+y], color="#8b0000", lw=1); xs.append(x); ys.append(y)
        if len(list(s.iter("pin"))) > 2:
            ax.text(ix+x+3.2*d, iy+y, p.get("name"), fontsize=6, va="center", ha="left" if d > 0 else "right")
    ax.text(ix+min(xs), iy+max(ys)+1.5, f'{i.get("part")}  {parts[i.get("part")][1]}', fontsize=7)
    boxes.append((i.get("part"), ix+min(xs)-1, iy+min(ys)-1, ix+max(xs)+1, iy+max(ys)+3))
for seg in root.iter("segment"):
    net = seg.getparent().get("name")
    for w in seg.iter("wire"):
        ax.plot([float(w.get("x1")), float(w.get("x2"))], [float(w.get("y1")), float(w.get("y2"))], color="#006400", lw=1)
    l = seg.find("label")
    r = l.get("rot") == "R180"
    ax.text(float(l.get("x")) + (-0.5 if r else 0.5), float(l.get("y")), net, fontsize=6, color="#006400",
            va="center", ha="right" if r else "left")
for t in root.iter("text"):
    if t.getparent().tag == "plain": ax.text(float(t.get("x")), float(t.get("y")), t.text, fontsize=7)
ax.set_aspect("equal"); ax.axis("off"); fig.tight_layout(); fig.savefig(sys.argv[2], dpi=110)
ov = [(a[0], b[0]) for k, a in enumerate(boxes) for b in boxes[k+1:] if a[1] < b[3] and b[1] < a[3] and a[2] < b[4] and b[2] < a[4]]
print("overlapping symbol boxes:", ov)
