#!/usr/bin/env python3
"""Generate hardware/schematic.svg (and .png) for the Cloudline controller.

Pure standard-library Python: every symbol is drawn from primitives here so
the schematic can be edited and reviewed as a diff. Run from the repo root:

    python3 hardware/schematic.py      # writes hardware/schematic.svg + .png

The PNG is rendered with `rsvg-convert` (librsvg) if it is installed, or
with the `cairosvg` Python package otherwise.

Coordinates are on a 10 px grid. Power rails use named power flags
(+10V_P, +5V, +3V3, GND); every signal is drawn as an explicit wire.
"""

import os
import shutil
import subprocess
import sys

W, H = 1500, 1030
out = []


def emit(s):
    out.append(s)


# --------------------------------------------------------------------------
# Primitives
# --------------------------------------------------------------------------
def line(x1, y1, x2, y2, cls="w"):
    emit(f'<line x1="{x1}" y1="{y1}" x2="{x2}" y2="{y2}" class="{cls}"/>')


def wire(*pts):
    """Polyline wire through the given (x, y) points."""
    d = " ".join(f"{x},{y}" for x, y in pts)
    emit(f'<polyline points="{d}" class="w" fill="none"/>')


def vwire_hop(x, y1, y2, hops):
    """Vertical wire from y1 to y2 that hops (arcs) over wires at `hops` y."""
    top, bot = min(y1, y2), max(y1, y2)
    d = f"M{x},{top}"
    for hy in sorted(hops):
        d += f" L{x},{hy - 6} A6,6 0 0,1 {x},{hy + 6}"
    d += f" L{x},{bot}"
    emit(f'<path d="{d}" class="w" fill="none"/>')


def dot(x, y):
    emit(f'<circle cx="{x}" cy="{y}" r="3.5" class="dot"/>')


def text(x, y, s, size=13, anchor="start", cls="t", weight="normal"):
    s = s.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;")
    emit(f'<text x="{x}" y="{y}" font-size="{size}" text-anchor="{anchor}" '
         f'font-weight="{weight}" class="{cls}">{s}</text>')


def label(x, y, ref, value, anchor="start", dy=0):
    """Reference designator (bold) and value under it."""
    text(x, y + dy, ref, 13, anchor, "ref", "bold")
    text(x, y + dy + 15, value, 12, anchor, "val")


# --------------------------------------------------------------------------
# Symbols
# --------------------------------------------------------------------------
def resistor_h(x1, x2, y, ref, value, lab_dy=-24, dnp=False):
    mid = (x1 + x2) / 2
    b1, b2 = mid - 20, mid + 20
    line(x1, y, b1, y)
    line(b2, y, x2, y)
    pts = [(b1, y)]
    for i in range(6):
        px = b1 + (i + 0.5) * (40 / 6)
        pts.append((px, y - 6 if i % 2 == 0 else y + 6))
    pts.append((b2, y))
    emit('<polyline points="%s" class="%s" fill="none"/>'
         % (" ".join(f"{a:.1f},{b}" for a, b in pts), "dnp" if dnp else "w"))
    label(mid, y + lab_dy, ref, value, "middle")


def resistor_v(x, y1, y2, ref, value, side="right", dnp=False, body_c=None):
    top, bot = min(y1, y2), max(y1, y2)
    mid = body_c if body_c is not None else (top + bot) / 2
    b1, b2 = mid - 20, mid + 20
    line(x, top, x, b1)
    line(x, b2, x, bot)
    pts = [(x, b1)]
    for i in range(6):
        py = b1 + (i + 0.5) * (40 / 6)
        pts.append((x - 6 if i % 2 == 0 else x + 6, py))
    pts.append((x, b2))
    emit('<polyline points="%s" class="%s" fill="none"/>'
         % (" ".join(f"{a},{b:.1f}" for a, b in pts), "dnp" if dnp else "w"))
    if side == "right":
        label(x + 12, mid - 4, ref, value)
    else:
        label(x - 12, mid - 4, ref, value, "end")


