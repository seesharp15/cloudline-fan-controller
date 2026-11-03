#!/usr/bin/env python3
"""Check the EAGLE export (make_eagle.py) against the KiCad board.

    python3 hardware/pcb/eagle/check_eagle.py     (needs KiCad 7's pcbnew, lxml)

1. Both files validate against EAGLE's DTD (eagle.dtd, EAGLE 9.1.3).
2. The .brd is read back with KiCad's own EAGLE importer:
   - every pad has the same position (±0.01 mm), size, drill and net as on the KiCad board;
   - the track segments are the same;
   - KiCad reports 0 unconnected pads;
   - ../check_pcb.py (copper geometry against PINOUT.md) passes on it.
3. The .sch is checked geometrically:
   - every net wire starts on the pin it references, at that pin's real position;
   - every label sits on its wire;
   - the device pin -> pad map gives exactly the board's nets.
"""

import math
import os
import subprocess
import sys
import tempfile

import pcbnew
from lxml import etree

HERE = os.path.dirname(os.path.abspath(__file__))
PCB = os.path.join(HERE, "..", "cloudline_controller.kicad_pcb")
BRD = os.path.join(HERE, "cloudline_controller.brd")
SCH = os.path.join(HERE, "cloudline_controller.sch")
DTD = os.path.join(HERE, "eagle.dtd")
W, H, OX, OY = 100.0, 75.0, 50.0, 40.0

problems = []


def bad(msg):
    problems.append(msg)
    print("PROBLEM:", msg)


def pads_of(board, shift):
    out = {}
    for fp in board.GetFootprints():
        for p in fp.Pads():
            if p.GetAttribute() == pcbnew.PAD_ATTRIB_NPTH or not p.GetNumber():
                continue
            c = p.GetPosition()
            x, y = pcbnew.ToMM(c.x) - shift[0], pcbnew.ToMM(c.y) - shift[1]
            sz = min(pcbnew.ToMM(p.GetSize().x), pcbnew.ToMM(p.GetSize().y))
            out[(fp.GetReference(), p.GetNumber())] = (x, y, sz, pcbnew.ToMM(p.GetDrillSize().x),
                                                       p.GetNetname())
    return out


def segs_of(board, shift):
    s = set()
    for t in board.GetTracks():
        a = (round(pcbnew.ToMM(t.GetStart().x) - shift[0], 2), round(pcbnew.ToMM(t.GetStart().y) - shift[1], 2))
        b = (round(pcbnew.ToMM(t.GetEnd().x) - shift[0], 2), round(pcbnew.ToMM(t.GetEnd().y) - shift[1], 2))
        s.add((min(a, b), max(a, b), round(pcbnew.ToMM(t.GetWidth()), 2), t.GetNetname()))
    return s


def outline_origin(board):
    xs, ys = [], []
    for d in board.GetDrawings():
        if d.GetLayer() == pcbnew.Edge_Cuts:
            for p in (d.GetStart(), d.GetEnd()):
                xs.append(pcbnew.ToMM(p.x))
                ys.append(pcbnew.ToMM(p.y))
    return min(xs), min(ys)


def check_dtd():
    dtd = etree.DTD(open(DTD))
    for fn in (BRD, SCH):
        if not dtd.validate(etree.parse(fn)):
            bad(f"{os.path.basename(fn)} fails the EAGLE DTD: {dtd.error_log.filter_from_errors()[:3]}")
    print("DTD: checked both files")


