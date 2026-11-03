#!/usr/bin/env python3
"""Export the KiCad board as an EAGLE 9 schematic + board pair, so that it
opens as an editable, linked electronics design in Fusion 360.

    python3 hardware/pcb/eagle/make_eagle.py     (needs KiCad 7's pcbnew)

KiCad 7 can read EAGLE files but not write them. This script writes them
from the KiCad board:

  cloudline_controller.brd   board: outline, mounting holes, every part at
                             its KiCad position, every track on the Bottom
                             layer (16), silkscreen, design rules.
  cloudline_controller.sch   schematic with the same parts and nets
                             (connected by net labels), so Fusion keeps the
                             schematic and the board in sync.

Both embed one library, "cloudline", made from the KiCad footprints. The
nets come from the board's pads, which check_pcb.py has already verified
against PINOUT.md. check_eagle.py verifies the output.
"""

import math
import os
import sys
from xml.sax.saxutils import escape, quoteattr

import pcbnew

HERE = os.path.dirname(os.path.abspath(__file__))
PCB = os.path.join(HERE, "..", "cloudline_controller.kicad_pcb")
OUT_BRD = os.path.join(HERE, "cloudline_controller.brd")
OUT_SCH = os.path.join(HERE, "cloudline_controller.sch")

W, H = 100.0, 75.0
OX, OY = 50.0, 40.0
LIB = "cloudline"
POWER_NETS = {"GND", "+5V", "+10V_FAN", "+10V_P", "N_F1_D1"}

# Pin names (schematic) for each pad, where the pad number alone says little.
NANO = ["D1/TX", "D0/RX", "RESET@1", "GND@1", "D2", "D3", "D4", "D5", "D6", "D7",
        "D8", "D9", "D10", "D11", "D12", "D13", "3V3", "AREF", "A0", "A1", "A2",
        "A3", "A4/SDA", "A5/SCL", "A6", "A7", "+5V", "RESET@2", "GND@2", "VIN"]
PIN_NAMES = {
    "U1": {str(i + 1): n for i, n in enumerate(NANO)},
    "U2": {"1": "GND", "2": "VIN", "3": "VOUT"},
    "Q1": {"1": "S", "2": "G", "3": "D"},
    "D": {"1": "K", "2": "A"},
    "C1": {"1": "+", "2": "-"},
    "J2": {"1": "1_+10V", "2": "2_TACH", "3": "3_GND", "4": "4_PWM"},
    "J3": {"1": "1_BAK", "2": "2_CON", "3": "3_PSH", "4": "4_TRB", "5": "5_TRA"},
    "J4": {"1": "1_SDA", "2": "2_SCL", "3": "3_VCC", "4": "4_GND"},
}

# Schematic placement (sheet mm, grid 2.54), grouped by function.
SCH_POS = {
    "U1": (60.96, 101.6), "J3": (12.7, 147.32), "J4": (220.98, 30.48),
    "U4": (190.5, 76.2), "R1": (154.94, 45.72), "R2": (154.94, 30.48),
    "U2": (124.46, 30.48), "C3": (78.74, 30.48), "C4": (124.46, 10.16),
    "J2": (289.56, 121.92), "F1": (261.62, 157.48), "D1": (210.82, 157.48),
    "D2": (160.02, 157.48), "C1": (160.02, 142.24), "C2": (160.02, 127.0),
    "R3": (195.58, 129.54), "Q1": (241.3, 129.54), "R4": (195.58, 114.3),
    "R5": (226.06, 109.22), "R7": (226.06, 91.44), "R6": (195.58, 99.06),
    "D3": (160.02, 106.68), "D4": (160.02, 91.44), "C5": (160.02, 76.2),
}


def f(v):
    return f"{v:.4f}".rstrip("0").rstrip(".") if v else "0"


def mmxy(v):
    return pcbnew.ToMM(v.x), pcbnew.ToMM(v.y)


def to_board(x, y):
    """KiCad page mm (y down) -> EAGLE board mm (origin lower-left, y up)."""
    return x - OX, H - (y - OY)


