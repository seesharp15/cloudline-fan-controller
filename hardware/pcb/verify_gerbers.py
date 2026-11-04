#!/usr/bin/env python3
"""Independent check of the fabrication files themselves (no KiCad needed).

    python3 hardware/pcb/verify_gerbers.py

check_pcb.py proves the *KiCad board* realises the schematic. This script
proves the *files sent to the fab* do: it reads only fab/gerbers/ (Gerber
RS-274X + Excellon), rebuilds copper connectivity from the flashed pads and
drawn tracks, and compares it with the expected netlist in check_pcb.py.
Pads are identified by the X2 object attributes KiCad writes in front of
every flash (TO.P,<ref>,<pad>). It also measures copper clearances and edge
distance from the Gerber geometry, and checks every drill hit has a pad
(or is a mounting hole), every pad has a hole and a mask opening, and the
outline is closed. Standard library only.
"""

import ast
import math
import os
import re
import sys
from collections import defaultdict

HERE = os.path.dirname(os.path.abspath(__file__))
GERB = sys.argv[1] if len(sys.argv) > 1 else os.path.join(HERE, "fab", "gerbers")
STEM = "cloudline_controller"
MIN_CLEARANCE = 0.60  # mm, the board's design rule (README: re-routed for isolation milling)
MIN_EDGE = 0.30       # mm, copper to board outline


# ---------------------------------------------------------------------------
# Readers
# ---------------------------------------------------------------------------
def read_gerber(path):
    text = open(path).read()
    fmt = re.search(r"%FSLAX(\d)(\d)Y(\d)(\d)\*%", text)
    assert fmt and "%MOMM*%" in text, f"{path}: expected metric FSLA format"
    scale = 10 ** int(fmt.group(2))
    aps = {}
    for m in re.finditer(r"%ADD(\d+)([CRO]),([\d.]+)(?:X([\d.]+))?\*%", text):
        w = float(m.group(3))
        aps[int(m.group(1))] = (m.group(2), w, float(m.group(4)) if m.group(4) else w)
    flashes, draws, regions = [], [], 0
    cur, x, y, pad = None, 0.0, 0.0, None
    for line in text.splitlines():
        line = line.strip()
        m = re.match(r"G04 #@! TO\.P,([^,*]+),([^,*]+)", line)
        if m:
            pad = f"{m.group(1)}.{m.group(2)}"
            continue
        if line.startswith("G04 #@! TD") or line.startswith("%TD"):
            pad = None
            continue
        if not line or line.startswith("G04") or line.startswith("%") or line in ("G01*", "G75*", "M02*"):
            continue
        if line in ("G36*", "G37*"):
            regions += 1
            continue
        m = re.fullmatch(r"D(\d+)\*", line)
        if m:
            cur = int(m.group(1))
            continue
        m = re.fullmatch(r"(?:G0?1)?X(-?\d+)Y(-?\d+)D0([123])\*", line)
        if not m:
            raise ValueError(f"{os.path.basename(path)}: unsupported Gerber statement {line!r}")
        nx, ny, op = int(m.group(1)) / scale, int(m.group(2)) / scale, m.group(3)
        if op == "1":
            draws.append(((x, y), (nx, ny), aps[cur][1]))
        elif op == "3":
            flashes.append((nx, ny, aps[cur], pad))
        x, y = nx, ny
    return {"flashes": flashes, "draws": draws, "regions": regions}


def read_drill(path):
    tools, holes, cur = {}, [], None
    for line in open(path).read().splitlines():
        if m := re.fullmatch(r"T(\d+)C([\d.]+)", line):
            tools[int(m.group(1))] = float(m.group(2))
        elif m := re.fullmatch(r"T(\d+)", line):
            cur = int(m.group(1))
        elif m := re.fullmatch(r"X(-?[\d.]+)Y(-?[\d.]+)", line):
            holes.append((float(m.group(1)), float(m.group(2)), tools[cur]))
    return holes


# ---------------------------------------------------------------------------
# Geometry: capsules (segment + radius) and axis-aligned rectangles
# ---------------------------------------------------------------------------
def _pt_seg(p, a, b):
    dx, dy = b[0] - a[0], b[1] - a[1]
    L = dx * dx + dy * dy
    t = 0 if L == 0 else max(0, min(1, ((p[0] - a[0]) * dx + (p[1] - a[1]) * dy) / L))
    return math.hypot(p[0] - a[0] - t * dx, p[1] - a[1] - t * dy)


