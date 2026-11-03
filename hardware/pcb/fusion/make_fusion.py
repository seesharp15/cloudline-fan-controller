#!/usr/bin/env python3
"""Export the PCB as Fusion 360 machining geometry for isolation milling.

    python3 hardware/pcb/fusion/make_fusion.py

Reads hardware/pcb/cloudline_controller.kicad_pcb (KiCad 7 pcbnew module) and
writes, into hardware/pcb/fusion/:

  cloudline_pcb_MILL_copper_up.step   3D model for Fusion's Manufacture
        workspace: FR-4 substrate (with every hole) and each copper island as
        its own thin body on top. MIRRORED: this is the board as it lies on
        the machine bed, copper side up. Origin = board corner, Z = 0 on the
        bed (bottom of the board), copper top at Z = T_BOARD + T_COPPER.
  cloudline_pcb_MILL_copper_up.dxf    The same geometry in 2D, mm, layers
        OUTLINE, COPPER, DRILL_<d>mm (one layer per hole size), FIDUCIAL.
  cloudline_pcb_TOPVIEW.step          Not mirrored, parts side up: for
        designing the enclosure in Fusion's Design workspace.
  mill_preview.png                    What the mill side looks like.
  milling_report.txt                  Copper gaps (which isolation tools fit),
        hole sizes and counts, copper-to-edge distances.

Copper geometry is rebuilt from the board's pads and tracks (not from the
Gerbers) and checked: islands of different nets must not touch.
"""

import math
import os
import sys
from collections import Counter, defaultdict

import pcbnew
from shapely import affinity
from shapely.geometry import LineString, MultiPolygon, Point, Polygon, box
from shapely.ops import unary_union

HERE = os.path.dirname(os.path.abspath(__file__))
PCB = os.path.join(os.path.dirname(HERE), "cloudline_controller.kicad_pcb")
OX, OY = 50.0, 40.0          # page offset used by make_pcb.py
W, H = 100.0, 75.0           # board size (mm)
T_BOARD = 1.6                # FR-4 thickness (mm), adjust to your blank
T_COPPER = 0.035             # 1 oz copper
QUAD = 16                    # arc segments per quarter circle

# Isolation tool (V-bit) used to compute the ISO_PASS lines in the DXF.
# Cut width = tip + 2 * depth * tan(angle / 2). Change these to match your
# bit and regenerate; milling_report.txt lists the result.
VBIT_ANGLE = 30.0            # included angle, degrees
VBIT_TIP = 0.1               # tip (flat) diameter, mm
CUT_DEPTH = 0.20             # below the copper surface, mm (copper is 0.035 or 0.07)
Z_TOL = 0.10                 # board height error the paths must tolerate (no height map)
PASS_OVERLAP = 0.4           # overlap between isolation passes
N_PASSES = 2                 # pass 1 hugs the copper, pass 2 widens the gap


def mm(v):
    return pcbnew.ToMM(v)


def pad_shape(p):
    """Copper shape of a pad (top-view board mm, y down), holes not removed."""
    c = p.GetPosition()
    x, y = mm(c.x) - OX, mm(c.y) - OY
    sx, sy = mm(p.GetSize().x), mm(p.GetSize().y)
    if abs((p.GetOrientation().AsDegrees() % 180) - 90) < 1:
        sx, sy = sy, sx
    shape = p.GetShape()
    if shape == pcbnew.PAD_SHAPE_CIRCLE:
        return Point(x, y).buffer(sx / 2, quad_segs=QUAD)
    if shape == pcbnew.PAD_SHAPE_OVAL:
        r = min(sx, sy) / 2
        dx, dy = (sx / 2 - r, 0) if sx >= sy else (0, sy / 2 - r)
        return LineString([(x - dx, y - dy), (x + dx, y + dy)]).buffer(r, quad_segs=QUAD) \
            if (dx or dy) else Point(x, y).buffer(r, quad_segs=QUAD)
    return box(x - sx / 2, y - sy / 2, x + sx / 2, y + sy / 2)


def pad_prim(p):
    """Exact primitive of a pad: ("circle", x, y, r) / ("slot", x1, y1, x2, y2, w) / ("rect", x, y, w, h)."""
    c = p.GetPosition()
    x, y = mm(c.x) - OX, mm(c.y) - OY
    sx, sy = mm(p.GetSize().x), mm(p.GetSize().y)
    if abs((p.GetOrientation().AsDegrees() % 180) - 90) < 1:
        sx, sy = sy, sx
    shape = p.GetShape()
    if shape == pcbnew.PAD_SHAPE_CIRCLE or (shape == pcbnew.PAD_SHAPE_OVAL and abs(sx - sy) < 1e-6):
        return ("circle", x, y, sx / 2)
    if shape == pcbnew.PAD_SHAPE_OVAL:
        r = min(sx, sy) / 2
        dx, dy = (sx / 2 - r, 0) if sx >= sy else (0, sy / 2 - r)
        return ("slot", x - dx, y - dy, x + dx, y + dy, 2 * r)
    return ("rect", x, y, sx, sy)