def sanitize(s):
    return "".join(c if c.isalnum() or c in "_-." else "_" for c in s)


def rot_attr(deg):
    d = round(deg) % 360
    return f"R{d}"


def local(fp, x, y):
    """Absolute KiCad page coords -> footprint-local EAGLE coords (y up)."""
    px, py = mmxy(fp.GetPosition())
    a = math.radians(fp.GetOrientationDegrees())
    dx, dy = x - px, y - py
    # inverse of KiCad's (y-down) CCW-visual rotation
    lx = dx * math.cos(a) - dy * math.sin(a)
    ly = dx * math.sin(a) + dy * math.cos(a)
    return lx, -ly


# ---------------------------------------------------------------------------
# Library packages from the KiCad footprints
# ---------------------------------------------------------------------------
GFX_LAYER = {pcbnew.F_SilkS: 21, pcbnew.F_Fab: 51}


def graphic_xml(fp, g, layer):
    out = []
    w = max(pcbnew.ToMM(g.GetWidth()), 0.1)
    shape = g.GetShape()
    if shape == pcbnew.SHAPE_T_SEGMENT:
        a, b = local(fp, *mmxy(g.GetStart())), local(fp, *mmxy(g.GetEnd()))
        out.append((a, b, 0))
    elif shape == pcbnew.SHAPE_T_RECT:
        (x0, y0), (x1, y1) = mmxy(g.GetStart()), mmxy(g.GetEnd())
        pts = [local(fp, *p) for p in ((x0, y0), (x1, y0), (x1, y1), (x0, y1))]
        out += [(pts[i], pts[(i + 1) % 4], 0) for i in range(4)]
    elif shape == pcbnew.SHAPE_T_POLY:
        poly = g.GetPolyShape().Outline(0)
        pts = [local(fp, *mmxy(poly.CPoint(i))) for i in range(poly.PointCount())]
        out += [(pts[i], pts[(i + 1) % len(pts)], 0) for i in range(len(pts))]
    elif shape == pcbnew.SHAPE_T_CIRCLE:
        c = local(fp, *mmxy(g.GetCenter()))
        r = pcbnew.ToMM(g.GetRadius())
        return [f'<circle x="{f(c[0])}" y="{f(c[1])}" radius="{f(r)}" width="{f(w)}" layer="{layer}"/>']
    elif shape == pcbnew.SHAPE_T_ARC:
        s, m, e = (local(fp, *mmxy(p)) for p in (g.GetStart(), g.GetArcMid(), g.GetEnd()))
        out.append((s, e, arc_sweep(s, m, e)))
    xml = []
    for a, b, curve in out:
        c = f' curve="{f(curve)}"' if curve else ""
        xml.append(f'<wire x1="{f(a[0])}" y1="{f(a[1])}" x2="{f(b[0])}" y2="{f(b[1])}" '
                   f'width="{f(w)}" layer="{layer}"{c}/>')
    return xml


def arc_sweep(s, m, e):
    """Signed sweep (deg, + = CCW) of the arc from s through m to e (y up)."""
    ax, ay = s
    bx, by = m
    cx, cy = e
    d = 2 * (ax * (by - cy) + bx * (cy - ay) + cx * (ay - by))
    ux = ((ax**2 + ay**2) * (by - cy) + (bx**2 + by**2) * (cy - ay) + (cx**2 + cy**2) * (ay - by)) / d
    uy = ((ax**2 + ay**2) * (cx - bx) + (bx**2 + by**2) * (ax - cx) + (cx**2 + cy**2) * (bx - ax)) / d
    t = lambda p: math.atan2(p[1] - uy, p[0] - ux)
    ts, tm, te = t(s), t(m), t(e)
    ccw = (te - ts) % (2 * math.pi)
    if (tm - ts) % (2 * math.pi) < ccw:
        return math.degrees(ccw)
    return -math.degrees(2 * math.pi - ccw)


