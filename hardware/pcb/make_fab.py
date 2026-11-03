#!/usr/bin/env python3
"""Produce the manufacturing package for the single-sided PCB.

    python3 hardware/pcb/make_fab.py     (needs KiCad 7: kicad-cli and pcbnew;
                                          gerbv for the check renders)

Steps:
  1. DRC (must report 0 errors and 0 unconnected) and the copper netlist check.
  2. Gerbers for the ONE copper layer and the layers a single-sided board needs:
     B.Cu (copper), B.Mask (solder mask, copper side), F.SilkS (top legend),
     Edge.Cuts (outline). F.Cu is KiCad's "jumper" layer holding the wire
     links and is deliberately NOT exported.
  3. Excellon drill file (mm) plus a drill map.
  4. A zip of 2 + 3 for upload to a board house.
  5. Assembly drawing (PDF), bottom-copper artwork at 1:1 (PDF, for
     home etching by toner transfer), PCB bill of materials (CSV), board renders (PNG), and
     renders of the Gerbers themselves made with gerbv (an independent
     Gerber viewer) to check what the fab will see.
"""

import csv
import glob
import os
import shutil
import subprocess
import sys
import zipfile

HERE = os.path.dirname(os.path.abspath(__file__))
PCB = os.path.join(HERE, "cloudline_controller.kicad_pcb")
FAB = os.path.join(HERE, "fab")
GERB = os.path.join(FAB, "gerbers")
ZIPNAME = os.path.join(FAB, "cloudline_controller_RevB_gerbers_drill.zip")
REV = "B"


def run(cmd, quiet=False, **kw):
    print("$", " ".join(cmd))
    subprocess.run(cmd, check=True, stderr=subprocess.DEVNULL if quiet else None, **kw)


def svg_png(svg, png, width):
    run(["rsvg-convert", "-w", str(width), "-b", "white", svg, "-o", png])


