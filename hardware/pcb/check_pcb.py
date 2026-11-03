#!/usr/bin/env python3
"""Check that the routed copper of the PCB realises the schematic.

    python3 hardware/pcb/check_pcb.py

Connectivity is computed from the copper geometry itself (pad and track
shapes that touch, plus the wire links on the jumper layer), NOT from the net
names stored in the board, so a routing mistake shows up as an open or a
short. The expected netlist below is transcribed from PINOUT.md section 6
(point-to-point list), with the PCB-specific connector names:

  fan cable J1 -> screw terminal J2 (1 +10V, 2 TACH, 3 GND, 4 PWM)
  OLED module U3 -> J3 (BAK, CON, PSH, TRB, TRA) and J4 (SDA, SCL, VCC, GND)
  only one GND pin each of the Nano (29) and of U4 (GND_LV) is wired; the
  modules join their own GND pins.

Pin numbers are footprint pad numbers: diodes 1 = cathode, 2 = anode;
TO-92 2N7000 1 S, 2 G, 3 D; MCP1700 1 GND, 2 VIN, 3 VOUT; CP 1 = +.
"""

import os
import sys

import pcbnew

HERE = os.path.dirname(os.path.abspath(__file__))

EXPECTED = {
    "+10V_FAN": ["J2.1", "F1.1", "R5.2"],
    "N_F1_D1": ["F1.2", "D1.2"],
    "+10V_P": ["D1.1", "U1.30", "C1.1", "C2.1", "D2.1"],
    "GND": ["J2.3", "U1.29", "C1.2", "C2.2", "D2.2", "Q1.1", "D4.2", "C5.2", "C3.2", "C4.2",
            "U2.1", "J4.4", "U4.GND_LV"],
    "FAN_PWM": ["J2.4", "Q1.3", "R5.1"],
    "PWM_D9": ["U1.12", "R3.1", "R4.1"],
    "GATE": ["R3.2", "Q1.2"],
    "TACH_IN": ["J2.2", "R7.2"],
    "TACH_5V": ["R7.1", "U1.5", "R6.1", "D3.2", "D4.1", "C5.1"],
    "+5V": ["U1.27", "U2.2", "C3.1", "R4.2", "R6.2", "D3.1", "U4.HV"],
    "+3V3": ["U2.3", "C4.1", "R1.2", "R2.2", "U4.LV", "J4.3"],
    "SDA_5V": ["U1.23", "U4.HV1"],
    "SCL_5V": ["U1.24", "U4.HV2"],
    "SDA_3V3": ["U4.LV1", "R1.1", "J4.1"],
    "SCL_3V3": ["U4.LV2", "R2.1", "J4.2"],
    "ENC_A": ["U1.6", "J3.5"],
    "ENC_B": ["U1.7", "J3.4"],
    "ENC_SW": ["U1.8", "J3.3"],
    "KEY_CON": ["U1.9", "J3.2"],
    "KEY_BAK": ["U1.10", "J3.1"],
}


def main():
    path = sys.argv[1] if len(sys.argv) > 1 else os.path.join(HERE, "cloudline_controller.kicad_pcb")
    board = pcbnew.LoadBoard(path)
    items, names = [], []
    for fp in board.GetFootprints():
        for p in fp.Pads():
            if p.GetAttribute() == pcbnew.PAD_ATTRIB_NPTH:
                continue
            items.append(p)
            names.append(f"{fp.GetReference()}.{p.GetNumber()}")
    for t in board.GetTracks():
        items.append(t)
        names.append(None)

    parent = list(range(len(items)))

    def find(i):
        while parent[i] != i:
            parent[i] = parent[parent[i]]
            i = parent[i]
        return i

    layers = (pcbnew.B_Cu, pcbnew.F_Cu)
    shapes = []
    for it in items:
        per = {}
        for L in layers:
            if it.IsOnLayer(L):
                per[L] = it.GetEffectiveShape(L)
        shapes.append(per)
    boxes = [it.GetBoundingBox() for it in items]
    for i in range(len(items)):
        for j in range(i + 1, len(items)):
            if not boxes[i].Intersects(boxes[j]):
                continue
            for L, si in shapes[i].items():
                sj = shapes[j].get(L)
                if sj is not None and si.Collide(sj, 0):
                    parent[find(i)] = find(j)
                    break

    clusters = {}
    for i, n in enumerate(names):
        if n:
            clusters.setdefault(find(i), set()).add(n)

    problems = 0
    expected_pads = {p: net for net, pads in EXPECTED.items() for p in pads}
    missing = set(expected_pads) - set(n for n in names if n)
    for p in sorted(missing):
        print(f"MISSING PAD  {p}")
        problems += 1
    for net, pads in EXPECTED.items():
        roots = {find(names.index(p)) for p in pads if p in names}
        status = "ok" if len(roots) == 1 else f"OPEN ({len(roots)} pieces)"
        if len(roots) != 1:
            problems += 1
        print(f"{net:9s} {status:16s} {' '.join(pads)}")
    for members in clusters.values():
        nets = {expected_pads[m] for m in members if m in expected_pads}
        if len(nets) > 1:
            print(f"SHORT between {sorted(nets)}: {sorted(members)}")
            problems += 1
        extra = [m for m in members if m not in expected_pads and not m.startswith("W")]
        if extra and nets:
            print(f"UNEXPECTED pads on {sorted(nets)}: {extra}")
            problems += 1
    # Stored net names must agree with the copper too (Gerbers carry them).
    for i, it in enumerate(items):
        n = names[i]
        if n in expected_pads and it.GetNetname() != expected_pads[n]:
            print(f"NET NAME {n}: board says {it.GetNetname()}, expected {expected_pads[n]}")
            problems += 1
    print(f"\npcb netlist check: {len(EXPECTED)} nets, "
          f"{sum(len(v) for v in EXPECTED.values())} pads, {problems} problems")
    sys.exit(1 if problems else 0)


if __name__ == "__main__":
    main()