def resistor_v_hop(x, y1, y2, body_c, hops, ref, value, side="right"):
    """Vertical resistor whose lower lead hops over wires at `hops`."""
    b1, b2 = body_c - 20, body_c + 20
    top, bot = min(y1, y2), max(y1, y2)
    line(x, top, x, b1)
    vwire_hop(x, b2, bot, hops)
    pts = [(x, b1)]
    for i in range(6):
        py = b1 + (i + 0.5) * (40 / 6)
        pts.append((x - 6 if i % 2 == 0 else x + 6, py))
    pts.append((x, b2))
    emit('<polyline points="%s" class="w" fill="none"/>'
         % " ".join(f"{a},{b:.1f}" for a, b in pts))
    if side == "right":
        label(x + 12, body_c - 4, ref, value)
    else:
        label(x - 12, body_c - 4, ref, value, "end")


def ptc_h(x1, x2, y, ref, value):
    mid = (x1 + x2) / 2
    b1, b2 = mid - 22, mid + 22
    line(x1, y, b1, y)
    line(b2, y, x2, y)
    emit(f'<rect x="{b1}" y="{y - 8}" width="44" height="16" class="w" fill="none"/>')
    # PTC mark: diagonal with a short horizontal foot
    wire((b1 - 6, y + 14), (b1 + 2, y + 14), (b2 + 4, y - 14))
    label(mid, y - 34, ref, value, "middle")


def cap_v(x, y1, y2, ref, value, polarized=False, side="right"):
    top, bot = min(y1, y2), max(y1, y2)
    mid = (top + bot) / 2
    line(x, top, x, mid - 4)
    line(x, mid + 4, x, bot)
    line(x - 13, mid - 4, x + 13, mid - 4, "p")
    if polarized:
        emit(f'<path d="M{x - 13},{mid + 9} Q{x},{mid + 1} {x + 13},{mid + 9}" class="p" fill="none"/>')
        text(x - 20, mid - 9, "+", 14, "middle", "t", "bold")
    else:
        line(x - 13, mid + 4, x + 13, mid + 4, "p")
    if side == "right":
        label(x + 18, mid - 4, ref, value)
    else:
        label(x - 18, mid - 4, ref, value, "end")


def diode_h(x1, x2, y, ref, value):
    """Anode at x1, cathode at x2."""
    mid = (x1 + x2) / 2
    a, k = mid - 10, mid + 10
    line(x1, y, a, y)
    line(k, y, x2, y)
    emit(f'<polygon points="{a},{y - 10} {a},{y + 10} {k},{y}" class="w" fill="none"/>')
    line(k, y - 10, k, y + 10, "p")
    label(mid, y - 34, ref, value, "middle")


def tvs_v(x, ytop, ybot, ref, value, side="right"):
    """Unidirectional TVS, cathode at top (clamps the rail to GND)."""
    mid = (ytop + ybot) / 2
    k, a = mid - 10, mid + 10
    line(x, ytop, x, k)
    line(x, a, x, ybot)
    emit(f'<polygon points="{x - 10},{a} {x + 10},{a} {x},{k}" class="w" fill="none"/>')
    wire((x - 14, k - 5), (x - 10, k), (x + 10, k), (x + 14, k + 5))
    if side == "right":
        label(x + 18, mid - 4, ref, value)
    else:
        label(x - 18, mid - 4, ref, value, "end")


def schottky_v(x, ytop, ybot, ref, value, side="right"):
    """Schottky diode drawn vertically, cathode at the TOP."""
    mid = (ytop + ybot) / 2
    k, a = mid - 9, mid + 9
    line(x, ytop, x, k)
    line(x, a, x, ybot)
    emit(f'<polygon points="{x - 9},{a} {x + 9},{a} {x},{k}" class="w" fill="none"/>')
    # cathode bar with the Schottky "S" hooks
    wire((x - 13, k + 4), (x - 13, k), (x + 13, k), (x + 13, k - 4))
    if side == "right":
        label(x + 18, mid - 4, ref, value)
    else:
        label(x - 18, mid - 4, ref, value, "end")


def gnd(x, y):
    line(x, y, x, y + 10)
    line(x - 12, y + 10, x + 12, y + 10, "p")
    line(x - 7, y + 15, x + 7, y + 15, "p")
    line(x - 3, y + 20, x + 3, y + 20, "p")