def pad_xml(fp, p):
    x, y = local(fp, *mmxy(p.GetPosition()))
    drill = pcbnew.ToMM(p.GetDrillSize().x)
    sx, sy = pcbnew.ToMM(p.GetSize().x), pcbnew.ToMM(p.GetSize().y)
    shape = "square" if p.GetShape() in (pcbnew.PAD_SHAPE_RECT, pcbnew.PAD_SHAPE_ROUNDRECT) else "round"
    first = ' first="yes"' if p.GetNumber() == "1" else ""
    # EAGLE pads are round or square; the few slightly oval KiCad pads
    # (1.9 x 2.0 mm) become round with the smaller size.
    return (f'<pad name={quoteattr(p.GetNumber())} x="{f(x)}" y="{f(y)}" drill="{f(drill)}" '
            f'diameter="{f(min(sx, sy))}" shape="{shape}"{first}/>')


def package_name(fp):
    name = sanitize(fp.GetFPID().GetLibItemName().wx_str())
    if fp.GetReference() == "J2":
        name += "_PAD3.3"
    return name


def package_xml(fp):
    xml = [f'<package name="{package_name(fp)}">',
           f'<description>{escape(fp.GetFPID().GetUniStringLibId())} (from KiCad, pads enlarged '
           f'for a single-sided board)</description>']
    for g in fp.GraphicalItems():
        if g.GetClass() == "MGRAPHIC" and g.GetLayer() in GFX_LAYER:
            xml += graphic_xml(fp, g, GFX_LAYER[g.GetLayer()])
    for p in fp.Pads():
        if p.GetAttribute() == pcbnew.PAD_ATTRIB_NPTH:
            continue
        xml.append(pad_xml(fp, p))
    rx, ry = local(fp, *mmxy(fp.Reference().GetPosition()))
    xml.append(f'<text x="{f(rx)}" y="{f(ry)}" size="1.27" layer="25" align="center">&gt;NAME</text>')
    xml.append(f'<text x="{f(rx)}" y="{f(ry - 1.9)}" size="1.27" layer="27" align="center">&gt;VALUE</text>')
    xml.append("</package>")
    return xml


# ---------------------------------------------------------------------------
# Symbols and devices (schematic)
# ---------------------------------------------------------------------------
def pin_names(ref, pads):
    names = PIN_NAMES.get(ref) or PIN_NAMES.get(ref[0]) or {}
    return {p: names.get(p, p) for p in pads}