def collect():
    board = pcbnew.LoadBoard(PCB)
    by_net = defaultdict(list)
    prims = defaultdict(list)
    holes = []          # (x, y, diameter, plated_pad?)
    for fp in board.GetFootprints():
        for p in fp.Pads():
            c = p.GetPosition()
            x, y = mm(c.x) - OX, mm(c.y) - OY
            d = round(mm(p.GetDrillSize().x), 3)
            if d > 0:
                holes.append((x, y, d, fp.GetReference()))
            if p.IsOnLayer(pcbnew.B_Cu) and p.GetAttribute() != pcbnew.PAD_ATTRIB_NPTH:
                net = p.GetNetname() or f"<nc:{fp.GetReference()}.{p.GetNumber()}>"
                by_net[net].append(pad_shape(p))
                prims[net].append(pad_prim(p))
    for t in board.GetTracks():
        if t.GetLayer() != pcbnew.B_Cu:
            raise SystemExit("copper outside B.Cu found: the board must be single-sided")
        s, e = t.GetStart(), t.GetEnd()
        line = LineString([(mm(s.x) - OX, mm(s.y) - OY), (mm(e.x) - OX, mm(e.y) - OY)])
        by_net[t.GetNetname()].append(line.buffer(mm(t.GetWidth()) / 2, quad_segs=QUAD))
        prims[t.GetNetname()].append(("slot", mm(s.x) - OX, mm(s.y) - OY, mm(e.x) - OX, mm(e.y) - OY,
                                      mm(t.GetWidth())))
    drills = unary_union([Point(x, y).buffer(d / 2, quad_segs=QUAD) for x, y, d, _ in holes])
    nets = {}
    for net, shapes in by_net.items():
        geom = unary_union(shapes).difference(drills)
        nets[net] = geom
    return nets, holes, prims


def mirror_up(geom):
    """Top view, y down  ->  mill side (copper up), y up: x' = W - x, y' = H - y."""
    return affinity.affine_transform(geom, [-1, 0, 0, -1, W, H])


def topview_up(geom):
    """Top view, y down  ->  top view, y up (for the enclosure model)."""
    return affinity.affine_transform(geom, [1, 0, 0, -1, 0, H])


def polys(geom):
    if isinstance(geom, Polygon):
        return [geom]
    if isinstance(geom, MultiPolygon):
        return list(geom.geoms)
    return [g for g in getattr(geom, "geoms", []) if isinstance(g, Polygon)]


def gap_report(nets, holes):
    lines = []
    names = sorted(nets)
    gaps = []
    for i, a in enumerate(names):
        for b in names[i + 1:]:
            d = nets[a].distance(nets[b])
            if d < 2.0:
                gaps.append((d, a, b))
            if d <= 1e-6:
                raise SystemExit(f"SHORT: copper of {a} touches {b}")
    gaps.sort()
    outline = box(0, 0, W, H).exterior
    edge = min(outline.distance(g) for g in nets.values())
    lines.append("Milling report: Cloudline controller PCB (Rev B)")
    lines.append("=" * 50)
    lines.append(f"Board {W:.1f} x {H:.1f} mm, copper islands: {sum(len(polys(g)) for g in nets.values())}"
                 f" in {len(nets)} nets")
    lines.append("")
    lines.append("Smallest copper-to-copper gaps between different nets")
    lines.append("(the isolation cut must fit inside these):")
    for d, a, b in gaps[:12]:
        lines.append(f"  {d:6.3f} mm   {a} <-> {b}")
    lines.append(f"  smallest gap: {gaps[0][0]:.3f} mm")
    for lim in (0.40, 0.50, 0.60):
        n = sum(1 for d, _, _ in gaps if d < lim)
        lines.append(f"  net pairs closer than {lim:.2f} mm: {n}")
    lines.append(f"Smallest copper-to-board-edge distance: {edge:.3f} mm")
    lines.append("")
    lines.append("Isolation cut width of a V-bit = tip + 2 * depth * tan(angle / 2):")
    for ang, tip in ((20, 0.1), (30, 0.1), (30, 0.2), (60, 0.1)):
        for depth in (0.10, 0.20, 0.30):
            wdt = tip + 2 * depth * math.tan(math.radians(ang / 2))
            ok = "fits" if wdt <= gaps[0][0] - 0.05 else "TOO WIDE for the smallest gap"
            lines.append(f"  {ang:2d} deg, {tip:.1f} mm tip, {depth:.2f} mm deep: {wdt:.3f} mm  {ok}")
    lines.append("  (0.05 mm kept as margin; a flat end mill cuts its own diameter)")
    lines.append("")
    cnt = Counter(d for _, _, d, _ in holes)
    lines.append("Holes (drill diameter: count):")
    for d in sorted(cnt):
        lines.append(f"  {d:.2f} mm: {cnt[d]}")
    lines.append(f"  total: {sum(cnt.values())}")
    return "\n".join(lines) + "\n", gaps[0][0]