def pwr(x, y, name, up=True):
    """Power flag: a stub and a bar with the rail name."""
    if up:
        line(x, y, x, y - 14)
        line(x - 10, y - 14, x + 10, y - 14, "p")
        text(x, y - 20, name, 12, "middle", "rail", "bold")
    else:
        line(x, y, x, y + 14)
        line(x - 10, y + 14, x + 10, y + 14, "p")
        text(x, y + 30, name, 12, "middle", "rail", "bold")


def nmos(cx, cy, ref, value):
    """N-channel MOSFET, gate on the RIGHT. Terminals:
    D = (cx-10, cy-40), S = (cx-10, cy+40), G = (cx+30, cy)."""
    emit(f'<circle cx="{cx - 1}" cy="{cy}" r="24" class="w" fill="none"/>')
    gx = cx + 12
    line(cx + 30, cy, gx, cy)                 # gate lead
    line(gx, cy - 15, gx, cy + 15, "p")       # gate plate
    for y0, y1 in ((cy - 18, cy - 8), (cy - 5, cy + 5), (cy + 8, cy + 18)):
        line(cx + 4, y0, cx + 4, y1, "p")     # enhancement channel segments
    wire((cx + 4, cy - 13), (cx - 10, cy - 13), (cx - 10, cy - 40))  # drain
    wire((cx + 4, cy + 13), (cx - 10, cy + 13), (cx - 10, cy + 40))  # source
    wire((cx - 10, cy), (cx + 4, cy))         # body to source
    line(cx - 10, cy, cx - 10, cy + 13)
    # N-channel: arrow points into the channel
    emit(f'<polygon points="{cx + 4},{cy} {cx - 4},{cy - 4} {cx - 4},{cy + 4}" class="fill"/>')
    label(cx + 30, cy - 46, ref, value)
    text(cx - 16, cy - 30, "D", 10, "end", "pin")
    text(cx - 16, cy + 38, "S", 10, "end", "pin")
    text(cx + 30, cy - 6, "G", 10, "end", "pin")


def npn(cx, cy, ref, value):
    """NPN BJT. Terminals: B = (cx-30, cy), C = (cx+10, cy-40), E = (cx+10, cy+40)."""
    emit(f'<circle cx="{cx}" cy="{cy}" r="24" class="w" fill="none"/>')
    bx = cx - 8
    line(cx - 30, cy, bx, cy)
    line(bx, cy - 14, bx, cy + 14, "p")
    wire((bx, cy - 6), (cx + 10, cy - 20), (cx + 10, cy - 40))
    wire((bx, cy + 6), (cx + 10, cy + 20), (cx + 10, cy + 40))
    # emitter arrow (pointing out)
    emit(f'<polygon points="{cx + 10},{cy + 20} {cx + 1},{cy + 19} {cx + 6},{cy + 12}" class="fill"/>')
    label(cx + 30, cy - 4, ref, value)
    text(cx - 32, cy - 6, "B", 10, "end", "pin")
    text(cx + 16, cy - 30, "C", 10, "start", "pin")
    text(cx + 16, cy + 38, "E", 10, "start", "pin")


def block(x, y, w, h, title, subtitle=None):
    emit(f'<rect x="{x}" y="{y}" width="{w}" height="{h}" rx="4" class="blk"/>')
    text(x + w / 2, y - 22 if subtitle else y - 8, title, 14, "middle", "ref", "bold")
    if subtitle:
        text(x + w / 2, y - 7, subtitle, 12, "middle", "val")


def pin_l(x, y, name, num=None, length=24):
    """Pin on the LEFT edge of a block at x; the wire attaches at x-length."""
    line(x - length, y, x, y)
    text(x + 6, y + 4, name, 12, "start", "pin")
    if num is not None:
        text(x - 4, y - 4, str(num), 10, "end", "num")
    return (x - length, y)


def pin_r(x, y, name, num=None, length=24):
    line(x, y, x + length, y)
    text(x - 6, y + 4, name, 12, "end", "pin")
    if num is not None:
        text(x + 4, y - 4, str(num), 10, "start", "num")
    return (x + length, y)


