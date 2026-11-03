#!/usr/bin/env python3
"""Generate the single-sided PCB for the Cloudline AI-CLS6 controller.

    python3 hardware/pcb/make_pcb.py      (needs KiCad 7's pcbnew module)

Writes hardware/pcb/cloudline_controller.kicad_pcb. The board has ONE copper
layer (B.Cu, the bottom). All parts are through-hole and sit on the top
side; their leads are soldered on the bottom. Where a single layer cannot
avoid a crossing, the router places a wire link (W1, W2, ...): a piece of
bare tinned wire on the top side, like a 0 ohm resistor.

The netlist below is the schematic (hardware/schematic.py) written out pin
by pin; hardware/pcb/check_pcb.py compares it with PINOUT.md.

Design rules (also stored in the board for KiCad's DRC):
  clearance 0.35 mm, signal tracks 0.5 mm, power tracks 0.8 mm,
  pads at least 1.7 mm (holes are unplated on a single-sided board, so
  the copper ring is what holds the lead).
"""

import heapq
import math
import os
import sys

import numpy as np
import pcbnew

HERE = os.path.dirname(os.path.abspath(__file__))
FPLIB = "/usr/share/kicad/footprints"
OUT = os.path.join(HERE, "cloudline_controller.kicad_pcb")

# Board outline (mm) and its offset on the KiCad page.
W, H = 100.0, 75.0
OX, OY = 50.0, 40.0

CLEAR = 0.35        # copper-to-copper clearance
EDGE = 0.8          # copper-to-board-edge clearance
GRID = 0.25         # router grid
TRACK = {"signal": 0.5, "power": 0.8}
POWER_NETS = {"GND", "+5V", "+10V_FAN", "+10V_P", "N_F1_D1"}