def cut_width(depth=CUT_DEPTH):
    return VBIT_TIP + 2 * depth * math.tan(math.radians(VBIT_ANGLE / 2))


def isolation_passes(nets):
    """Tool-centre lines for isolation, in top-view coordinates.
    Pass k runs at w/2 + (k-1) * w * (1 - overlap) from its own copper and is
    clipped wherever it would come closer than w/2 + 0.02 mm to another
    net's copper, or to the board edge."""
    w = cut_width()
    passes = []
    inside = box(0, 0, W, H).buffer(-(w / 2 + 0.2))
    for k in range(N_PASSES):
        off = w / 2 + k * w * (1 - PASS_OVERLAP)
        lines = []
        for net, geom in nets.items():
            others = unary_union([g for n, g in nets.items() if n != net])
            keep_out = others.buffer(w / 2 + 0.02, quad_segs=QUAD)
            solid = unary_union([Polygon(p.exterior) for p in polys(geom)])  # ignore drill holes
            ring = solid.buffer(off, quad_segs=QUAD).boundary
            ring = ring.intersection(inside).difference(keep_out)
            lines.append(ring)
        passes.append(unary_union(lines))
    return passes


def verify_isolation(nets, passes, w):
    """Simulate the cut: remove copper along every pass (width w) and check
    that no remaining copper area joins two different nets."""
    cut = unary_union([p.buffer(w / 2, quad_segs=QUAD) for p in passes])
    sheet = box(0, 0, W, H).difference(cut)        # copper left after milling
    comps = polys(sheet)
    bad = []
    for comp in comps:
        hit = {n for n, g in nets.items() if comp.intersects(g.representative_point().buffer(0.01))}
        if len(hit) > 1:
            bad.append(sorted(hit))
    # every net's copper must survive (the cut never eats into a pad/track)
    # (slivers under 5 um are the polygon approximation of arcs, not cuts)
    kept = sheet.buffer(0.005, quad_segs=QUAD)
    eaten = [n for n, g in nets.items() if g.difference(kept).area > 1e-6]
    return bad, eaten


def write_dxf(path, nets, holes, transform, passes=()):
    import ezdxf
    doc = ezdxf.new("R2010", setup=True)
    doc.units = ezdxf.units.MM
    doc.header["$INSUNITS"] = 4
    msp = doc.modelspace()
    doc.layers.add("OUTLINE", color=7)
    doc.layers.add("COPPER", color=1)
    doc.layers.add("FIDUCIAL", color=3)
    corners = [(0, 0), (W, 0), (W, H), (0, H)]
    msp.add_lwpolyline(corners, close=True, dxfattribs={"layer": "OUTLINE"})
    for geom in nets.values():
        for p in polys(transform(geom)):
            msp.add_lwpolyline(list(p.exterior.coords)[:-1], close=True, dxfattribs={"layer": "COPPER"})
            for ring in p.interiors:
                msp.add_lwpolyline(list(ring.coords)[:-1], close=True, dxfattribs={"layer": "COPPER"})
    for k, pas in enumerate(passes, 1):
        layer = f"ISO_PASS{k}"
        doc.layers.add(layer, color=2 if k == 1 else 6)
        geoms = getattr(transform(pas), "geoms", [transform(pas)])
        for g in geoms:
            if g.is_empty:
                continue
            coords = list(g.coords)
            closed = coords[0] == coords[-1]
            msp.add_lwpolyline(coords[:-1] if closed else coords, close=closed, dxfattribs={"layer": layer})
    for x, y, d, _ in holes:
        pt = transform(Point(x, y))
        layer = f"DRILL_{d:.2f}mm".replace(".", "p")
        if layer not in doc.layers:
            doc.layers.add(layer, color=5)
        msp.add_circle((pt.x, pt.y), d / 2, dxfattribs={"layer": layer})
    # Corner marks 1 mm outside the board, to help align a blank on the bed.
    for cx, cy in corners:
        sx = -1 if cx == 0 else 1
        sy = -1 if cy == 0 else 1
        msp.add_line((cx + sx, cy + sy * 3), (cx + sx, cy + sy), dxfattribs={"layer": "FIDUCIAL"})
        msp.add_line((cx + sx, cy + sy), (cx + sx * 3, cy + sy), dxfattribs={"layer": "FIDUCIAL"})
    doc.saveas(path)