def check_board():
    ref = pcbnew.LoadBoard(PCB)
    imp = pcbnew.IO_MGR.Load(pcbnew.IO_MGR.EAGLE, BRD)
    o_ref, o_imp = outline_origin(ref), outline_origin(imp)
    pa, pb = pads_of(ref, o_ref), pads_of(imp, o_imp)
    if set(pa) != set(pb):
        bad(f"pad sets differ: only KiCad {sorted(set(pa) - set(pb))[:5]}, only EAGLE {sorted(set(pb) - set(pa))[:5]}")
    worst = 0.0
    for k in set(pa) & set(pb):
        (xa, ya, sa, da, na), (xb, yb, sb, db, nb) = pa[k], pb[k]
        d = math.hypot(xa - xb, ya - yb)
        worst = max(worst, d)
        if d > 0.01:
            bad(f"{k}: pad moved {d:.3f} mm")
        if abs(sa - sb) > 0.01 or abs(da - db) > 0.01:
            bad(f"{k}: pad {sa}/{da} became {sb}/{db}")
        if na != nb:
            bad(f"{k}: net {na!r} became {nb!r}")
    print(f"board: {len(pb)} pads compared, worst position error {worst:.4f} mm")
    sa, sb = segs_of(ref, o_ref), segs_of(imp, o_imp)
    if sa != sb:
        bad(f"track segments differ: {len(sa - sb)} only in KiCad, {len(sb - sa)} only in EAGLE")
    print(f"board: {len(sb)} track segments, identical: {sa == sb}")
    if any(t.GetLayer() != pcbnew.B_Cu for t in imp.GetTracks()):
        bad("imported tracks not all on the bottom layer")
    imp.BuildConnectivity()
    unc = imp.GetConnectivity().GetUnconnectedCount(False)
    print(f"board: KiCad's connectivity on the imported board: {unc} unconnected")
    if unc:
        bad(f"{unc} unconnected items after import")
    # check_pcb.py expects the KiCad page offset; move the imported board there
    imp.Move(pcbnew.VECTOR2I(pcbnew.FromMM(OX - o_imp[0]), pcbnew.FromMM(OY - o_imp[1])))
    with tempfile.TemporaryDirectory() as tmp:
        path = os.path.join(tmp, "imported.kicad_pcb")
        pcbnew.SaveBoard(path, imp)
        r = subprocess.run([sys.executable, os.path.join(HERE, "..", "check_pcb.py"), path],
                           capture_output=True, text=True)
        last = (r.stdout.strip().splitlines() or ["(no output)"])[-1]
        print("board: check_pcb.py on the imported board:", last)
        if r.returncode or "0 problems" not in last:
            bad("check_pcb.py fails on the imported board:\n" + r.stdout[-2000:] + r.stderr[-2000:])
    return {k: v[4] for k, v in pa.items()}


def check_schematic(board_nets):
    root = etree.parse(SCH).getroot()
    lib = root.find(".//schematic/libraries/library")
    syms = {s.get("name"): {p.get("name"): (float(p.get("x")), float(p.get("y"))) for p in s.iter("pin")}
            for s in lib.iter("symbol")}
    dsets = {}
    for ds in lib.iter("deviceset"):
        gate = ds.find("gates/gate")
        conn = {c.get("pin"): c.get("pad") for c in ds.iter("connect")}
        dsets[ds.get("name")] = (gate.get("symbol"), conn)
        pins = set(syms[gate.get("symbol")])
        if set(conn) != pins:
            bad(f"{ds.get('name')}: connects {sorted(set(conn) ^ pins)} not matching symbol pins")
    parts = {p.get("name"): p.get("deviceset") for p in root.iter("part")}
    inst = {i.get("part"): (float(i.get("x")), float(i.get("y"))) for i in root.iter("instance")}
    sch_nets = {}
    n_seg = 0
    for net in root.iter("net"):
        name = net.get("name")
        for seg in net.iter("segment"):
            n_seg += 1
            pr = seg.find("pinref")
            wires = seg.findall("wire")
            lab = seg.find("label")
            part, pin = pr.get("part"), pr.get("pin")
            sym, conn = dsets[parts[part]]
            px, py = syms[sym][pin]
            ix, iy = inst[part]
            ends = [(float(w.get("x1")), float(w.get("y1")), float(w.get("x2")), float(w.get("y2"))) for w in wires]
            if not any(math.isclose(x1, ix + px, abs_tol=1e-3) and math.isclose(y1, iy + py, abs_tol=1e-3)
                       for x1, y1, _, _ in ends):
                bad(f"net {name}: wire does not start on {part}.{pin}")
            if lab is None or not any(math.isclose(float(lab.get("x")), x2, abs_tol=1e-3) and
                                      math.isclose(float(lab.get("y")), y2, abs_tol=1e-3) for _, _, x2, y2 in ends):
                bad(f"net {name}: label not on its wire at {part}.{pin}")
            if (ix + px) % 2.54 > 1e-3 and 2.54 - (ix + px) % 2.54 > 1e-3:
                bad(f"{part}.{pin} off the 0.1 inch grid")
            sch_nets[(part, conn[pin])] = name
    # any two pin ends at the same spot would join nets silently
    spots = {}
    for part, (ix, iy) in inst.items():
        sym, _ = dsets[parts[part]]
        for pin, (px, py) in syms[sym].items():
            k = (round(ix + px, 3), round(iy + py, 3))
            if k in spots:
                bad(f"{part}.{pin} and {spots[k]} share a point")
            spots[k] = f"{part}.{pin}"
    want = {k: v for k, v in board_nets.items() if v}
    if sch_nets != want:
        diff = set(sch_nets.items()) ^ set(want.items())
        bad(f"schematic netlist differs from the board: {sorted(diff)[:6]}")
    print(f"schematic: {len(parts)} parts, {len(set(sch_nets.values()))} nets, {n_seg} pin connections, "
          f"same netlist as the board: {sch_nets == want}")


def main():
    check_dtd()
    nets = check_board()
    check_schematic(nets)
    print(f"eagle export check: {len(problems)} problems")
    sys.exit(1 if problems else 0)


if __name__ == "__main__":
    main()