# ---------------------------------------------------------------------------
# Parts: ref, (library, footprint), value, position of pad 1 (board mm),
# rotation (deg, KiCad CCW), {pad: net}
# ---------------------------------------------------------------------------
NANO_X0, NANO_YT = 5.5, 30.0          # pin 15 (D12) x, top-row y
# Nano pin x positions: top row pin p at 41.06 - (p-1)*2.54 (D9 13.12, D7 18.2,
# D3 28.36, D2 30.9); bottom row pin q at 5.5 + (q-16)*2.54 (A4 23.28,
# A5 25.82, 5V 33.44, GND 38.52, VIN 41.06).
PARTS = [
    # Only one GND pin of the Nano (29) and of U4 (GND_LV) is wired: the
    # module's own ground plane joins its two GND pins.
    ("U1", ("Module", "Arduino_Nano"), "Arduino Nano (socketed)",
     (NANO_X0 + 35.56, NANO_YT), -90,
     {"5": "TACH_5V", "6": "ENC_A", "7": "ENC_B", "8": "ENC_SW",
      "9": "KEY_CON", "10": "KEY_BAK", "12": "PWM_D9",
      "23": "SDA_5V", "24": "SCL_5V", "27": "+5V", "29": "GND", "30": "+10V_P"}),
    # Fan cable screw terminal, wire entry facing the right board edge.
    # Pins bottom to top: 1 +10V, 2 TACH, 3 GND, 4 PWM (labelled on the silkscreen).
    ("J2", ("TerminalBlock_Phoenix", "TerminalBlock_Phoenix_MKDS-1,5-4-5.08_1x04_P5.08mm_Horizontal"),
     "Fan: 1 +10V 2 TACH 3 GND 4 PWM", (92.5, 32.24), 90,
     {"1": "+10V_FAN", "2": "TACH_IN", "3": "GND", "4": "FAN_PWM"}),
    # OLED module: keys/encoder header above the Nano (straight above D7..D3),
    # power/I2C header below the level shifter (straight below LV1..GND).
    ("J3", ("Connector_PinHeader_2.54mm", "PinHeader_1x05_P2.54mm_Vertical"),
     "OLED keys: BAK CON PSH TRB TRA", (18.2, 18.5), 90,
     {"1": "KEY_BAK", "2": "KEY_CON", "3": "ENC_SW", "4": "ENC_B", "5": "ENC_A"}),
    ("J4", ("Connector_PinHeader_2.54mm", "PinHeader_1x04_P2.54mm_Vertical"),
     "OLED: SDA SCL VCC GND", (23.28, 72.2), 90,
     {"1": "SDA_3V3", "2": "SCL_3V3", "3": "+3V3", "4": "GND"}),
    ("U4", ("custom", "BOB-12009"), "SparkFun BOB-12009", (23.28, 51.5), 0,
     {"HV1": "SDA_5V", "HV2": "SCL_5V", "HV": "+5V",
      "LV1": "SDA_3V3", "LV2": "SCL_3V3", "LV": "+3V3", "GND_LV": "GND"}),
    # I2C pull-ups standing upright between neighbouring columns
    ("R2", ("Resistor_THT", "R_Axial_DIN0207_L6.3mm_D2.5mm_P2.54mm_Vertical"), "2.2k", (25.82, 65.0), 0, {"1": "SCL_3V3", "2": "+3V3"}),
    ("R1", ("Resistor_THT", "R_Axial_DIN0207_L6.3mm_D2.5mm_P2.54mm_Vertical"), "2.2k", (23.28, 68.4), 180, {"1": "SDA_3V3", "2": "+3V3"}),
    # 3.3 V regulator, VOUT towards the level shifter
    ("U2", ("Package_TO_SOT_THT", "TO-92_Inline_Wide"), "MCP1700-3302E", (47.0, 60.0), 180,
     {"1": "GND", "2": "+5V", "3": "+3V3"}),
    ("C3", ("Capacitor_THT", "C_Disc_D5.0mm_W2.5mm_P5.00mm"), "1uF", (44.46, 55.5), 0, {"1": "+5V", "2": "GND"}),
    ("C4", ("Capacitor_THT", "C_Disc_D5.0mm_W2.5mm_P5.00mm"), "1uF", (41.92, 64.5), 0, {"1": "+3V3", "2": "GND"}),
    # Power input: J2.1 -> F1 -> D1 -> +10V_P line (D2, C1, C2) -> Nano VIN
    ("F1", ("Fuse", "Fuse_BelFuse_0ZRE0005FF_L8.3mm_W3.8mm"), "PTC 0.2A MF-R020",
     (88.0, 40.0), 0, {"1": "+10V_FAN", "2": "N_F1_D1"}),
    ("D1", ("Diode_THT", "D_DO-41_SOD81_P10.16mm_Horizontal"), "1N5817",
     (70.0, 46.5), 0, {"1": "+10V_P", "2": "N_F1_D1"}),
    ("D2", ("Diode_THT", "D_DO-15_P12.70mm_Horizontal"), "P6KE13A",
     (64.0, 46.5), 270, {"1": "+10V_P", "2": "GND"}),
    ("C1", ("Capacitor_THT", "CP_Radial_D8.0mm_P3.50mm"), "220uF 25V",
     (57.0, 46.5), 270, {"1": "+10V_P", "2": "GND"}),
    ("C2", ("Capacitor_THT", "C_Disc_D5.0mm_W2.5mm_P5.00mm"), "100nF", (50.0, 46.5), 270, {"1": "+10V_P", "2": "GND"}),
    # Speed output: D9 -> R3 -> Q1 gate (from above); R4 (D9 end of R3) stands
    # across the tach and GND lines to +5V; Q1 drain -> J2.4
    ("Q1", ("Package_TO_SOT_THT", "TO-92_Inline_Wide"), "2N7000", (74.0, 11.0), 0,
     {"1": "GND", "2": "GATE", "3": "FAN_PWM"}),
    ("R3", ("Resistor_THT", "R_Axial_DIN0207_L6.3mm_D2.5mm_P7.62mm_Horizontal"), "220", (62.0, 5.0), 0, {"1": "PWM_D9", "2": "GATE"}),
    ("R4", ("Resistor_THT", "R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal"), "10k", (44.0, 15.0), 270, {"1": "PWM_D9", "2": "+5V"}),
    ("R5", ("Resistor_THT", "R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal"), "4.7k DNP", (80.0, 4.0), 0, {"1": "FAN_PWM", "2": "+10V_FAN"}),
    # Tach input: J2.2 -> R7 -> node line (y ~25) -> Nano D2; D4/C5 up to GND,
    # R6/D3 down to +5V
    ("R7", ("Resistor_THT", "R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal"), "1k", (72.0, 27.16), 0, {"1": "TACH_5V", "2": "TACH_IN"}),
    ("D4", ("Diode_THT", "D_DO-34_SOD68_P7.62mm_Horizontal"), "BAT85", (60.0, 25.0), 90, {"1": "TACH_5V", "2": "GND"}),
    ("C5", ("Capacitor_THT", "C_Disc_D5.0mm_W2.5mm_P5.00mm"), "10nF", (65.0, 25.0), 90, {"1": "TACH_5V", "2": "GND"}),
    ("R6", ("Resistor_THT", "R_Axial_DIN0207_L6.3mm_D2.5mm_P7.62mm_Horizontal"), "10k", (49.0, 25.0), 270, {"1": "TACH_5V", "2": "+5V"}),
    ("D3", ("Diode_THT", "D_DO-34_SOD68_P7.62mm_Horizontal"), "BAT85", (53.0, 32.62), 90, {"1": "+5V", "2": "TACH_5V"}),
    # Mounting holes (M3)
    ("H1", ("MountingHole", "MountingHole_3.2mm_M3"), "M3", (3.5, 3.5), 0, {}),
    ("H2", ("MountingHole", "MountingHole_3.2mm_M3"), "M3", (W - 3.5, 3.5), 0, {}),
    ("H3", ("MountingHole", "MountingHole_3.2mm_M3"), "M3", (3.5, H - 3.5), 0, {}),
    ("H4", ("MountingHole", "MountingHole_3.2mm_M3"), "M3", (W - 3.5, H - 3.5), 0, {}),
]

