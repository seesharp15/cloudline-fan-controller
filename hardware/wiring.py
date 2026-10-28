#!/usr/bin/env python3
"""Generate hardware/wiring.svg (and .png): the wiring view for builders.

Unlike the schematic, this drawing shows the parts where they physically are:
the fan-cable plug, the small interface board (perfboard), the Arduino Nano
with its pins in their real order, and the OLED/encoder module. Every wire is
drawn in the colour suggested for it. Run from the repo root:

    python3 hardware/wiring.py      # writes hardware/wiring.svg + .png
"""

import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from schematic import svg_to_png  # noqa: E402

W, H = 1500, 1180
out = []

# Suggested wire colours (name, stroke). White is drawn light grey.
COL = {
    "+10V": ("red", "#d32f2f"),
    "GND": ("black", "#212121"),
    "PWM": ("yellow", "#f2c200"),
    "TACH": ("green", "#2e7d32"),
    "VIN": ("red", "#d32f2f"),
    "+5V": ("orange", "#ef6c00"),
    "+3V3": ("pink", "#e05297"),
    "SDA": ("blue", "#1565c0"),
    "SCL": ("white", "#eeeeee"),
    "GATE": ("brown", "#795548"),
    "TACH_D2": ("light green", "#9ccc65"),
    "TRA": ("purple", "#7b1fa2"),
    "TRB": ("grey", "#8d8d8d"),
    "PSH": ("cyan", "#00acc1"),
    "CON": ("light blue", "#64b5f6"),
    "BAK": ("violet", "#ce93d8"),
}


def emit(s):
    out.append(s)


def esc(s):
    return s.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;")


def text(x, y, s, size=13, anchor="start", cls="t", weight="normal"):
    emit(f'<text x="{x}" y="{y}" font-size="{size}" text-anchor="{anchor}" '
         f'font-weight="{weight}" class="{cls}">{esc(s)}</text>')


def wire(net, pts, hops_v=()):
    """Coloured wire through `pts`. `hops_v` = y values where the LAST vertical
    segment jumps over other wires."""
    colour = COL[net][1]
    d = f"M{pts[0][0]},{pts[0][1]}"
    for i, (x, y) in enumerate(pts[1:], start=1):
        px, py = pts[i - 1]
        if hops_v and i == len(pts) - 1 and px == x:
            sgn = 1 if y > py else -1
            for hy in sorted(hops_v, reverse=(sgn < 0)):
                d += f" L{x},{hy - 8 * sgn} A8,8 0 0,{1 if sgn > 0 else 0} {x},{hy + 8 * sgn}"
        d += f" L{x},{y}"
    emit(f'<path d="{d}" fill="none" stroke="#1b1b1b" stroke-width="8" '
         f'stroke-linejoin="round" stroke-linecap="round"/>')
    emit(f'<path d="{d}" fill="none" stroke="{colour}" stroke-width="5" '
         f'stroke-linejoin="round" stroke-linecap="round"/>')


def pad(x, y, r=5):
    emit(f'<circle cx="{x}" cy="{y}" r="{r}" fill="#d4af37" stroke="#5c4a00" stroke-width="1.5"/>')


def box(x, y, w, h, cls, rx=8):
    emit(f'<rect x="{x}" y="{y}" width="{w}" height="{h}" rx="{rx}" class="{cls}"/>')


