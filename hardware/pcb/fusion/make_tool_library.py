#!/usr/bin/env python3
"""Write a Fusion 360 tool library (JSON) for milling this PCB on the
Stepcraft M.1000 with the DI 1000 spindle (8 000-20 000 rpm).

    python3 hardware/pcb/fusion/make_tool_library.py

Import in Fusion: Manufacture -> Tool Library -> right-click a local library
folder -> Import tool libraries -> cloudline_pcb_tools.json.

The tool numbers (T1..T7) are the tool-changer slots the posted program asks
for. Change them in Fusion, or in TOOLS below, to match your rack. Speeds are
starting values for carbide tools in FR-4; nothing exceeds the spindle's
20 000 rpm, and nothing goes under its 8 000 rpm.
"""

import json
import math
import os
import uuid

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(HERE, "cloudline_pcb_tools.json")
NS = uuid.UUID("6f1c8a52-1d0e-4c55-9a43-2a7d6c1b9e01")
SHANK = 3.175  # 1/8" shank, standard for PCB drills, V-bits and small end mills

# number, type, description, geometry, rpm, feed, plunge, stepdown
TOOLS = [
    (1, "chamfer mill", "V-bit 30 deg, 0.1 mm tip - isolation",
     {"DC": SHANK, "tip-diameter": 0.1, "TA": 15.0, "NOF": 1}, 20000, 400, 100, 0.20),
    (2, "drill", "Drill 0.8 mm - 0.75/0.80 holes", {"DC": 0.8}, 20000, 100, 100, None),
    (3, "drill", "Drill 1.0 mm", {"DC": 1.0}, 20000, 120, 120, None),
    (4, "drill", "Drill 1.2 mm - 1.10/1.20 holes", {"DC": 1.2}, 20000, 140, 140, None),
    (5, "drill", "Drill 1.3 mm - J2 screw terminal", {"DC": 1.3}, 20000, 140, 140, None),
    (6, "drill", "Drill 3.2 mm - M3 mounting holes", {"DC": 3.2}, 12000, 200, 200, None),
    (7, "flat end mill", "End mill 2 mm 2-flute - board outline", {"DC": 2.0, "NOF": 2}, 18000, 500, 100, 0.5),
]


def tool(num, kind, desc, geo, rpm, feed, plunge, stepdown):
    dc = geo["DC"]
    g = {"DC": dc, "SFDM": SHANK if dc <= SHANK else dc, "OAL": 38.0, "NOF": geo.get("NOF", 2),
         "HAND": True, "CSP": False, "RE": 0.0, "NT": 1}
    if kind == "drill":
        g.update({"SIG": 118.0, "LCF": max(6.0, 8 * dc), "LB": 15.0})
    elif kind == "chamfer mill":
        g.update({"tip-diameter": geo["tip-diameter"], "TA": geo["TA"],
                  "LCF": round((dc - geo["tip-diameter"]) / (2 * math.tan(math.radians(geo["TA"]))), 3),
                  "LB": 15.0})
    else:
        g.update({"LCF": 8.0, "LB": 15.0, "shoulder-length": 8.0})
    preset = {
        "guid": "{%s}" % uuid.uuid5(NS, f"preset{num}"),
        "name": "FR-4 PCB (Stepcraft DI 1000)",
        "description": "",
        "n": rpm, "n_ramp": rpm,
        "v_f": feed, "v_f_leadIn": feed, "v_f_leadOut": feed, "v_f_transition": feed,
        "v_f_ramp": plunge, "v_f_plunge": plunge, "v_f_retract": plunge * 2,
        "f_z": round(feed / rpm / g["NOF"], 5), "f_n": round(feed / rpm, 5),
        "v_c": round(math.pi * dc * rpm / 1000, 2),
        "tool-coolant": "disabled",
    }
    if stepdown:
        preset.update({"use-stepdown": True, "stepdown": stepdown})
    return {
        "type": kind, "unit": "millimeters", "description": desc,
        "guid": "{%s}" % uuid.uuid5(NS, f"tool{num}"),
        "vendor": "", "product-id": "", "product-link": "",
        "BMC": "carbide", "GRADE": "Mill Generic",
        "geometry": g,
        "post-process": {"number": num, "length-offset": num, "diameter-offset": num, "turret": 0,
                         "comment": desc, "manual-tool-change": False, "break-control": False, "live": True},
        "start-values": {"presets": [preset]},
    }


def main():
    lib = {"data": [tool(*t) for t in TOOLS], "version": 15}
    for t in TOOLS:
        assert 8000 <= t[4] <= 20000, f"T{t[0]}: rpm outside the DI 1000 range"
    with open(OUT, "w") as f:
        json.dump(lib, f, indent=2)
    print("wrote", OUT)
    for num, kind, desc, geo, rpm, feed, plunge, sd in TOOLS:
        print(f"  T{num}  {desc:42s} {rpm:6d} rpm  feed {feed:4d}  plunge {plunge:4d}"
              + (f"  step-down {sd} mm" if sd else ""))


if __name__ == "__main__":
    main()