# Routing order: short, straight nets first; the big power nets last, when
# the router may bridge over earlier tracks with a wire link if it must.
ROUTE_ORDER = ["ENC_A", "ENC_B", "ENC_SW", "KEY_CON", "KEY_BAK", "SDA_5V", "SCL_5V",
               "SDA_3V3", "SCL_3V3", "+3V3", "GATE", "FAN_PWM", "TACH_IN", "N_F1_D1",
               "+10V_FAN", "+10V_P", "TACH_5V", "PWM_D9", "+5V", "GND"]


def mm(v):
    return pcbnew.FromMM(v)


def vec(x, y):
    return pcbnew.VECTOR2I(mm(x + OX), mm(y + OY))


# ---------------------------------------------------------------------------
# Footprints
# ---------------------------------------------------------------------------
def bob12009(board):
    """SparkFun BOB-12009: 15.24 x 12.7 mm, two rows of six 0.1" pins, rows
    0.4" apart (pad positions from SparkFun's Eagle board file). Origin = HV1.
    Top row (towards -y here) is the HV side; the LV row is 10.16 mm below."""
    fp = pcbnew.FOOTPRINT(board)
    fp.SetFPID(pcbnew.LIB_ID("cloudline", "SparkFun_BOB-12009"))
    names_hv = ["HV1", "HV2", "HV", "GND_HV", "HV3", "HV4"]
    names_lv = ["LV1", "LV2", "LV", "GND_LV", "LV3", "LV4"]
    for row, names in ((0.0, names_hv), (10.16, names_lv)):
        for i, n in enumerate(names):
            p = pcbnew.PAD(fp)
            p.SetNumber(n)
            p.SetAttribute(pcbnew.PAD_ATTRIB_PTH)
            p.SetShape(pcbnew.PAD_SHAPE_OVAL)
            p.SetSize(pcbnew.VECTOR2I(mm(1.7), mm(2.0)))
            p.SetDrillSize(pcbnew.VECTOR2I(mm(1.0), mm(1.0)))
            p.SetPos0(pcbnew.VECTOR2I(mm(2.54 * i), mm(row)))
            p.SetPosition(pcbnew.VECTOR2I(mm(2.54 * i), mm(row)))
            fp.Add(p)
    for layer, inset in ((pcbnew.F_SilkS, 0.0), (pcbnew.F_CrtYd, -0.25), (pcbnew.F_Fab, 0.0)):
        x0, y0, x1, y1 = -1.27 + inset, -1.27 + inset, 13.97 - inset, 11.43 - inset
        for a, b in (((x0, y0), (x1, y0)), ((x1, y0), (x1, y1)), ((x1, y1), (x0, y1)), ((x0, y1), (x0, y0))):
            fp_line(fp, a, b, layer, 0.12 if layer != pcbnew.F_CrtYd else 0.05)
    for txt, y in (("HV (5 V)", 1.9), ("LV (3.3 V)", 8.3)):
        t = pcbnew.FP_TEXT(fp)
        t.SetText(txt)
        t.SetLayer(pcbnew.F_SilkS)
        t.SetPos0(pcbnew.VECTOR2I(mm(6.35), mm(y)))
        t.SetTextSize(pcbnew.VECTOR2I(mm(1.0), mm(1.0)))
        t.SetTextThickness(mm(0.15))
        fp.Add(t)
        t.SetDrawCoord()
    return fp


def fp_line(fp, a, b, layer, width):
    s = pcbnew.FP_SHAPE(fp)
    s.SetShape(pcbnew.SHAPE_T_SEGMENT)
    s.SetStart0(pcbnew.VECTOR2I(mm(a[0]), mm(a[1])))
    s.SetEnd0(pcbnew.VECTOR2I(mm(b[0]), mm(b[1])))
    s.SetLayer(layer)
    s.SetWidth(mm(width))
    fp.Add(s)
    s.SetDrawCoord()


def wire_link(board, length):
    """Wire link (jumper): two pads `length` mm apart on the x axis."""
    fp = pcbnew.FOOTPRINT(board)
    fp.SetFPID(pcbnew.LIB_ID("cloudline", f"WireLink_P{length:.2f}mm"))
    for i, x in enumerate((0.0, length)):
        p = pcbnew.PAD(fp)
        p.SetNumber(str(i + 1))
        p.SetAttribute(pcbnew.PAD_ATTRIB_PTH)
        p.SetShape(pcbnew.PAD_SHAPE_CIRCLE)
        p.SetSize(pcbnew.VECTOR2I(mm(1.8), mm(1.8)))
        p.SetDrillSize(pcbnew.VECTOR2I(mm(0.8), mm(0.8)))
        p.SetPos0(pcbnew.VECTOR2I(mm(x), 0))
        p.SetPosition(pcbnew.VECTOR2I(mm(x), 0))
        fp.Add(p)
    fp_line(fp, (1.1, 0), (length - 1.1, 0), pcbnew.F_SilkS, 0.3)
    for a, b in (((-1.15, -1.15), (length + 1.15, -1.15)), ((length + 1.15, -1.15), (length + 1.15, 1.15)),
                 ((length + 1.15, 1.15), (-1.15, 1.15)), ((-1.15, 1.15), (-1.15, -1.15))):
        fp_line(fp, a, b, pcbnew.F_CrtYd, 0.05)
    return fp