def symbol_xml(sym, ref, pads):
    """Generic symbol: 2-pin parts left/right, others a box with pins on both sides."""
    names = pin_names(ref, pads)
    xml = [f'<symbol name="{sym}">']
    if len(pads) == 2:
        # pad order: the diode's anode (pad 2) on the left
        left, right = (pads[1], pads[0]) if ref[0] == "D" else (pads[0], pads[1])
        vis = "pad" if ref[0] in "RCF" and ref != "C1" else "pin"
        xml.append(f'<pin name={quoteattr(names[left])} x="-7.62" y="0" length="short" visible="{vis}"/>')
        xml.append(f'<pin name={quoteattr(names[right])} x="7.62" y="0" length="short" visible="{vis}" rot="R180"/>')
        if ref[0] == "D":
            xml += ['<wire x1="-1.27" y1="1.27" x2="-1.27" y2="-1.27" width="0.254" layer="94"/>',
                    '<wire x1="-1.27" y1="1.27" x2="1.27" y2="0" width="0.254" layer="94"/>',
                    '<wire x1="-1.27" y1="-1.27" x2="1.27" y2="0" width="0.254" layer="94"/>',
                    '<wire x1="1.27" y1="1.27" x2="1.27" y2="-1.27" width="0.254" layer="94"/>',
                    '<wire x1="-5.08" y1="0" x2="-1.27" y2="0" width="0.1524" layer="94"/>',
                    '<wire x1="1.27" y1="0" x2="5.08" y2="0" width="0.1524" layer="94"/>']
        elif ref[0] == "C":
            xml += ['<wire x1="-0.635" y1="1.778" x2="-0.635" y2="-1.778" width="0.4064" layer="94"/>',
                    '<wire x1="0.635" y1="1.778" x2="0.635" y2="-1.778" width="0.4064" layer="94"/>',
                    '<wire x1="-5.08" y1="0" x2="-0.635" y2="0" width="0.1524" layer="94"/>',
                    '<wire x1="0.635" y1="0" x2="5.08" y2="0" width="0.1524" layer="94"/>']
            if ref == "C1":
                xml.append('<text x="-2.54" y="1.27" size="1.27" layer="94">+</text>')
        else:  # resistor / PTC: a box
            xml += [f'<wire x1="{a}" y1="{b}" x2="{c}" y2="{d}" width="0.254" layer="94"/>'
                    for a, b, c, d in ((-5.08, 1.016, 5.08, 1.016), (5.08, 1.016, 5.08, -1.016),
                                       (5.08, -1.016, -5.08, -1.016), (-5.08, -1.016, -5.08, 1.016))]
            if ref[0] == "F":
                xml.append('<wire x1="-5.08" y1="-1.016" x2="5.08" y2="1.016" width="0.1524" layer="94"/>')
        xml += ['<text x="-5.08" y="2.54" size="1.778" layer="95">&gt;NAME</text>',
                '<text x="-5.08" y="-4.318" size="1.778" layer="96">&gt;VALUE</text>']
    else:
        n = len(pads)
        nl = (n + 1) // 2 if n > 4 else n
        left, right = pads[:nl], pads[nl:]
        rows = max(len(left), len(right))
        hw = 10.16 if ref == "U1" else 7.62
        top = 2.54 * (rows - 1) / 2
        top = round(top / 2.54) * 2.54
        bot = top - 2.54 * (rows - 1)
        for i, p in enumerate(left):
            xml.append(f'<pin name={quoteattr(names[p])} x="{f(-hw - 2.54)}" y="{f(top - 2.54 * i)}" length="short"/>')
        for i, p in enumerate(right):
            xml.append(f'<pin name={quoteattr(names[p])} x="{f(hw + 2.54)}" y="{f(top - 2.54 * i)}" '
                       f'length="short" rot="R180"/>')
        y0, y1 = bot - 2.54, top + 2.54
        xml += [f'<wire x1="{f(a)}" y1="{f(b)}" x2="{f(c)}" y2="{f(d)}" width="0.254" layer="94"/>'
                for a, b, c, d in ((-hw, y1, hw, y1), (hw, y1, hw, y0), (hw, y0, -hw, y0), (-hw, y0, -hw, y1))]
        xml += [f'<text x="{f(-hw)}" y="{f(y1 + 0.762)}" size="1.778" layer="95">&gt;NAME</text>',
                f'<text x="{f(-hw)}" y="{f(y0 - 2.54)}" size="1.778" layer="96">&gt;VALUE</text>']
    xml.append("</symbol>")
    return xml


def pin_points(ref, pads):
    """Connection point (symbol coords) and outward direction of each pad's pin."""
    names = pin_names(ref, pads)
    pts = {}
    if len(pads) == 2:
        left, right = (pads[1], pads[0]) if ref[0] == "D" else (pads[0], pads[1])
        pts[left] = ((-7.62, 0.0), -1)
        pts[right] = ((7.62, 0.0), 1)
    else:
        n = len(pads)
        nl = (n + 1) // 2 if n > 4 else n
        rows = max(nl, n - nl)
        hw = 10.16 if ref == "U1" else 7.62
        top = round((2.54 * (rows - 1) / 2) / 2.54) * 2.54
        for i, p in enumerate(pads[:nl]):
            pts[p] = ((-hw - 2.54, top - 2.54 * i), -1)
        for i, p in enumerate(pads[nl:]):
            pts[p] = ((hw + 2.54, top - 2.54 * i), 1)
    return pts, names


def pad_order(fp):
    pads = [p.GetNumber() for p in fp.Pads() if p.GetAttribute() != pcbnew.PAD_ATTRIB_NPTH]
    if fp.GetReference() == "U4":  # HV side left, LV side right
        pads = ["HV1", "HV2", "HV3", "HV4", "HV", "GND_HV", "LV1", "LV2", "LV3", "LV4", "LV", "GND_LV"]
    return list(dict.fromkeys(pads))