def _cross(a, b, c, d):
    def o(p, q, r):
        return (q[0] - p[0]) * (r[1] - p[1]) - (q[1] - p[1]) * (r[0] - p[0])
    return o(a, b, c) * o(a, b, d) < 0 and o(c, d, a) * o(c, d, b) < 0


def seg_seg(a, b, c, d):
    if _cross(a, b, c, d):
        return 0.0
    return min(_pt_seg(a, c, d), _pt_seg(b, c, d), _pt_seg(c, a, b), _pt_seg(d, a, b))


def seg_rect(a, b, r):
    x1, y1, x2, y2 = r
    if any(x1 <= p[0] <= x2 and y1 <= p[1] <= y2 for p in (a, b)):
        return 0.0
    edges = [((x1, y1), (x2, y1)), ((x2, y1), (x2, y2)), ((x2, y2), (x1, y2)), ((x1, y2), (x1, y1))]
    return min(seg_seg(a, b, c, d) for c, d in edges)


def shape_of(flash):
    x, y, (kind, w, h), _ = flash
    if kind == "R":
        return ("rect", (x - w / 2, y - h / 2, x + w / 2, y + h / 2))
    if kind == "C" or abs(w - h) < 1e-9:
        return ("cap", ((x, y), (x, y), w / 2))
    d = abs(w - h) / 2
    if w > h:
        return ("cap", ((x - d, y), (x + d, y), h / 2))
    return ("cap", ((x, y - d), (x, y + d), w / 2))


def gap(s, t):
    if s[0] == "rect" and t[0] == "rect":
        a, b = s[1], t[1]
        dx = max(0, a[0] - b[2], b[0] - a[2])
        dy = max(0, a[1] - b[3], b[1] - a[3])
        return math.hypot(dx, dy) if dx or dy else -1.0
    if s[0] == "rect":
        s, t = t, s
    (a, b, r) = s[1]
    if t[0] == "rect":
        return seg_rect(a, b, t[1]) - r
    (c, d, r2) = t[1]
    return seg_seg(a, b, c, d) - r - r2


def bbox(s):
    if s[0] == "rect":
        return s[1]
    (a, b, r) = s[1]
    return (min(a[0], b[0]) - r, min(a[1], b[1]) - r, max(a[0], b[0]) + r, max(a[1], b[1]) + r)


def outline_closed(draws):
    """True when the outline segments, in any order and direction, form one
    closed loop: every vertex joins exactly two segments and all are linked."""
    if not draws:
        return False
    key = lambda p: (round(p[0], 4), round(p[1], 4))  # noqa: E731
    adj = defaultdict(list)
    for a, b, _ in draws:
        adj[key(a)].append(key(b))
        adj[key(b)].append(key(a))
    if any(len(v) != 2 for v in adj.values()):
        return False
    start = next(iter(adj))
    seen, stack = {start}, [start]
    while stack:
        for q in adj[stack.pop()]:
            if q not in seen:
                seen.add(q)
                stack.append(q)
    return len(seen) == len(adj)


# ---------------------------------------------------------------------------
def expected_netlist():
    """EXPECTED from check_pcb.py, read without importing it (it needs pcbnew)."""
    tree = ast.parse(open(os.path.join(HERE, "check_pcb.py")).read())
    for node in tree.body:
        if isinstance(node, ast.Assign) and getattr(node.targets[0], "id", "") == "EXPECTED":
            return ast.literal_eval(node.value)
    raise SystemExit("EXPECTED not found in check_pcb.py")