def load_fp(board, lib, name):
    if lib == "custom":
        return bob12009(board)
    fp = pcbnew.FootprintLoad(os.path.join(FPLIB, lib + ".pretty"), name)
    if fp is None:
        sys.exit(f"footprint not found: {lib}:{name}")
    return fp


def single_sided_pads(fp):
    """Copper on B.Cu only, mask opening on B.Mask only; enlarge small pads."""
    lset = pcbnew.LSET()
    lset.AddLayer(pcbnew.B_Cu)
    lset.AddLayer(pcbnew.B_Mask)
    for p in fp.Pads():
        if p.GetAttribute() == pcbnew.PAD_ATTRIB_NPTH:
            continue
        p.SetLayerSet(lset)
        sx, sy = pcbnew.ToMM(p.GetSize().x), pcbnew.ToMM(p.GetSize().y)
        drill = pcbnew.ToMM(p.GetDrillSize().x)
        minimum = max(1.7, drill + 0.9)  # >= 0.45 mm copper ring
        if min(sx, sy) < minimum:
            p.SetSize(pcbnew.VECTOR2I(mm(max(sx, minimum)), mm(max(sy, minimum))))


# ---------------------------------------------------------------------------
# Geometry of copper objects for the router
# ---------------------------------------------------------------------------
class Obj:
    """Copper object: circle/rect pad or a track segment."""

    def __init__(self, net, kind, **kw):
        self.net, self.kind = net, kind
        self.__dict__.update(kw)

    def dist_grid(self, X, Y):
        """Distance from points (X, Y) to the object's copper edge (<0 inside)."""
        if self.kind == "circle":
            return np.hypot(X - self.x, Y - self.y) - self.r
        if self.kind == "rect":  # axis-aligned rectangle with corner radius rr
            dx = np.abs(X - self.x) - (self.hw - self.rr)
            dy = np.abs(Y - self.y) - (self.hh - self.rr)
            out = np.hypot(np.maximum(dx, 0), np.maximum(dy, 0))
            inside = np.minimum(np.maximum(dx, dy), 0)
            return out + inside - self.rr
        if self.kind == "seg":
            vx, vy = self.x2 - self.x1, self.y2 - self.y1
            L2 = vx * vx + vy * vy
            t = np.clip(((X - self.x1) * vx + (Y - self.y1) * vy) / L2, 0, 1) if L2 else 0
            return np.hypot(X - (self.x1 + t * vx), Y - (self.y1 + t * vy)) - self.w / 2
        raise ValueError(self.kind)

    def bbox(self, m):
        if self.kind == "circle":
            return self.x - self.r - m, self.y - self.r - m, self.x + self.r + m, self.y + self.r + m
        if self.kind == "rect":
            return self.x - self.hw - m, self.y - self.hh - m, self.x + self.hw + m, self.y + self.hh + m
        return (min(self.x1, self.x2) - self.w / 2 - m, min(self.y1, self.y2) - self.w / 2 - m,
                max(self.x1, self.x2) + self.w / 2 + m, max(self.y1, self.y2) + self.w / 2 + m)


def pad_obj(p, net):
    c = p.GetPosition()
    x, y = pcbnew.ToMM(c.x) - OX, pcbnew.ToMM(c.y) - OY
    sx, sy = pcbnew.ToMM(p.GetSize().x), pcbnew.ToMM(p.GetSize().y)
    ang = p.GetOrientation().AsDegrees() % 180
    if abs(ang - 90) < 1:
        sx, sy = sy, sx
    if p.GetShape() == pcbnew.PAD_SHAPE_CIRCLE:
        return Obj(net, "circle", x=x, y=y, r=sx / 2)
    rr = min(sx, sy) / 2 if p.GetShape() == pcbnew.PAD_SHAPE_OVAL else 0.0
    return Obj(net, "rect", x=x, y=y, hw=sx / 2, hh=sy / 2, rr=rr)


# ---------------------------------------------------------------------------
# Grid router (one layer, 8 directions, optional wire links)
# ---------------------------------------------------------------------------
NX, NY = int(W / GRID) + 1, int(H / GRID) + 1
GX = np.arange(NX) * GRID
GY = np.arange(NY) * GRID
XX, YY = np.meshgrid(GX, GY)  # [iy, ix]


