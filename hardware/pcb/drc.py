#!/usr/bin/env python3
"""Run KiCad's DRC on the board and summarise the report.

    python3 hardware/pcb/drc.py [report.txt]
"""
import os
import re
import sys

# Headless KiCad doesn't set the library path its default tables refer to.
os.environ.setdefault("KICAD7_FOOTPRINT_DIR", "/usr/share/kicad/footprints")
os.environ.setdefault("KICAD7_SYMBOL_DIR", "/usr/share/kicad/symbols")
os.environ.setdefault("KIPRJMOD", os.path.dirname(os.path.abspath(__file__)))

import pcbnew  # noqa: E402

HERE = os.path.dirname(os.path.abspath(__file__))
board = pcbnew.LoadBoard(os.path.join(HERE, "cloudline_controller.kicad_pcb"))
out = sys.argv[1] if len(sys.argv) > 1 else os.path.join(HERE, "drc_report.txt")
ok = pcbnew.WriteDRCReport(board, out, pcbnew.EDA_UNITS_MILLIMETRES, True)
text = open(out).read()
errors = re.findall(r"^\[(\w+)\]: .*\n\s+(?:Rule: .*; )?(error|warning)", text, re.M)
print(text.split("\n** Found")[0].split("\n")[0] if text else "no report")
for line in text.splitlines():
    if line.startswith("** Found"):
        print(line)
sys.exit(0 if ok else 1)