def main():
    f = lambda suffix: os.path.join(GERB, f"{STEM}-{suffix}")  # noqa: E731
    cu = read_gerber(f("B_Cu.gbl"))
    mask = read_gerber(f("B_Mask.gbs"))
    edge = read_gerber(f("Edge_Cuts.gm1"))
    holes = read_drill(os.path.join(GERB, f"{STEM}.drl"))
    expected = expected_netlist()
    problems = []

    shapes = [shape_of(fl) for fl in cu["flashes"]] + \
             [("cap", (a, b, w / 2)) for a, b, w in cu["draws"]]
    pads = [fl[3] for fl in cu["flashes"]] + [None] * len(cu["draws"])
    boxes = [bbox(s) for s in shapes]
    n = len(shapes)
    parent = list(range(n))

    def find(i):
        while parent[i] != i:
            parent[i] = parent[parent[i]]
            i = parent[i]
        return i

    near = []
    for i in range(n):
        for j in range(i + 1, n):
            bi, bj = boxes[i], boxes[j]
            if bj[0] > bi[2] + 2 or bj[2] < bi[0] - 2 or bj[1] > bi[3] + 2 or bj[3] < bi[1] - 2:
                continue
            g = gap(shapes[i], shapes[j])
            if g <= 1e-6:
                parent[find(i)] = find(j)
            else:
                near.append((g, i, j))

    # connectivity vs. expected netlist
    where = {p: i for i, p in enumerate(pads) if p}
    for net, pins in expected.items():
        missing = [p for p in pins if p not in where]
        if missing:
            problems.append(f"{net}: pads not found in the copper Gerber: {missing}")
        roots = {find(where[p]) for p in pins if p in where}
        if len(roots) > 1:
            problems.append(f"OPEN  {net}: {len(roots)} separate copper islands")
    owner = {}
    for net, pins in expected.items():
        for p in pins:
            if p in where:
                r = find(where[p])
                if owner.setdefault(r, net) != net:
                    problems.append(f"SHORT {net} and {owner[r]} share copper (at {p})")
    islands = defaultdict(list)
    for i in range(n):
        islands[find(i)].append(i)
    for r, members in islands.items():
        if r not in owner and any(pads[k] is None for k in members):
            named = [pads[k] for k in members if pads[k]]
            if named and all(p.startswith("W") for p in named):
                continue
            problems.append(f"copper with tracks belonging to no expected net: pads {named}")

    # clearance and edge distance, measured on the Gerber geometry
    gmin, worst = 99.0, None
    for g, i, j in near:
        if find(i) != find(j) and g < gmin:
            gmin, worst = g, (i, j)
    xs = [p[0] for a, b, _ in edge["draws"] for p in (a, b)]
    ys = [p[1] for a, b, _ in edge["draws"] for p in (a, b)]
    ex1, ey1, ex2, ey2 = min(xs), min(ys), max(xs), max(ys)
    emin = min(min(b[0] - ex1, b[1] - ey1, ex2 - b[2], ey2 - b[3]) for b in boxes)
    closed = outline_closed(edge["draws"])
    if gmin < MIN_CLEARANCE - 1e-6:
        problems.append(f"copper clearance {gmin:.3f} mm < {MIN_CLEARANCE} "
                        f"({pads[worst[0]] or 'track'} / {pads[worst[1]] or 'track'})")
    if emin < MIN_EDGE:
        problems.append(f"copper {emin:.3f} mm from the board edge")
    if not closed:
        problems.append("board outline is not a single closed path")

    # drills, mask
    flash_xy = [(fl[0], fl[1]) for fl in cu["flashes"]]
    unmatched = [(x, y, d) for x, y, d in holes if not any(abs(x - fx) < 0.005 and abs(y - fy) < 0.005 for fx, fy in flash_xy)]
    mounting = [h for h in unmatched if h[2] >= 3.0]
    stray = [h for h in unmatched if h[2] < 3.0]
    if stray:
        problems.append(f"{len(stray)} drill hits with no copper pad: {stray[:4]}")
    no_hole = [fl[3] for fl in cu["flashes"] if not any(abs(fl[0] - x) < 0.005 and abs(fl[1] - y) < 0.005 for x, y, _ in holes)]
    if no_hole:
        problems.append(f"pads with no drill hole: {no_hole[:6]}")
    ring = min(min(fl[2][1], fl[2][2]) / 2 - d / 2 for fl in cu["flashes"] for x, y, d in holes
               if abs(fl[0] - x) < 0.005 and abs(fl[1] - y) < 0.005)
    mask_xy = {(round(fl[0], 3), round(fl[1], 3)) for fl in mask["flashes"]}
    no_mask = [fl[3] for fl in cu["flashes"] if (round(fl[0], 3), round(fl[1], 3)) not in mask_xy]
    if no_mask:
        problems.append(f"pads without a solder-mask opening: {no_mask[:6]}")

    print(f"read {len(cu['flashes'])} pads, {len(cu['draws'])} track segments, "
          f"{cu['regions']} filled regions, {len(holes)} drill hits ({len(mounting)} mounting holes)")
    print(f"outline {ex2 - ex1:.2f} x {ey2 - ey1:.2f} mm, {'closed' if closed else 'NOT closed'}")
    print(f"{len(expected)} expected nets, {sum(map(len, expected.values()))} pads; "
          f"{len(islands)} copper islands in the Gerber")
    print(f"min clearance between islands {gmin:.3f} mm; min copper-to-edge {emin:.3f} mm; "
          f"min annular ring {ring:.3f} mm")
    for p in problems:
        print("PROBLEM", p)
    print("RESULT:", "FAIL" if problems else "PASS (no opens, no shorts, rules met in the fab files)")
    return 1 if problems else 0


if __name__ == "__main__":
    sys.exit(main())