# ---------------------------------------------------------------------------
LAYERS = [
    (1, "Top", 4, 1), (16, "Bottom", 1, 1), (17, "Pads", 2, 1), (18, "Vias", 2, 1),
    (19, "Unrouted", 6, 1), (20, "Dimension", 24, 1), (21, "tPlace", 7, 1), (22, "bPlace", 7, 1),
    (23, "tOrigins", 15, 1), (24, "bOrigins", 15, 1), (25, "tNames", 7, 1), (26, "bNames", 7, 1),
    (27, "tValues", 7, 1), (28, "bValues", 7, 1), (29, "tStop", 7, 3), (30, "bStop", 7, 6),
    (31, "tCream", 7, 4), (32, "bCream", 7, 5), (35, "tGlue", 7, 4), (36, "bGlue", 7, 5),
    (39, "tKeepout", 4, 11), (40, "bKeepout", 1, 11), (41, "tRestrict", 4, 10),
    (42, "bRestrict", 1, 10), (43, "vRestrict", 2, 10), (44, "Drills", 7, 1), (45, "Holes", 7, 1),
    (46, "Milling", 3, 1), (47, "Measures", 7, 1), (48, "Document", 7, 1), (49, "Reference", 7, 1),
    (51, "tDocu", 7, 1), (52, "bDocu", 7, 1), (88, "SimResults", 9, 1), (89, "SimProbes", 9, 1),
    (90, "Modules", 5, 1), (91, "Nets", 2, 1), (92, "Busses", 1, 1), (93, "Pins", 2, 1),
    (94, "Symbols", 4, 1), (95, "Names", 7, 1), (96, "Values", 7, 1), (97, "Info", 7, 1),
    (98, "Guide", 6, 1),
]


def header():
    xml = ['<?xml version="1.0" encoding="utf-8"?>',
           '<!DOCTYPE eagle SYSTEM "eagle.dtd">',
           '<eagle version="9.6.2">', "<drawing>",
           '<settings><setting alwaysvectorfont="no"/><setting verticaltext="up"/></settings>',
           '<grid distance="0.1" unitdist="inch" unit="inch" style="lines" multiple="1" display="no" '
           'altdistance="0.01" altunitdist="inch" altunit="inch"/>', "<layers>"]
    xml += [f'<layer number="{n}" name="{nm}" color="{c}" fill="{fi}" visible="yes" active="yes"/>'
            for n, nm, c, fi in LAYERS]
    xml.append("</layers>")
    return xml


CLASSES = ['<classes>',
           '<class number="0" name="default" width="0.5" drill="0"><clearance class="0" value="0.6"/></class>',
           '<class number="1" name="power" width="0.8" drill="0"><clearance class="1" value="0.6"/>'
           '<clearance class="0" value="0.6"/></class>',
           '</classes>']


def net_class(n):
    return "1" if n in POWER_NETS else "0"


DESIGNRULES = """<designrules name="cloudline_single_sided">
<description language="en">Cloudline controller: one copper layer (Bottom, 16); 0.6 mm clearance for isolation milling.</description>
<param name="layerSetup" value="(1*16)"/>
<param name="mtCopper" value="0.035mm 0.035mm"/>
<param name="mtIsolate" value="1.5mm"/>
<param name="mdWireWire" value="0.6mm"/>
<param name="mdWirePad" value="0.6mm"/>
<param name="mdWireVia" value="0.6mm"/>
<param name="mdPadPad" value="0.6mm"/>
<param name="mdPadVia" value="0.6mm"/>
<param name="mdViaVia" value="0.6mm"/>
<param name="mdSmdPad" value="0.6mm"/>
<param name="mdSmdVia" value="0.6mm"/>
<param name="mdSmdSmd" value="0.6mm"/>
<param name="mdCopperDimension" value="0.8mm"/>
<param name="mdDrill" value="0.25mm"/>
<param name="msWidth" value="0.4mm"/>
<param name="msDrill" value="0.7mm"/>
<param name="rvPadTop" value="0.25"/>
<param name="rvPadBottom" value="0.25"/>
<param name="rlMinPadTop" value="0.25mm"/>
<param name="rlMaxPadTop" value="0.5mm"/>
<param name="rlMinPadBottom" value="0.25mm"/>
<param name="rlMaxPadBottom" value="0.5mm"/>
<param name="psElongationLong" value="100"/>
<param name="psElongationOffset" value="100"/>
</designrules>"""