def stamp(mask, obj, margin):
    x0, y0, x1, y1 = obj.bbox(margin)
    i0, i1 = max(0, int(x0 / GRID) - 1), min(NX, int(x1 / GRID) + 2)
    j0, j1 = max(0, int(y0 / GRID) - 1), min(NY, int(y1 / GRID) + 2)
    if i0 >= i1 or j0 >= j1:
        return
    d = obj.dist_grid(XX[j0:j1, i0:i1], YY[j0:j1, i0:i1])
    mask[j0:j1, i0:i1] |= d < margin


def edge_mask(margin):
    m = np.zeros((NY, NX), bool)
    m |= (XX < margin) | (YY < margin) | (XX > W - margin) | (YY > H - margin)
    return m


DIRS = [(1, 0, 1.0), (-1, 0, 1.0), (0, 1, 1.0), (0, -1, 1.0),
        (1, 1, 1.4142), (1, -1, 1.4142), (-1, 1, 1.4142), (-1, -1, 1.4142)]
LINK_LENGTHS = (7.5, 10.0)   # mm, wire link pad spacing
LINK_COST = 60.0             # mm-equivalent penalty: links only when needed


def astar(blocked, padfree, bodyfree, sources, target, allow_links):
    """Shortest path from any source cell to the target cell. Returns a list
    of steps: ("cell", (ix, iy)) and ("link", a, b) for wire links."""
    tx, ty = target
    INF = float("inf")
    g = {}
    prev = {}
    heap = []
    for s in sources:
        g[s] = 0.0
        heapq.heappush(heap, (math.hypot(s[0] - tx, s[1] - ty), 0.0, s, None))
    turn_pen = 0.3
    while heap:
        f, gc, cur, d = heapq.heappop(heap)
        if gc > g.get((cur, d) if False else cur, INF) + 1e-9:
            continue
        if cur == target:
            path = []
            node = cur
            while node in prev:
                p, kind = prev[node]
                path.append((kind, p, node))
                node = p
            path.reverse()
            return path
        cx, cy = cur
        for k, (dx, dy, c) in enumerate(DIRS):
            nx_, ny_ = cx + dx, cy + dy
            if not (0 <= nx_ < NX and 0 <= ny_ < NY) or blocked[ny_, nx_]:
                continue
            ng = gc + c + (turn_pen if d is not None and d != k else 0.0)
            if ng < g.get((nx_, ny_), INF):
                g[(nx_, ny_)] = ng
                prev[(nx_, ny_)] = (cur, "track")
                heapq.heappush(heap, (ng + math.hypot(nx_ - tx, ny_ - ty), ng, (nx_, ny_), k))
        if allow_links and padfree[cy, cx] and bodyfree[cy, cx]:
            for L in LINK_LENGTHS:
                n = int(round(L / GRID))
                for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                    ex, ey = cx + dx * n, cy + dy * n
                    if not (0 <= ex < NX and 0 <= ey < NY):
                        continue
                    if not (padfree[ey, ex] and bodyfree[ey, ex]):
                        continue
                    xs = range(min(cx, ex), max(cx, ex) + 1)
                    ys = range(min(cy, ey), max(cy, ey) + 1)
                    if dx:
                        if not all(bodyfree[cy, i] for i in xs):
                            continue
                    else:
                        if not all(bodyfree[j, cx] for j in ys):
                            continue
                    ng = gc + n + LINK_COST / GRID
                    if ng < g.get((ex, ey), INF):
                        g[(ex, ey)] = ng
                        prev[(ex, ey)] = (cur, "link")
                        heapq.heappush(heap, (ng + math.hypot(ex - tx, ey - ty), ng, (ex, ey), None))
    return None


def simplify(cells):
    """Grid cells -> list of corner points (mm)."""
    pts = [cells[0]]
    for i in range(1, len(cells) - 1):
        a, b, c = cells[i - 1], cells[i], cells[i + 1]
        if (b[0] - a[0], b[1] - a[1]) != (c[0] - b[0], c[1] - b[1]):
            pts.append(b)
    if len(cells) > 1:
        pts.append(cells[-1])
    return [(p[0] * GRID, p[1] * GRID) for p in pts]


def to_cell(x, y):
    return int(round(x / GRID)), int(round(y / GRID))