# --------------------------------------------------------------------------
def draw():
    text(30, 40, "Cloudline AI-CLS6 controller - wiring view", 22, "start", "title", "bold")
    text(30, 62, "Physical layout and suggested wire colours. Electrical detail and part values: "
         "hardware/schematic.svg. Pin numbers: PINOUT.md.", 13, "start", "sub")

    # ---------------- J1: fan-lead plug ----------------
    jx, jy = 40, 250
    text(jx, jy - 60, "J1  FAN CABLE PLUG", 14, "start", "ref", "bold")
    text(jx, jy - 44, "cut-off end of a 4-pin Molex", 12, "start", "sub")
    text(jx, jy - 30, "extension (plugs into the fan lead)", 12, "start", "sub")
    box(jx, jy, 110, 250, "plug", 6)
    jpins = {4: ("+10V", 300), 3: ("GND", 360), 2: ("PWM", 420), 1: ("TACH", 480)}
    for num, (name, y) in jpins.items():
        emit(f'<circle cx="{jx + 80}" cy="{y}" r="11" fill="#fff" stroke="#333" stroke-width="2"/>')
        text(jx + 80, y + 5, str(num), 12, "middle", "num", "bold")
        text(jx + 12, y + 5, name, 12, "start", "t", "bold")
    text(jx, jy + 270, "Pin numbers per builder report;", 11, "start", "sub")
    text(jx, jy + 284, "fan-side wire colours are NOT", 11, "start", "warn")
    text(jx, jy + 298, "documented: identify each pin by", 11, "start", "warn")
    text(jx, jy + 312, "MEASURING, then label the wires.", 11, "start", "warn")

    # ---------------- Interface board ----------------
    bx, by, bw, bh = 330, 150, 320, 650
    box(bx, by, bw, bh, "board")
    text(bx + bw / 2, by - 12, "INTERFACE BOARD (perfboard, ~5 x 7 cm)", 14, "middle", "ref", "bold")
    sections = [
        (by + 30, "POWER", ["F1 PTC 0.2 A  ->  D1 1N5817 (band ->)", "D2 P6KE13A, C1 220uF, C2 100nF",
                             "to GND: gives protected +10V_P"]),
        (by + 120, "SPEED OUTPUT", ["Q1 2N7000 (S-G-D), R3 220R D9 -> gate", "R4 10k D9 end of R3 -> +5V (OFF at boot)",
                                     "R5 4.7k to +10V_P: DNP"]),
        (by + 210, "TACH INPUT", ["R7 1k series, R6 10k -> +5V", "D3 BAT85 -> +5V, D4 BAT85 -> GND",
                                   "C5 10nF -> GND"]),
        (by + 300, "3.3 V AND I2C FOR OLED", ["U2 MCP1700-3302E (GND-VIN-VOUT), C3, C4 1uF",
                                               "U4 level shifter: HV = 5V (Nano),",
                                               "LV = 3V3 (OLED); R1, R2 2.2k -> +3V3"]),
    ]
    for y, title, lines in sections:
        text(bx + 16, y, title, 12, "start", "sec", "bold")
        for i, s in enumerate(lines):
            text(bx + 16, y + 17 + 15 * i, s, 12, "start", "t")
    text(bx + 16, by + 410, "Build and test this board first", 12, "start", "warn", "bold")
    text(bx + 16, by + 425, "(README steps 2-3) before wiring", 12, "start", "warn", "bold")
    text(bx + 16, by + 440, "the Nano to it.", 12, "start", "warn", "bold")

    # left terminals (from J1)
    left_terms = {"+10V": 300, "GND": 360, "PWM": 420, "TACH": 480}
    for name, y in left_terms.items():
        pad(bx, y)
        text(bx - 10, y - 9, {"+10V": "FAN +10V", "GND": "FAN GND", "PWM": "FAN PWM",
                              "TACH": "FAN TACH"}[name], 11, "end", "term")
    # right terminals (to Nano)
    rt = {"TACH_D2": 200, "GATE": 300, "SDA": 640, "SCL": 670, "+5V": 700, "GND": 730, "VIN": 760}
    rt_label = {"TACH_D2": "TACH OUT", "GATE": "GATE IN", "SDA": "SDA (5V side)", "SCL": "SCL (5V side)",
                "+5V": "+5V IN", "GND": "GND", "VIN": "+10V_P OUT"}
    for name, y in rt.items():
        pad(bx + bw, y)
        text(bx + bw + 12, y - 9, rt_label[name], 11, "start", "term")
    # bottom terminals (to OLED)
    bt = {"+3V3": 550, "GND": 500, "SDA": 450, "SCL": 400}
    for name, x in bt.items():
        pad(x, by + bh)
        text(x, by + bh - 10, {"+3V3": "3V3", "GND": "GND", "SDA": "SDA3", "SCL": "SCL3"}[name],
             11, "middle", "term")

    # J1 -> board
    for name, y in jpins.values():
        wire(name, [(jx + 91, y), (bx, y)])

    # ---------------- Arduino Nano (top view, USB left) ----------------
    nx, ny, nw, nh = 770, 380, 340, 180
    box(nx, ny, nw, nh, "nano", 6)
    emit(f'<rect x="{nx - 22}" y="{ny + 65}" width="34" height="50" rx="4" fill="#b0b8c0" stroke="#333"/>')
    text(nx - 40, ny + 95, "USB", 11, "end", "sub")
    text(nx + nw / 2, ny + 85, "ARDUINO NANO", 16, "middle", "chip", "bold")
    text(nx + nw / 2, ny + 103, "top view, components up, USB left", 11, "middle", "chipsub")
    text(nx + nw / 2, ny + 118, "check against the silkscreen", 11, "middle", "chipsub")
    top = ["D12", "D11", "D10", "D9", "D8", "D7", "D6", "D5", "D4", "D3", "D2", "GND", "RST",
           "RX0", "TX1"]
    bot = ["D13", "3V3", "REF", "A0", "A1", "A2", "A3", "A4", "A5", "A6", "A7", "5V", "RST",
           "GND", "VIN"]
    px = {}
    for i, n in enumerate(top):
        x = nx + 30 + 20 * i
        pad(x, ny + 20, 6)
        emit(f'<text x="{x + 3}" y="{ny + 34}" font-size="10" class="pinv" '
             f'transform="rotate(-90 {x + 3} {ny + 34})" text-anchor="end">{n}</text>')
        px[("T", n)] = x
    for i, n in enumerate(bot):
        x = nx + 30 + 20 * i
        pad(x, ny + nh - 20, 6)
        emit(f'<text x="{x + 3}" y="{ny + nh - 34}" font-size="10" class="pinv" '
             f'transform="rotate(-90 {x + 3} {ny + nh - 34})" text-anchor="start">{n}</text>')
        px[("B", n)] = x
    ty, byy = ny + 20, ny + nh - 20

    # ---------------- OLED / encoder module ----------------
    ox, oy, ow, oh = 1250, 190, 210, 450
    box(ox, oy, ow, oh, "pcb", 6)
    emit(f'<rect x="{ox + 40}" y="{oy + 20}" width="150" height="80" fill="#0b0f14" stroke="#555"/>')
    text(ox + 115, oy + 65, "1.3\" OLED", 13, "middle", "oled")
    emit(f'<circle cx="{ox + 115}" cy="{oy + 330}" r="42" fill="#9aa4ad" stroke="#333" stroke-width="2"/>')
    emit(f'<circle cx="{ox + 115}" cy="{oy + 330}" r="14" fill="#c9d0d6" stroke="#333"/>')
    text(ox + 115, oy + 395, "EC11 knob", 12, "middle", "chipsub")
    text(ox + ow / 2, oy - 26, "U3  AITRIP 1.3\" OLED + EC11", 14, "middle", "ref", "bold")
    text(ox + ow / 2, oy - 11, "pin ORDER varies: wire by the LABELS", 12, "middle", "warn")
    opins = {"BAK": 240, "CON": 270, "PSH": 300, "TRB": 330, "TRA": 360,
             "VCC": 520, "GND": 550, "SDA": 580, "SCL": 610}
    for n, y in opins.items():
        pad(ox, y)
        text(ox + 10, y + 4, n, 11, "start", "term")

    # ---------------- Wires: board <-> Nano ----------------
    # D2 (tach): runs above the encoder wires and hops over them.
    wire("TACH_D2", [(bx + bw, rt["TACH_D2"]), (px[("T", "D2")], rt["TACH_D2"]),
                     (px[("T", "D2")], ty)], hops_v=(240, 270, 300, 330, 360))
    wire("GATE", [(bx + bw, rt["GATE"]), (px[("T", "D9")], rt["GATE"]), (px[("T", "D9")], ty)])
    for net, pin in (("SDA", "A4"), ("SCL", "A5"), ("+5V", "5V"), ("GND", "GND"), ("VIN", "VIN")):
        wire(net, [(px[("B", pin)], byy), (px[("B", pin)], rt[net]), (bx + bw, rt[net])])

    # ---------------- Wires: Nano <-> OLED (encoder and keys) ----------------
    for net, pin in (("BAK", "D7"), ("CON", "D6"), ("PSH", "D5"), ("TRB", "D4"), ("TRA", "D3")):
        y = opins[net]
        wire(net, [(px[("T", pin)], ty), (px[("T", pin)], y), (ox, y)])

    # ---------------- Wires: board -> OLED (power and I2C) ----------------
    route = {"+3V3": (840, 1140, "VCC"), "GND": (870, 1170, "GND"),
             "SDA": (900, 1200, "SDA"), "SCL": (930, 1230, "SCL")}
    for net, (depth, vx, pin) in route.items():
        x0 = bt[net]
        wire(net, [(x0, by + bh), (x0, depth), (vx, depth), (vx, opins[pin]), (ox, opins[pin])])

    # ---------------- Legend ----------------
    lx, ly = 30, 1010
    text(lx, ly - 20, "WIRE COLOURS (suggested)", 13, "start", "ref", "bold")
    legend = [
        ("+10V", "+10 V from fan / +10V_P to Nano VIN"), ("GND", "ground (all GNDs are common)"),
        ("PWM", "fan speed line (Q1 drain)"), ("TACH", "fan tach (unconfirmed pin)"),
        ("+5V", "Nano 5V -> board (R4, R6, D3, U2, U4)"), ("+3V3", "3.3 V to OLED VCC"),
        ("SDA", "I2C data (A4 / U4 / OLED)"), ("SCL", "I2C clock (A5 / U4 / OLED)"), ("GATE", "D9 -> R3 -> Q1 gate"),
        ("TACH_D2", "conditioned tach -> D2"), ("TRA", "encoder A -> D3"), ("TRB", "encoder B -> D4"),
        ("PSH", "encoder push -> D5"), ("CON", "CON key -> D6 (optional)"),
        ("BAK", "BAK key -> D7 (optional)"),
    ]
    for i, (net, desc) in enumerate(legend):
        col, row = i // 5, i % 5
        x, y = lx + col * 360, ly + row * 24
        emit(f'<line x1="{x}" y1="{y - 4}" x2="{x + 40}" y2="{y - 4}" stroke="#1b1b1b" stroke-width="8" stroke-linecap="round"/>')
        emit(f'<line x1="{x}" y1="{y - 4}" x2="{x + 40}" y2="{y - 4}" stroke="{COL[net][1]}" stroke-width="5" stroke-linecap="round"/>')
        text(x + 52, y, f"{COL[net][0]}: {desc}", 12, "start", "t")
    text(1110, ly, "Wires: 24-26 AWG stranded for signals,", 12, "start", "t")
    text(1110, ly + 16, "22 AWG for +10V / GND. Keep the fan", 12, "start", "t")
    text(1110, ly + 32, "cable under 50 cm, OLED wires 30 cm.", 12, "start", "t")
    text(1110, ly + 56, "A wire hop (arc) means the wires", 12, "start", "t")
    text(1110, ly + 72, "cross WITHOUT connecting.", 12, "start", "t")