def main():
    board = pcbnew.LoadBoard(PCB)
    fps = sorted((fp for fp in board.GetFootprints() if not fp.GetReference().startswith("H")),
                 key=lambda fp: (fp.GetReference()[0], int(fp.GetReference()[1:])))
    holes = [fp for fp in board.GetFootprints() if fp.GetReference().startswith("H")]

    # One package per footprint; one symbol/deviceset per part (pin names differ).
    packages, pkg_of = {}, {}
    for fp in fps:
        name = package_name(fp)
        pkg_of[fp.GetReference()] = name
        if name not in packages:
            # the package is drawn from the first instance; all instances of a
            # footprint have the same pads (checked by check_eagle.py)
            packages[name] = package_xml(fp)

    nets = {}  # net -> [(ref, pad)]
    for fp in fps:
        for p in fp.Pads():
            n = p.GetNetname()
            if n:
                nets.setdefault(n, []).append((fp.GetReference(), p.GetNumber()))

    lib_pk = ["<packages>"] + sum(packages.values(), []) + ["</packages>"]

    # ------------------------------------------------------------- board
    brd = header() + ["<board>", "<plain>"]
    for d in board.GetDrawings():
        if d.GetLayer() == pcbnew.Edge_Cuts:
            (x1, y1), (x2, y2) = (to_board(*mmxy(d.GetStart())), to_board(*mmxy(d.GetEnd())))
            brd.append(f'<wire x1="{f(x1)}" y1="{f(y1)}" x2="{f(x2)}" y2="{f(y2)}" width="0" layer="20"/>')
        elif d.GetClass() == "PCB_TEXT" and d.GetLayer() == pcbnew.F_SilkS:
            x, y = to_board(*mmxy(d.GetPosition()))
            size = pcbnew.ToMM(d.GetTextHeight())
            brd.append(f'<text x="{f(x)}" y="{f(y)}" size="{f(size)}" layer="21" ratio="15" '
                       f'rot="{rot_attr(d.GetTextAngleDegrees())}" align="center">{escape(d.GetText())}</text>')
    for fp in holes:
        x, y = to_board(*mmxy(fp.GetPosition()))
        drill = pcbnew.ToMM(list(fp.Pads())[0].GetDrillSize().x)
        brd.append(f'<hole x="{f(x)}" y="{f(y)}" drill="{f(drill)}"/>')
    brd.append(f'<text x="2" y="-3" size="1.27" layer="48">Single-sided: copper on Bottom (16) only. '
               f'Generated from KiCad by make_eagle.py.</text>')
    brd += ["</plain>", "<libraries>", f'<library name="{LIB}">', *lib_pk, "</library>", "</libraries>",
            *CLASSES, DESIGNRULES, "<elements>"]
    for fp in fps:
        x, y = to_board(*mmxy(fp.GetPosition()))
        brd.append(f'<element name="{fp.GetReference()}" library="{LIB}" package="{pkg_of[fp.GetReference()]}" '
                   f'value={quoteattr(fp.GetValue())} x="{f(x)}" y="{f(y)}" '
                   f'rot="{rot_attr(fp.GetOrientationDegrees())}"/>')
    brd += ["</elements>", "<signals>"]
    tracks = {}
    for t in board.GetTracks():
        if t.GetLayer() != pcbnew.B_Cu:
            sys.exit("copper found off B.Cu")
        tracks.setdefault(t.GetNetname(), []).append(t)
    for n in sorted(nets):
        brd.append(f'<signal name={quoteattr(n)} class="{net_class(n)}">')
        brd += [f'<contactref element="{r}" pad={quoteattr(p)}/>' for r, p in nets[n]]
        for t in tracks.get(n, []):
            (x1, y1), (x2, y2) = to_board(*mmxy(t.GetStart())), to_board(*mmxy(t.GetEnd()))
            brd.append(f'<wire x1="{f(x1)}" y1="{f(y1)}" x2="{f(x2)}" y2="{f(y2)}" '
                       f'width="{f(pcbnew.ToMM(t.GetWidth()))}" layer="16"/>')
        brd.append("</signal>")
    brd += ["</signals>", "</board>", "</drawing>", "</eagle>"]

    # ------------------------------------------------------------- schematic
    syms, dsets = [], []
    for fp in fps:
        ref = fp.GetReference()
        pads = pad_order(fp)
        sym = f"SYM_{ref}"
        syms += symbol_xml(sym, ref, pads)
        names = pin_names(ref, pads)
        prefix = "".join(c for c in ref if c.isalpha())
        dsets += [f'<deviceset name="DEV_{ref}" prefix="{prefix}">',
                  f'<gates><gate name="G$1" symbol="{sym}" x="0" y="0"/></gates>',
                  f'<devices><device name="" package="{pkg_of[ref]}"><connects>']
        dsets += [f'<connect gate="G$1" pin={quoteattr(names[p])} pad={quoteattr(p)}/>' for p in pads]
        dsets += ['</connects><technologies><technology name=""/></technologies></device></devices>',
                  "</deviceset>"]
    sch = header() + ["<schematic>", "<libraries>", f'<library name="{LIB}">', *lib_pk,
                      "<symbols>", *syms, "</symbols>", "<devicesets>", *dsets, "</devicesets>",
                      "</library>", "</libraries>", *CLASSES, "<parts>"]
    sch += [f'<part name="{fp.GetReference()}" library="{LIB}" deviceset="DEV_{fp.GetReference()}" '
            f'device="" value={quoteattr(fp.GetValue())}/>' for fp in fps]
    sch += ["</parts>", "<sheets>", "<sheet>", "<plain>",
            '<text x="0" y="-10.16" size="2.54" layer="97">Cloudline AI-CLS6 controller, Rev B. '
            'Nets are joined by their labels. Generated from the KiCad board by make_eagle.py; '
            'the drawn schematic is hardware/schematic.png.</text>',
            "</plain>", "<instances>"]
    sch += [f'<instance part="{r}" gate="G$1" x="{f(SCH_POS[r][0])}" y="{f(SCH_POS[r][1])}"/>'
            for r in (fp.GetReference() for fp in fps)]
    sch += ["</instances>", "<nets>"]
    stubs = {}
    for fp in fps:
        ref = fp.GetReference()
        pts, names = pin_points(ref, pad_order(fp))
        stubs[ref] = (pts, names)
    for n in sorted(nets):
        sch.append(f'<net name={quoteattr(n)} class="{net_class(n)}">')
        for ref, pad in nets[n]:
            (px, py), d = stubs[ref][0][pad]
            x0, y0 = SCH_POS[ref][0] + px, SCH_POS[ref][1] + py
            x1 = x0 + 5.08 * d
            sch += ["<segment>",
                    f'<pinref part="{ref}" gate="G$1" pin={quoteattr(stubs[ref][1][pad])}/>',
                    f'<wire x1="{f(x0)}" y1="{f(y0)}" x2="{f(x1)}" y2="{f(y0)}" width="0.1524" layer="91"/>',
                    f'<label x="{f(x1)}" y="{f(y0)}" size="1.27" layer="95"'
                    f'{"" if d > 0 else " rot=" + chr(34) + "R180" + chr(34)}/>',
                    "</segment>"]
        sch.append("</net>")
    sch += ["</nets>", "</sheet>", "</sheets>", "</schematic>", "</drawing>", "</eagle>"]

    for path, lines in ((OUT_BRD, brd), (OUT_SCH, sch)):
        with open(path, "w", encoding="utf-8") as fh:
            fh.write("\n".join(lines) + "\n")
        print("wrote", path)
    print(f"{len(fps)} parts, {len(packages)} packages, {len(nets)} nets, "
          f"{sum(len(v) for v in tracks.values())} track segments, {len(holes)} holes")


if __name__ == "__main__":
    main()