def note_box(x, y, w, lines, title):
    h = 26 + 16 * len(lines)
    emit(f'<rect x="{x}" y="{y}" width="{w}" height="{h}" rx="4" class="note"/>')
    text(x + 10, y + 18, title, 13, "start", "ref", "bold")
    for i, s in enumerate(lines):
        text(x + 10, y + 36 + 16 * i, s, 12, "start", "val")


def section(x, y, w, h, title):
    emit(f'<rect x="{x}" y="{y}" width="{w}" height="{h}" rx="6" class="sec"/>')
    text(x + 8, y + 16, title, 12, "start", "sec-t", "bold")


def connector_face(x, y):
    """J1 mating face: the fan-lead end of the cable, looking into its pins."""
    text(x, y, "J1 MATING FACE", 12, "start", "sec-t", "bold")
    text(x, y + 14, "fan lead end, looking into the", 11, "start", "val")
    text(x, y + 27, "contacts, chamfered corners UP", 11, "start", "val")
    bx, by, bw, bh = x + 10, y + 40, 200, 50
    c = 12  # chamfer
    pts = [(bx + c, by), (bx + bw - c, by), (bx + bw, by + c), (bx + bw, by + bh),
           (bx, by + bh), (bx, by + c)]
    emit('<polygon points="%s" class="blk"/>' % " ".join(f"{a},{b}" for a, b in pts))
    names = {1: "TACH?", 2: "PWM", 3: "GND", 4: "+10V"}
    for i in range(4):
        px = bx + 25 + i * 50
        emit(f'<circle cx="{px}" cy="{by + 25}" r="8" class="w" fill="#ffffff"/>')
        text(px, by + 29, str(i + 1), 10, "middle", "num", "bold")
        text(px, by + bh + 16, names[i + 1], 11, "middle", "pin")
    text(x, by + bh + 34, "Pin numbers follow the builder report", 10, "start", "val")
    text(x, by + bh + 46, "(PINOUT.md). Identify by MEASURING.", 10, "start", "warn")


