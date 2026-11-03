#!/usr/bin/env python3
"""Read the exported STEP files back and check them.

    python3 hardware/pcb/fusion/check_fusion.py

  - every body is a valid solid; one substrate plus one copper body per net
  - substrate 100 x 75 x 1.6 mm, with the right number of holes
  - copper volume per net equals the area of the reference (shapely)
    geometry built from the KiCad board, times the copper thickness
  - the MILL file is mirrored: the Nano's VIN pad (top view 41.06, 45.24)
    lands at (W - x, H - y) on top of the substrate; TOPVIEW keeps (x, H - y)
"""
import os
import sys

import build123d as bd

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import make_fusion as mf  # noqa: E402

HERE = os.path.dirname(os.path.abspath(__file__))
problems = 0


def check(cond, msg):
    global problems
    print(("ok    " if cond else "FAIL  ") + msg)
    problems += 0 if cond else 1


nets, holes, prims = mf.collect()
ref_area = {n: g.area for n, g in nets.items()}
for name, xf, zc in (("cloudline_pcb_MILL_copper_up.step", lambda x, y: (mf.W - x, mf.H - y), "up"),
                     ("cloudline_pcb_TOPVIEW.step", lambda x, y: (x, mf.H - y), "down")):
    shape = bd.import_step(os.path.join(HERE, name))
    solids = shape.solids()
    check(all(s.is_valid for s in solids), f"{name}: {len(solids)} solids, all valid")
    sub = max(solids, key=lambda s: s.volume)
    bb = sub.bounding_box()
    check(abs(bb.size.X - mf.W) < 1e-3 and abs(bb.size.Y - mf.H) < 1e-3 and abs(bb.size.Z - mf.T_BOARD) < 1e-3,
          f"{name}: substrate {bb.size.X:.2f} x {bb.size.Y:.2f} x {bb.size.Z:.3f} mm at origin "
          f"({bb.min.X:.2f}, {bb.min.Y:.2f}, {bb.min.Z:.2f})")
    hole_vol = sum(3.141592653589793 * (d / 2) ** 2 * mf.T_BOARD for *_, d, _ in [(h[0], h[1], h[2], h[3]) for h in holes])
    check(abs(mf.W * mf.H * mf.T_BOARD - hole_vol - sub.volume) < 0.05,
          f"{name}: substrate volume matches {len(holes)} through-holes")
    copper = [s for s in solids if s is not sub]
    check(len(copper) == len(nets), f"{name}: {len(copper)} copper bodies for {len(nets)} nets")
    cu_vol = sum(s.volume for s in copper)
    ref_vol = sum(ref_area.values()) * mf.T_COPPER
    check(abs(cu_vol - ref_vol) / ref_vol < 0.005,
          f"{name}: copper volume {cu_vol:.3f} mm3 vs reference {ref_vol:.3f} mm3")
    zs = [s.bounding_box() for s in copper]
    if zc == "up":
        check(all(abs(b.min.Z - mf.T_BOARD) < 1e-6 for b in zs), f"{name}: copper sits on top (Z {mf.T_BOARD} .. {mf.T_BOARD + mf.T_COPPER})")
    else:
        check(all(abs(b.max.Z) < 1e-6 for b in zs), f"{name}: copper under the board (Z -{mf.T_COPPER} .. 0)")
    vx, vy = xf(41.06, 45.24)
    z = mf.T_BOARD + mf.T_COPPER / 2 if zc == "up" else -mf.T_COPPER / 2
    p = bd.Vector(vx + 0.7, vy, z)  # inside the VIN pad ring, beside the hole
    hit = [s for s in copper if s.is_inside(p)]
    check(len(hit) == 1, f"{name}: Nano VIN pad found at ({vx:.2f}, {vy:.2f}) as expected")
print(f"\nfusion export check: {problems} problems")
sys.exit(1 if problems else 0)