# ---------------------------------------------------------------------------
def main():
    board = pcbnew.BOARD()
    ds = board.GetDesignSettings()
    ds.SetCopperLayerCount(2)
    # F.Cu is a "jumper" layer: it carries only the wire links (bare wires on
    # the top side), so KiCad's connectivity and DRC see them. It is NOT a
    # copper layer of the fabricated board and is not exported as a Gerber.
    board.SetLayerType(pcbnew.F_Cu, pcbnew.LT_JUMPER)
    board.SetLayerName(pcbnew.F_Cu, "F.Cu")
    ds.m_MinClearance = mm(CLEAR)
    ds.m_TrackMinWidth = mm(0.4)
    ds.m_CopperEdgeClearance = mm(EDGE - 0.05)
    ds.m_MinThroughDrill = mm(0.7)
    ds.m_HoleClearance = mm(0.25)
    nc = ds.m_NetSettings.m_DefaultNetClass
    nc.SetClearance(mm(CLEAR))
    nc.SetTrackWidth(mm(TRACK["signal"]))

    tb = board.GetTitleBlock()
    tb.SetTitle("Cloudline AI-CLS6 controller - single-sided PCB")
    tb.SetRevision("B")
    tb.SetDate("2026-11-03")
    tb.SetCompany("github.com/seesharp15/cloudline-fan-controller")
    tb.SetComment(0, "1 copper layer (B.Cu). F.Cu = jumper layer (wire link W1), not fabricated.")
    tb.SetComment(1, "Generated by hardware/pcb/make_pcb.py")

    nets = {}

    def net(name):
        if name not in nets:
            n = pcbnew.NETINFO_ITEM(board, name)
            board.Add(n)
            nets[name] = n
        return nets[name]

    # Board outline
    for a, b in (((0, 0), (W, 0)), ((W, 0), (W, H)), ((W, H), (0, H)), ((0, H), (0, 0))):
        s = pcbnew.PCB_SHAPE(board)
        s.SetShape(pcbnew.SHAPE_T_SEGMENT)
        s.SetStart(vec(*a))
        s.SetEnd(vec(*b))
        s.SetLayer(pcbnew.Edge_Cuts)
        s.SetWidth(mm(0.1))
        board.Add(s)

    objs = []        # copper objects for the router
    pads_by_net = {}
    body = np.zeros((NY, NX), bool)  # top-side keep-out for wire links
    for ref, (lib, name), value, (x, y), rot, padnets in PARTS:
        fp = load_fp(board, lib, name)
        fp.SetReference(ref)
        fp.SetValue(value)
        if lib != "custom":
            fp.SetFPID(pcbnew.LIB_ID(lib, name))
        board.Add(fp)
        fp.SetOrientationDegrees(rot)
        fp.SetPosition(vec(x, y))
        single_sided_pads(fp)
        fp.Value().SetVisible(False)
        for p in fp.Pads():
            num = p.GetNumber()
            nname = padnets.get(num)
            if nname:
                p.SetNet(net(nname))
                pads_by_net.setdefault(nname, []).append(p)
            if p.GetAttribute() == pcbnew.PAD_ATTRIB_NPTH:
                c = p.GetPosition()
                objs.append(Obj("<hole>", "circle", x=pcbnew.ToMM(c.x) - OX,
                                y=pcbnew.ToMM(c.y) - OY, r=3.0))  # screw head
            else:
                objs.append(pad_obj(p, nname or f"<nc:{ref}.{num}>"))
        # Courtyard bounding box marks the top-side body area.
        bb = fp.GetBoundingBox(False, False)
        x0, y0 = pcbnew.ToMM(bb.GetX()) - OX, pcbnew.ToMM(bb.GetY()) - OY
        x1, y1 = x0 + pcbnew.ToMM(bb.GetWidth()), y0 + pcbnew.ToMM(bb.GetHeight())
        body |= (XX >= x0 - 0.3) & (XX <= x1 + 0.3) & (YY >= y0 - 0.3) & (YY <= y1 + 0.3)
        missing = set(padnets) - {p.GetNumber() for p in fp.Pads()}
        if missing:
            sys.exit(f"{ref}: pads {missing} not in footprint")

    links = []
    unrouted = []
    for nname in ([] if os.environ.get("PLACE_ONLY") else ROUTE_ORDER):
        pads = pads_by_net.get(nname, [])
        if len(pads) < 2:
            continue
        w = TRACK["power" if nname in POWER_NETS else "signal"]
        margin = CLEAR + w / 2 + 0.06
        blocked = edge_mask(EDGE + w / 2)
        padfree = ~edge_mask(EDGE + 0.9)
        for o in objs:
            if o.net == nname:
                continue
            stamp(blocked, o, margin)
            pf = np.zeros((NY, NX), bool)
        # cells where a wire-link pad (r 0.9) can sit without touching others
        occ = np.zeros((NY, NX), bool)
        for o in objs:
            if o.net != nname:
                stamp(occ, o, CLEAR + 0.9 + 0.06)
        padfree &= ~occ
        # a link needs ~2.3 mm of free top side: dilate the bodies by 1.2 mm
        r = int(math.ceil(1.2 / GRID))
        dil = body.copy()
        for k in range(1, r + 1):
            dil[k:, :] |= body[:-k, :]
            dil[:-k, :] |= body[k:, :]
            dil[:, k:] |= body[:, :-k]
            dil[:, :-k] |= body[:, k:]
        bodyfree = ~dil
        for lk in links:  # existing links' wires are bodies too
            (ax, ay), (bx, by) = lk[1], lk[2]
            bodyfree &= ~((XX >= min(ax, bx) - 1.2) & (XX <= max(ax, bx) + 1.2) &
                          (YY >= min(ay, by) - 1.2) & (YY <= max(ay, by) + 1.2))

        centers = []
        for p in pads:
            c = p.GetPosition()
            centers.append((pcbnew.ToMM(c.x) - OX, pcbnew.ToMM(c.y) - OY))
        # own pads are free
        for (px, py), p in zip(centers, pads):
            o = pad_obj(p, nname)
            m = np.zeros((NY, NX), bool)
            stamp(m, o, 0.0)
            blocked &= ~m

        tree = {to_cell(*centers[0])}
        tree_pts = [centers[0]]
        remaining = list(range(1, len(centers)))
        ok = True
        while remaining:
            # nearest remaining pad to the tree (straight-line)
            best = min(remaining, key=lambda i: min(math.hypot(centers[i][0] - a, centers[i][1] - b)
                                                    for a, b in tree_pts))
            target = to_cell(*centers[best])
            path = astar(blocked, padfree, bodyfree, list(tree), target, allow_links=True)
            if path is None:
                ok = False
                unrouted.append((nname, best))
                remaining.remove(best)
                continue
            if not path:  # target already on the tree
                remaining.remove(best)
                continue
            # split into track runs and links
            run = [path[0][1]]
            for kind, a, b in path:
                if kind == "track":
                    run.append(b)
                else:
                    if len(run) > 1:
                        add_tracks(board, net(nname), w, simplify(run), objs, nname)
                    lk_len = math.hypot(b[0] - a[0], b[1] - a[1]) * GRID
                    links.append((nname, (a[0] * GRID, a[1] * GRID), (b[0] * GRID, b[1] * GRID), lk_len))
                    add_link(board, net(nname), (a[0] * GRID, a[1] * GRID), (b[0] * GRID, b[1] * GRID),
                             len(links), objs, nname)
                    run = [b]
            if len(run) > 1:
                add_tracks(board, net(nname), w, simplify(run), objs, nname)
            for kind, a, b in path:
                tree.add(a)
                tree.add(b)
                tree_pts.append((b[0] * GRID, b[1] * GRID))
            tree.add(target)
            remaining.remove(best)
        print(f"{nname:10s} {'ok' if ok else 'INCOMPLETE'}  pads={len(pads)}")

    add_silk(board)
    tidy_silk(board)
    save_custom_footprints(board)
    board.SetFileName(OUT)
    pcbnew.SaveBoard(OUT, board)
    print(f"links: {len(links)}")
    for i, (nname, a, b, L) in enumerate(links, 1):
        print(f"  W{i}: {nname} {L:.1f} mm at ({a[0]:.2f},{a[1]:.2f})-({b[0]:.2f},{b[1]:.2f})")
    if unrouted:
        print("UNROUTED:", unrouted)
        sys.exit(1)
    print("wrote", OUT)