CSS = """
<style>
  text { font-family: "Helvetica Neue", Arial, sans-serif; }
  .t { fill: #1d2733; }
  .title { fill: #0b3d75; }
  .sub { fill: #4a5663; }
  .ref { fill: #0b3d75; }
  .sec { fill: #6b7a8a; letter-spacing: 0.5px; }
  .warn { fill: #b3261e; }
  .num { fill: #1d2733; }
  .term { fill: #1d2733; font-weight: bold; }
  .pinv { fill: #f0f0f0; font-family: Menlo, Consolas, monospace; }
  .chip { fill: #ffffff; }
  .chipsub { fill: #d8e2ea; }
  .oled { fill: #6fd3ff; font-family: Menlo, Consolas, monospace; }
  .board { fill: #f3e7c6; stroke: #a08a4f; stroke-width: 2; }
  .plug { fill: #f7f7f2; stroke: #333; stroke-width: 2; }
  .nano { fill: #1f5fa8; stroke: #0d2f57; stroke-width: 2; }
  .pcb { fill: #1f6b3a; stroke: #0e3a1d; stroke-width: 2; }
</style>
"""


def main():
    draw()
    svg = [f'<svg xmlns="http://www.w3.org/2000/svg" width="{W}" height="{H}" viewBox="0 0 {W} {H}">',
           CSS, f'<rect fill="#ffffff" width="{W}" height="{H}"/>'] + out + ["</svg>"]
    here = os.path.dirname(os.path.abspath(__file__))
    path = os.path.join(here, "wiring.svg")
    with open(path, "w") as f:
        f.write("\n".join(svg) + "\n")
    print(f"wrote {path}")
    svg_to_png(path, os.path.splitext(path)[0] + ".png", 2 * W)


if __name__ == "__main__":
    main()