def main():
    env = dict(os.environ)
    env.setdefault("KICAD7_FOOTPRINT_DIR", "/usr/share/kicad/footprints")
    env.setdefault("KIPRJMOD", HERE)

    # 1. checks
    drc_txt = os.path.join(FAB, "drc_report.txt")
    os.makedirs(FAB, exist_ok=True)
    run([sys.executable, os.path.join(HERE, "drc.py"), drc_txt], env=env)
    report = open(drc_txt).read()
    if "Severity: error" in report or "** Found 0 unconnected pads **" not in report:
        sys.exit("DRC has errors or unconnected pads; see " + drc_txt)
    run([sys.executable, os.path.join(HERE, "check_pcb.py")], env=env)

    # 2./3. gerbers and drill
    if os.path.isdir(GERB):
        shutil.rmtree(GERB)
    os.makedirs(GERB)
    # Plain RS-274X (no X2 attributes) for the widest board-house compatibility.
    run(["kicad-cli", "pcb", "export", "gerbers", "-l", "B.Cu,B.Mask,F.SilkS,Edge.Cuts",
         "--ev", "--no-x2", "-o", GERB + "/", PCB], env=env)
    run(["kicad-cli", "pcb", "export", "drill", "--format", "excellon", "-u", "mm",
         "--generate-map", "--map-format", "pdf", "-o", GERB + "/", PCB], env=env)
    produced = sorted(os.listdir(GERB))
    print("gerber/drill files:", produced)
    for unwanted in ("-F_Cu.", "-F_Mask."):
        assert not any(unwanted in f for f in produced), unwanted

    # 4. zip
    with zipfile.ZipFile(ZIPNAME, "w", zipfile.ZIP_DEFLATED) as z:
        for f in produced:
            # The .gbrjob would describe 2 copper layers (F.Cu is KiCad's
            # jumper layer), which could mislead a fab's auto-detection.
            if not f.endswith((".pdf", ".gbrjob")):
                z.write(os.path.join(GERB, f), f)
    print("wrote", ZIPNAME)

    # 5a. assembly drawing and home-etch artwork
    run(["kicad-cli", "pcb", "export", "pdf", "-l", "F.Fab,F.SilkS,F.Cu,Edge.Cuts",
         "--ibt", "--black-and-white", "-o", os.path.join(FAB, "assembly_top.pdf"), PCB], env=env)
    # Toner transfer: the bottom layer is printed as seen from the TOP (not
    # mirrored); laying the print face-down on the copper mirrors it.
    run(["kicad-cli", "pcb", "export", "pdf", "-l", "B.Cu,Edge.Cuts",
         "--black-and-white", "-o", os.path.join(FAB, "bottom_copper_toner_transfer_1to1.pdf"), PCB], env=env)

    # 5b. renders (KiCad's view)
    tmp = os.path.join(FAB, "tmp")
    os.makedirs(tmp, exist_ok=True)
    run(["kicad-cli", "pcb", "export", "svg", "-l", "F.SilkS,F.Cu,B.Cu,Edge.Cuts",
         "--page-size-mode", "2", "--exclude-drawing-sheet", "-o", os.path.join(tmp, "top.svg"), PCB], env=env)
    run(["kicad-cli", "pcb", "export", "svg", "-l", "B.Cu,B.Mask,Edge.Cuts", "--mirror",
         "--page-size-mode", "2", "--exclude-drawing-sheet", "-o", os.path.join(tmp, "bottom.svg"), PCB], env=env)
    svg_png(os.path.join(tmp, "top.svg"), os.path.join(FAB, "render_top.png"), 2000)
    svg_png(os.path.join(tmp, "bottom.svg"), os.path.join(FAB, "render_bottom_mirrored.png"), 2000)

    # 5c. gerbv renders of the actual Gerber/drill files (what the fab sees)
    g = {k: glob.glob(os.path.join(GERB, "*" + k))[0] for k in ("B_Cu.gbl", "B_Mask.gbs", "F_Silkscreen.gto",
                                                              "Edge_Cuts.gm1")}
    drl = glob.glob(os.path.join(GERB, "*.drl"))
    if shutil.which("gerbv"):
        run(["gerbv", "-x", "png", "--dpi=400", "-a", "-b", "#ffffff",
             "-o", os.path.join(FAB, "gerbv_copper_bottom.png"),
             "-f", "#b87333", g["B_Cu.gbl"], "-f", "#000000", g["Edge_Cuts.gm1"],
             *sum((["-f", "#303030", d] for d in drl), [])], quiet=True)
        run(["gerbv", "-x", "png", "--dpi=400", "-a", "-b", "#ffffff",
             "-o", os.path.join(FAB, "gerbv_silk_top.png"),
             "-f", "#1a4fa0", g["F_Silkscreen.gto"], "-f", "#000000", g["Edge_Cuts.gm1"],
             *sum((["-f", "#a0a0a0", d] for d in drl), [])], quiet=True)
    shutil.rmtree(tmp)

    # 5d. PCB bill of materials (from the board itself)
    import pcbnew
    board = pcbnew.LoadBoard(PCB)
    groups = {}
    for fp in board.GetFootprints():
        ref = fp.GetReference()
        if ref.startswith("H"):
            continue
        key = (fp.GetValue(), fp.GetFPID().GetUniStringLibId())
        groups.setdefault(key, []).append(ref)

    def refkey(r):
        head = r.rstrip("0123456789")
        return head, int(r[len(head):] or 0)

    rows = sorted(groups.items(), key=lambda kv: refkey(sorted(kv[1], key=refkey)[0]))
    with open(os.path.join(FAB, "pcb_bom.csv"), "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["Refs", "Qty", "Value", "Footprint", "Notes"])
        notes = {
            "U1": "Plug into two 1x15 female headers (2.54 mm); see hardware/bom.csv",
            "U4": "SparkFun BOB-12009 on its 1x6 male headers, HV row towards the Nano",
            "W1": "Bare tinned copper wire ~0.6 mm, on the TOP side",
            "R5": "Do not fit unless README check M3 says so",
            "J2": "Phoenix MKDS 1,5/4-5,08 (1715747) or any 5.08 mm 4-way screw terminal",
            "J3": "1x5 male pin header 2.54 mm", "J4": "1x4 male pin header 2.54 mm",
        }
        for (value, fpid), refs in rows:
            refs = sorted(refs, key=refkey)
            note = "; ".join(notes[r] for r in refs if r in notes)
            w.writerow([" ".join(refs), len(refs), value, fpid, note])
    print("wrote pcb_bom.csv")


if __name__ == "__main__":
    main()