def add_tracks(board, netinfo, w, pts, objs, nname):
    for (x1, y1), (x2, y2) in zip(pts, pts[1:]):
        t = pcbnew.PCB_TRACK(board)
        t.SetStart(vec(x1, y1))
        t.SetEnd(vec(x2, y2))
        t.SetWidth(mm(w))
        t.SetLayer(pcbnew.B_Cu)
        t.SetNet(netinfo)
        board.Add(t)
        objs.append(Obj(nname, "seg", x1=x1, y1=y1, x2=x2, y2=y2, w=w))


def add_link(board, netinfo, a, b, n, objs, nname):
    L = math.hypot(b[0] - a[0], b[1] - a[1])
    fp = wire_link(board, L)
    fp.SetReference(f"W{n}")
    fp.SetValue("wire link")
    board.Add(fp)
    ang = math.degrees(math.atan2(-(b[1] - a[1]), b[0] - a[0]))
    fp.SetOrientationDegrees(ang)
    fp.SetPosition(vec(*a))
    single_sided_pads(fp)
    fp.Value().SetVisible(False)
    lset = pcbnew.LSET()
    for layer in (pcbnew.B_Cu, pcbnew.B_Mask, pcbnew.F_Cu):
        lset.AddLayer(layer)
    for p in fp.Pads():
        p.SetNet(netinfo)
        p.SetLayerSet(lset)
        objs.append(pad_obj(p, nname))
    t = pcbnew.PCB_TRACK(board)  # the wire itself, on the jumper layer
    t.SetStart(vec(*a))
    t.SetEnd(vec(*b))
    t.SetWidth(mm(0.6))
    t.SetLayer(pcbnew.F_Cu)
    t.SetNet(netinfo)
    board.Add(t)