def write_step(path, prims, holes, xf, label, copper_up):
    """Exact B-rep: copper made of true circles, rectangles and round-ended
    slots (tracks), so Fusion sees real arcs. xf maps top-view (x, y-down)
    board coordinates to the model's (X, Y-up)."""
    import build123d as bd

    def face_of(pr):
        kind = pr[0]
        if kind == "circle":
            _, x, y, r = pr
            X, Y = xf(x, y)
            return bd.Pos(X, Y) * bd.Circle(r)
        if kind == "rect":
            _, x, y, w, h = pr
            X, Y = xf(x, y)
            return bd.Pos(X, Y) * bd.Rectangle(w, h)
        _, x1, y1, x2, y2, w = pr
        X1, Y1 = xf(x1, y1)
        X2, Y2 = xf(x2, y2)
        L = math.hypot(X2 - X1, Y2 - Y1)
        if L < 1e-6:
            return bd.Pos(X1, Y1) * bd.Circle(w / 2)
        ang = math.degrees(math.atan2(Y2 - Y1, X2 - X1))
        return bd.Pos((X1 + X2) / 2, (Y1 + Y2) / 2) * bd.Rot(0, 0, ang) * bd.SlotCenterToCenter(L, w)

    hole_faces = []
    for x, y, d, _ in holes:
        X, Y = xf(x, y)
        hole_faces.append(bd.Pos(X, Y) * bd.Circle(d / 2))
    holes_sk = bd.Sketch() + hole_faces
    sub = bd.extrude(bd.Rectangle(W, H, align=(bd.Align.MIN, bd.Align.MIN)) - holes_sk, amount=T_BOARD)
    sub.label = "FR4_substrate"
    parts = [sub]
    z0 = T_BOARD if copper_up else 0.0
    direction = 1 if copper_up else -1
    for net, plist in sorted(prims.items()):
        sk = (bd.Sketch() + [face_of(pr) for pr in plist]) - holes_sk
        if not sk.faces():
            continue
        solid = bd.Pos(0, 0, z0) * bd.extrude(sk, amount=direction * T_COPPER)
        solid.label = "copper_" + net.replace("+", "P").replace("/", "_").replace("<", "").replace(">", "").replace(":", "_")
        parts.append(solid)
    bd.export_step(bd.Compound(children=parts, label=label), path)