# --------------------------------------------------------------------------
# Sheet
# --------------------------------------------------------------------------
def draw():
    # Section backdrops (drawn first so everything sits on top)
    section(30, 60, 640, 230, "POWER INPUT CONDITIONING  (fan +10 V -> Nano VIN)")
    section(910, 130, 300, 190, "3.3 V SUPPLY FOR OLED MODULE")

    # ---------------- J1: fan connector ----------------
    jx, jy = 60, 330
    emit(f'<rect x="{jx}" y="{jy}" width="70" height="230" rx="4" class="blk"/>')
    text(jx + 35, jy - 22, "J1", 14, "middle", "ref", "bold")
    text(jx + 35, jy - 7, "to fan cable", 12, "middle", "val")
    text(jx + 35, jy + 250, "4-pin Molex", 12, "middle", "val")
    text(jx + 35, jy + 265, "(8981, 5.08 mm)", 12, "middle", "val")
    text(jx + 35, jy + 280, "VERIFY pins!", 12, "middle", "warn", "bold")
    text(jx + 35, jy + 295, "face view: below", 11, "middle", "val")
    j4 = pin_r(jx + 70, 360, "+10V", 4, 40)
    j3 = pin_r(jx + 70, 410, "GND", 3, 40)
    j2 = pin_r(jx + 70, 480, "PWM", 2, 40)
    j1 = pin_r(jx + 70, 520, "TACH", 1, 40)

    # ---------------- Power conditioning ----------------
    ry = 140
    wire(j4, (170, 360), (170, ry), (190, ry))
    ptc_h(190, 270, ry, "F1", "PTC 0.2 A hold")
    diode_h(270, 350, ry, "D1", "1N5817")
    wire((350, ry), (640, ry))
    for x in (400, 480, 580):
        dot(x, ry)
    tvs_v(400, ry, 230, "D2", "P6KE13A", side="left")
    gnd(400, 230)
    cap_v(480, ry, 230, "C1", "220µF 25V", polarized=True)
    gnd(480, 230)
    cap_v(580, ry, 230, "C2", "100nF")
    gnd(580, 230)
    pwr(640, ry, "+10V_P")
    text(190, 270, "Rev-polarity (D1), surge clamp (D2), short protection (F1),", 11, "start", "val")
    text(190, 284, "bulk/hold-up + HF decoupling (C1, C2). ~9.6 V reaches the Nano VIN.", 11, "start", "val")

    # J1 ground
    wire(j3, (190, 410))
    gnd(190, 410)

    # ---------------- Arduino Nano ----------------
    nx, ny, nw, nh = 720, 320, 150, 480
    block(nx, ny, nw, nh, "U1  Arduino Nano", "ATmega328P, 5 V / 16 MHz")
    n_vin = pin_l(nx, 360, "VIN", 30)
    n_gnd_l = pin_l(nx, 400, "GND", 29)
    n_d9 = pin_l(nx, 520, "D9 / OC1A", 12)
    n_d2 = pin_l(nx, 700, "D2 / INT0", 5)
    n_5v = pin_r(nx + nw, 360, "+5V", 27)
    n_gnd_r = pin_r(nx + nw, 400, "GND", 4)
    n_sda = pin_r(nx + nw, 460, "A4 / SDA", 23)
    n_scl = pin_r(nx + nw, 500, "A5 / SCL", 24)
    n_d3 = pin_r(nx + nw, 560, "D3", 6)
    n_d4 = pin_r(nx + nw, 600, "D4", 7)
    n_d5 = pin_r(nx + nw, 640, "D5", 8)
    n_d6 = pin_r(nx + nw, 680, "D6", 9)
    n_d7 = pin_r(nx + nw, 720, "D7", 10)
    text(nx + nw / 2, ny + nh - 34, "D13: on-board LED", 11, "middle", "val")
    text(nx + nw / 2, ny + nh - 20, "(lit = fan ON)", 11, "middle", "val")
    text(nx + nw / 2, ny + nh - 6, "USB: programming only", 11, "middle", "val")

    wire(n_vin, (680, 360))
    pwr(680, 360, "+10V_P")
    wire(n_gnd_l, (680, 400))
    gnd(680, 400)
    wire(n_5v, (910, 360))
    pwr(910, 360, "+5V")
    wire(n_gnd_r, (925, 400))
    gnd(925, 400)

    # ---------------- Fan speed output: Q1 ----------------
    # Drain on the J1 PWM line, gate driven from D9 through R3, R4 holds the
    # gate high (fan OFF) while D9 is high-impedance during reset/boot.
    qx, qy = 330, 520
    nmos(qx, qy, "Q1", "2N7000")
    wire(j2, (qx - 10, 480))
    dot(260, 480)
    resistor_v(260, 480, 380, "R5", "4.7k  DNP", side="right", dnp=True)
    pwr(260, 380, "+10V_P")
    wire((qx - 10, qy + 40), (qx - 10, 570))
    gnd(qx - 10, 570)
    resistor_h(qx + 30, 470, qy, "R3", "220Ω", lab_dy=18)
    wire((470, qy), n_d9)
    dot(540, qy)
    resistor_v(540, qy, 450, "R4", "10k", side="right", body_c=480)
    pwr(540, 450, "+5V")
    text(380, 448, "FAN SPEED OUTPUT", 12, "start", "sec-t", "bold")
    text(440, 572, "D9 HIGH = Q1 on = line LOW", 11, "start", "val")
    text(440, 585, "= fan slower / stopped.", 11, "start", "val")
    text(440, 598, "R4: fan OFF during boot.", 11, "start", "val")

    # ---------------- Tach input: 5 V pull-up, series R, clamps ----------
    # The fan side only ever sees R7. The D2 node is pulled up to +5V (never
    # to the fan's 10 V) and clamped to 0..5 V by D3/D4.
    ty = 700
    wire(j1, (170, 520), (170, ty), (200, ty))
    resistor_h(200, 290, ty, "R7", "1k", lab_dy=-30)
    wire((290, ty), n_d2)
    dot(360, ty)
    resistor_v(360, ty, 630, "R6", "10k", side="left", body_c=660)
    pwr(360, 630, "+5V")
    dot(430, ty)
    schottky_v(430, 630, ty, "D3", "BAT85", side="right")
    pwr(430, 630, "+5V")
    dot(500, ty)
    schottky_v(500, ty, 770, "D4", "BAT85", side="right")
    gnd(500, 770)
    dot(590, ty)
    cap_v(590, ty, 770, "C5", "10nF", side="right")
    gnd(590, 770)
    text(180, 800, "TACH INPUT  (pin 1 function UNCONFIRMED)", 12, "start", "sec-t", "bold")
    text(180, 815, "Accepts open-collector or push-pull 0-12 V. D2 sees only 0-5 V:", 11, "start", "val")
    text(180, 829, "R6 pulls up to +5V, R7 limits clamp current to <5 mA into D3/D4.", 11, "start", "val")

    # ---------------- 3.3 V regulator U2 ----------------
    ux, uy = 1040, 190
    emit(f'<rect x="{ux}" y="{uy}" width="90" height="60" rx="4" class="blk"/>')
    text(ux + 45, uy + 20, "U2", 13, "middle", "ref", "bold")
    text(ux + 45, uy + 36, "MCP1700", 12, "middle", "val")
    text(ux + 45, uy + 50, "-3302E", 12, "middle", "val")
    text(ux - 4, uy + 16, "VIN", 10, "end", "pin")
    text(ux + 94, uy + 16, "VOUT", 10, "start", "pin")
    line(ux, 220, ux - 50, 220)
    line(ux + 90, 220, ux + 140, 220)
    line(ux + 45, uy + 60, ux + 45, 270)
    text(ux + 50, uy + 72, "GND", 10, "start", "pin")
    gnd(ux + 45, 270)
    pwr(ux - 50, 220, "+5V")
    dot(ux - 25, 220)
    cap_v(ux - 25, 220, 280, "C3", "1µF", side="left")
    gnd(ux - 25, 280)
    dot(ux + 115, 220)
    cap_v(ux + 115, 220, 280, "C4", "1µF")
    gnd(ux + 115, 280)
    pwr(ux + 140, 220, "+3V3")

    # ---------------- OLED / encoder module ----------------
    mx, my, mw, mh = 1250, 400, 180, 360
    block(mx, my, mw, mh, "U3  AITRIP 1.3\" OLED", "SH1106 128x64 I2C + EC11")
    m_vcc = pin_l(mx, 420, "VCC (3.3 V)")
    m_sda = pin_l(mx, 460, "SDA")
    m_scl = pin_l(mx, 500, "SCL")
    m_tra = pin_l(mx, 560, "TRA (enc A)")
    m_trb = pin_l(mx, 600, "TRB (enc B)")
    m_psh = pin_l(mx, 640, "PSH (push)")
    m_con = pin_l(mx, 680, "CON / KEY1")
    m_bak = pin_l(mx, 720, "BAK / KEY0")
    m_gnd = pin_l(mx, 745, "GND")
    text(mx + mw / 2, my + mh + 18, "keys switch to GND", 11, "middle", "val")
    text(mx + mw / 2, my + mh + 32, "I2C address 0x3C", 11, "middle", "val")

    wire(m_vcc, (1200, 420))
    pwr(1200, 420, "+3V3")
    wire(m_gnd, (1200, 745))
    gnd(1200, 745)

    wire(n_sda, m_sda)
    wire(n_scl, m_scl)
    wire(n_d3, m_tra)
    wire(n_d4, m_trb)
    wire(n_d5, m_psh)
    wire(n_d6, m_con)
    wire(n_d7, m_bak)
    for (x, y), name in ((n_sda, "SDA"), (n_scl, "SCL"), (n_d3, "ENC_A"), (n_d4, "ENC_B"),
                         (n_d5, "ENC_SW"), (n_d6, "KEY_CON (opt.)"), (n_d7, "KEY_BAK (opt.)")):
        text(1140, y - 5, name, 11, "middle", "net")

    # I2C pull-ups to 3.3 V (R2's lead hops over the SDA wire)
    dot(990, 460)
    resistor_v(990, 460, 390, "R1", "2.2k", side="left", body_c=420)
    pwr(990, 390, "+3V3")
    dot(1060, 500)
    resistor_v_hop(1060, 390, 500, 420, [460], "R2", "2.2k")
    pwr(1060, 390, "+3V3")

    # ---------------- Notes and title ----------------
    note_box(30, 860, 640, [
        "1. J1 pin functions come from builder reports, not AC Infinity. Measure your fan first",
        "   (README: Pre-connection verification). Fit R5 only if PWM does not idle near +10 V.",
        "2. TO-92 pinouts, flat face towards you, legs down: 2N7000 = S-G-D, MCP1700 = GND-VIN-VOUT.",
        "3. U3 pin ORDER differs between batches - wire by the printed labels, not by position.",
        "4. Resistors 1/4 W 5 %. C1 electrolytic >= 25 V. Diode band = cathode. DNP = not fitted.",
        "5. Full-size wiring view with wire colours: hardware/wiring.svg.",
    ], "NOTES")
    connector_face(690, 870)
    tbx, tby = 940, 860
    emit(f'<rect x="{tbx}" y="{tby}" width="530" height="130" rx="4" class="blk"/>')
    text(tbx + 14, tby + 26, "Cloudline AI-CLS6 fan controller", 17, "start", "ref", "bold")
    text(tbx + 14, tby + 46, "Arduino Nano + AITRIP 1.3\" OLED / EC11 encoder", 13, "start", "val")
    text(tbx + 14, tby + 72, "Sheet 1/1  ·  Rev B  ·  2026-10-28", 12, "start", "val")
    text(tbx + 14, tby + 90, "Firmware: firmware/cloudline_controller", 12, "start", "val")
    text(tbx + 14, tby + 108, "Generated by hardware/schematic.py", 12, "start", "val")
    text(tbx + 516, tby + 120, "github.com/seesharp15/cloudline-fan-controller", 10, "end", "val")