def tidy_silk(board):
    """Drop footprint silk that lies off the board (the Nano's USB end hangs
    over the edge on purpose), then place every reference designator where it
    overlaps no other silkscreen, pad or the board edge."""
    for fp in board.GetFootprints():
        for item in list(fp.GraphicalItems()):
            if item.GetLayer() != pcbnew.F_SilkS:
                continue
            bb = item.GetBoundingBox()
            if pcbnew.ToMM(bb.GetX()) - OX < 0.3 or pcbnew.ToMM(bb.GetRight()) - OX > W - 0.3:
                fp.Remove(item)

    def rect_of(bb, grow=0.0):
        return (pcbnew.ToMM(bb.GetX()) - OX - grow, pcbnew.ToMM(bb.GetY()) - OY - grow,
                pcbnew.ToMM(bb.GetRight()) - OX + grow, pcbnew.ToMM(bb.GetBottom()) - OY + grow)

    def hit(a, b):
        return a[0] < b[2] and b[0] < a[2] and a[1] < b[3] and b[1] < a[3]

    taken = []
    for fp in board.GetFootprints():
        for item in fp.GraphicalItems():
            if item.GetLayer() == pcbnew.F_SilkS:
                taken.append(rect_of(item.GetBoundingBox(), 0.15))
        for p in fp.Pads():
            taken.append(rect_of(p.GetBoundingBox(), 0.25))
    for d in board.GetDrawings():
        if d.GetLayer() == pcbnew.F_SilkS:
            taken.append(rect_of(d.GetBoundingBox(), 0.15))
    fps = sorted(board.GetFootprints(), key=lambda f: f.GetReference())
    for fp in fps:
        ref = fp.Reference()
        if fp.GetReference().startswith("H"):
            ref.SetVisible(False)
            continue
        ref.SetTextSize(pcbnew.VECTOR2I(mm(1.0), mm(1.0)))
        ref.SetTextThickness(mm(0.15))
        ref.SetTextAngleDegrees(0)
        ref.SetKeepUpright(True)
        bb = fp.GetBoundingBox(False, False)
        x0, y0, x1, y1 = rect_of(bb)
        cx, cy = (x0 + x1) / 2, (y0 + y1) / 2
        tw = 0.9 * len(fp.GetReference()) + 0.4
        cands = [(cx, y0 - 0.9), (cx, y1 + 0.9), (x0 - tw / 2 - 0.3, cy), (x1 + tw / 2 + 0.3, cy),
                 (cx, cy), (x0 + tw / 2, y0 - 0.9), (x1 - tw / 2, y1 + 0.9)]
        for dy in (1.0, 2.0, 3.0):
            cands += [(cx, y0 - 0.9 - dy), (cx, y1 + 0.9 + dy)]
        placed = False
        for x, y in cands:
            r = (x - tw / 2, y - 0.65, x + tw / 2, y + 0.65)
            if r[0] < 0.5 or r[1] < 0.5 or r[2] > W - 0.5 or r[3] > H - 0.5:
                continue
            if any(hit(r, t) for t in taken):
                continue
            ref.SetPosition(vec(x, y))
            taken.append(r)
            placed = True
            break
        if not placed:
            print(f"  note: no free spot for {fp.GetReference()}'s label; left on the body (F.Fab)")
            ref.SetLayer(pcbnew.F_Fab)


def save_custom_footprints(board):
    """Write the two custom footprints to the project library cloudline.pretty
    (referenced by hardware/pcb/fp-lib-table) so the project is self-contained."""
    lib = os.path.join(HERE, "cloudline.pretty")
    os.makedirs(lib, exist_ok=True)
    seen = set()
    for fp in board.GetFootprints():
        name = fp.GetFPID().GetLibItemName().wx_str()
        if fp.GetFPID().GetLibNickname().wx_str() != "cloudline" or name in seen:
            continue
        seen.add(name)
        copy = pcbnew.FOOTPRINT(fp)
        copy.SetPosition(pcbnew.VECTOR2I(0, 0))
        copy.SetOrientationDegrees(0)
        copy.SetReference("REF**")
        pcbnew.FootprintSave(lib, copy)


def add_silk(board):
    def text(s, x, y, size=1.2, rot=0, layer=pcbnew.F_SilkS, bold=False):
        t = pcbnew.PCB_TEXT(board)
        t.SetText(s)
        t.SetLayer(layer)
        t.SetPosition(vec(x, y))
        t.SetTextSize(pcbnew.VECTOR2I(mm(size), mm(size)))
        t.SetTextThickness(mm(size * (0.2 if bold else 0.15)))
        t.SetTextAngleDegrees(rot)
        board.Add(t)

    text("CLOUDLINE AI-CLS6 CONTROLLER  Rev B", 30.0, 4.0, 1.4, bold=True)
    text("single-sided, 1 copper layer (bottom)", 30.0, 6.5, 1.0)
    # Pin functions next to the connectors (builders wire by these).
    for y, name in ((17.0, "PWM"), (22.08, "GND"), (27.16, "TACH"), (32.24, "+10V")):
        text(name, 98.6, y, 0.8, rot=90)
    text("FAN", 98.6, 11.5, 0.8, rot=90)
    for i, name in enumerate(("BAK", "CON", "PSH", "TRB", "TRA")):
        text(name, 18.2 + 2.54 * i, 16.3, 0.8)
    text("OLED: 1 SDA  2 SCL  3 VCC  4 GND", 44.0, 72.2, 0.8)


if __name__ == "__main__":
    main()