def main():
    nets, holes, prims = collect()
    report, min_gap = gap_report(nets, holes)
    passes = isolation_passes(nets)
    w = cut_width()
    bad, eaten = verify_isolation(nets, passes, w)
    # The board may sit up to Z_TOL higher or lower than the touch-off point
    # (no height map): shallower -> narrower cut, which must still isolate;
    # deeper -> wider cut, which bites a little into the copper edges.
    shallow = max(CUT_DEPTH - Z_TOL, 0.0)
    bad_shallow, _ = verify_isolation(nets, passes, cut_width(shallow))
    bite = (cut_width(CUT_DEPTH + Z_TOL) - w) / 2
    if shallow < 0.07:
        bad_shallow.append(f"cut only {shallow:.2f} mm deep at the high spot: not through 70 um copper")
    report += (f"\nIsolation lines in the DXF (layers ISO_PASS1..{N_PASSES}) computed for a "
               f"{VBIT_ANGLE:.0f} deg V-bit, {VBIT_TIP:.2f} mm tip, {CUT_DEPTH:.2f} mm deep:\n"
               f"  cut width {w:.3f} mm, pass offsets from the copper: "
               + ", ".join(f"{w / 2 + k * w * (1 - PASS_OVERLAP):.3f}" for k in range(N_PASSES)) + " mm\n"
               f"  total path length: " + ", ".join(f"pass {k + 1} {p.length:.0f} mm" for k, p in enumerate(passes)) + "\n"
               f"  simulated cut: {'every net isolated from every other' if not bad else 'NOT ISOLATED: ' + str(bad)}; "
               f"{'no copper of any net removed' if not eaten else 'COPPER CUT on: ' + str(eaten)}\n"
               f"  board height error +/-{Z_TOL:.2f} mm (no height map): at {shallow:.2f} mm deep the cut is "
               f"{cut_width(shallow):.3f} mm wide and "
               f"{'still isolates every net' if not bad_shallow else 'FAILS: ' + str(bad_shallow)}; "
               f"at {CUT_DEPTH + Z_TOL:.2f} mm it is {cut_width(CUT_DEPTH + Z_TOL):.3f} mm wide and trims "
               f"{bite:.3f} mm off copper edges (harmless)\n")
    if bad or eaten or bad_shallow:
        print(report)
        raise SystemExit("isolation check failed")
    os.makedirs(HERE, exist_ok=True)
    with open(os.path.join(HERE, "milling_report.txt"), "w") as f:
        f.write(report)
    print(report)
    preview_passes(nets, holes, passes)
    write_dxf(os.path.join(HERE, "cloudline_pcb_MILL_copper_up.dxf"), nets, holes, mirror_up, passes)
    write_step(os.path.join(HERE, "cloudline_pcb_MILL_copper_up.step"), prims, holes,
               lambda x, y: (W - x, H - y), "cloudline_pcb_MILL_copper_up", copper_up=True)
    write_step(os.path.join(HERE, "cloudline_pcb_TOPVIEW.step"), prims, holes,
               lambda x, y: (x, H - y), "cloudline_pcb_TOPVIEW", copper_up=False)
    preview(nets, holes)
    print("wrote", HERE)


def preview(nets, holes):
    import matplotlib
    matplotlib.use("Agg")
    import matplotlib.pyplot as plt
    from matplotlib.patches import Polygon as MPoly
    fig, ax = plt.subplots(figsize=(10, 7.5), dpi=150)
    ax.add_patch(MPoly([(0, 0), (W, 0), (W, H), (0, H)], closed=True, fc="#d8c79a", ec="k"))
    for geom in nets.values():
        for p in polys(mirror_up(geom)):
            ax.add_patch(MPoly(list(p.exterior.coords), closed=True, fc="#c77d3a", ec="#5a3410", lw=0.4))
            for ring in p.interiors:
                ax.add_patch(MPoly(list(ring.coords), closed=True, fc="white", ec="#5a3410", lw=0.4))
    for x, y, d, _ in holes:
        pt = mirror_up(Point(x, y))
        ax.add_patch(plt.Circle((pt.x, pt.y), d / 2, fc="white", ec="k", lw=0.3))
    ax.annotate("origin (0,0)", (0, 0), (3, -5), fontsize=8, arrowprops={"arrowstyle": "->"})
    ax.set_xlim(-8, W + 3)
    ax.set_ylim(-8, H + 3)
    ax.set_aspect("equal")
    ax.set_title("MILL SIDE: copper up, mirrored (as on the machine bed). X right, Y up, mm", fontsize=10)
    ax.text(W - 2, -5, "Nano USB end is on the RIGHT on this side", ha="right", fontsize=8)
    fig.tight_layout()
    fig.savefig(os.path.join(HERE, "mill_preview.png"))


def preview_passes(nets, holes, passes):
    import matplotlib
    matplotlib.use("Agg")
    import matplotlib.pyplot as plt
    from matplotlib.patches import Polygon as MPoly
    fig, ax = plt.subplots(figsize=(10, 7.5), dpi=150)
    ax.add_patch(MPoly([(0, 0), (W, 0), (W, H), (0, H)], closed=True, fc="#f4efe0", ec="k"))
    for geom in nets.values():
        for p in polys(mirror_up(geom)):
            ax.add_patch(MPoly(list(p.exterior.coords), closed=True, fc="#e3b07a", ec="none"))
    colours = ["#d01c1c", "#1c4fd0"]
    for k, pas in enumerate(passes):
        for g in getattr(mirror_up(pas), "geoms", [mirror_up(pas)]):
            if not g.is_empty:
                x, y = g.xy
                ax.plot(x, y, color=colours[k % 2], lw=0.5)
    ax.set_xlim(-3, W + 3)
    ax.set_ylim(-3, H + 3)
    ax.set_aspect("equal")
    ax.set_title("Isolation tool paths (mill side): red = ISO_PASS1, blue = ISO_PASS2", fontsize=10)
    fig.tight_layout()
    fig.savefig(os.path.join(HERE, "isolation_preview.png"))


if __name__ == "__main__":
    sys.exit(main())