CSS = """
<style>
  .bg { fill: #ffffff; }
  .w { stroke: #1d2733; stroke-width: 2; stroke-linecap: round; stroke-linejoin: round; }
  .p { stroke: #1d2733; stroke-width: 3; stroke-linecap: round; }
  .dnp { stroke: #8a96a3; stroke-width: 2; stroke-dasharray: 4 3; }
  .fill { fill: #1d2733; }
  .dot { fill: #1d2733; }
  .blk { fill: #f4f7fb; stroke: #1d2733; stroke-width: 2; }
  .note { fill: #fffbea; stroke: #c9a227; stroke-width: 1.5; }
  .sec { fill: none; stroke: #b8c4d2; stroke-width: 1.2; stroke-dasharray: 6 4; }
  text { font-family: "Helvetica Neue", Arial, sans-serif; }
  .t, .pin { fill: #1d2733; }
  .ref { fill: #0b3d75; }
  .val { fill: #3b4754; }
  .num { fill: #7a4b00; font-family: Menlo, Consolas, monospace; }
  .rail { fill: #b3261e; }
  .net { fill: #2e6b2e; font-style: italic; }
  .warn { fill: #b3261e; }
  .sec-t { fill: #6b7a8a; letter-spacing: 0.5px; }
</style>
"""


def svg_to_png(svg_path, png_path, width):
    """Render an SVG to PNG with rsvg-convert, or cairosvg as a fallback."""
    if shutil.which("rsvg-convert"):
        subprocess.run(["rsvg-convert", "-w", str(width), "-b", "white", svg_path,
                        "-o", png_path], check=True)
    else:
        try:
            import cairosvg
        except ImportError:
            print("PNG skipped: install librsvg (rsvg-convert) or `pip install cairosvg`")
            return
        cairosvg.svg2png(url=svg_path, write_to=png_path, output_width=width,
                         background_color="white")
    print(f"wrote {png_path}")


def main():
    draw()
    svg = [f'<svg xmlns="http://www.w3.org/2000/svg" width="{W}" height="{H}" viewBox="0 0 {W} {H}">',
           CSS, f'<rect class="bg" width="{W}" height="{H}"/>'] + out + ["</svg>"]
    here = os.path.dirname(os.path.abspath(__file__))
    path = sys.argv[1] if len(sys.argv) > 1 else os.path.join(here, "schematic.svg")
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "w") as f:
        f.write("\n".join(svg) + "\n")
    print(f"wrote {path}")
    svg_to_png(path, os.path.splitext(path)[0] + ".png", 2 * W)


if __name__ == "__main__":
    main()
