#!/usr/bin/env python3
"""Generate hardware/overview.svg (and .png): an illustration of the finished
build for the README - the fan, its control cable, and the controller box.

    python3 hardware/overview.py
"""

import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from schematic import svg_to_png  # noqa: E402

W, H = 1440, 620


def main():
    o = []
    a = o.append
    a(f'<svg xmlns="http://www.w3.org/2000/svg" width="{W}" height="{H}" viewBox="0 0 {W} {H}">')
    a("""<defs>
  <linearGradient id="fanBody" x1="0" y1="0" x2="0" y2="1">
    <stop offset="0" stop-color="#4a4f57"/><stop offset="0.45" stop-color="#2a2e34"/>
    <stop offset="1" stop-color="#15181c"/></linearGradient>
  <linearGradient id="duct" x1="0" y1="0" x2="0" y2="1">
    <stop offset="0" stop-color="#d9dde2"/><stop offset="0.5" stop-color="#aab2bb"/>
    <stop offset="1" stop-color="#7d8792"/></linearGradient>
  <linearGradient id="box" x1="0" y1="0" x2="0" y2="1">
    <stop offset="0" stop-color="#3b4048"/><stop offset="1" stop-color="#1e2126"/></linearGradient>
  <linearGradient id="knob" x1="0" y1="0" x2="1" y2="1">
    <stop offset="0" stop-color="#e6e9ec"/><stop offset="1" stop-color="#7c858e"/></linearGradient>
  <filter id="sh" x="-20%" y="-20%" width="140%" height="160%">
    <feDropShadow dx="0" dy="8" stdDeviation="8" flood-opacity="0.35"/></filter>
</defs>
<style>
  text { font-family: "Helvetica Neue", Arial, sans-serif; }
  .lbl { fill: #1d2733; font-size: 15px; }
  .lblb { fill: #0b3d75; font-size: 17px; font-weight: bold; }
  .scr { fill: #8fe3ff; font-family: Menlo, Consolas, monospace; }
</style>""")
    a(f'<rect width="{W}" height="{H}" fill="#f6f7f9"/>')
    a('<text x="40" y="50" class="lblb" font-size="22">Finished build: overview</text>')
    a('<text x="40" y="74" class="lbl">Illustration, not to scale. The controller replaces the stock AC Infinity '
      'controller and is powered by the fan.</text>')

    # Ducting and fan
    a('<rect x="40" y="210" width="140" height="120" rx="10" fill="url(#duct)" stroke="#6b7580"/>')
    a('<rect x="560" y="210" width="140" height="120" rx="10" fill="url(#duct)" stroke="#6b7580"/>')
    for x in range(52, 180, 14):
        a(f'<line x1="{x}" y1="212" x2="{x}" y2="328" stroke="#8e98a3" stroke-width="2"/>')
        a(f'<line x1="{x + 520}" y1="212" x2="{x + 520}" y2="328" stroke="#8e98a3" stroke-width="2"/>')
    a('<g filter="url(#sh)"><rect x="170" y="170" width="400" height="200" rx="40" fill="url(#fanBody)"/></g>')
    a('<rect x="190" y="185" width="360" height="12" rx="6" fill="#5c626b" opacity="0.6"/>')
    a('<text x="370" y="262" text-anchor="middle" fill="#e8ecf0" font-size="26" font-weight="bold">inline EC fan</text>')
    a('<text x="370" y="290" text-anchor="middle" fill="#aeb6bf" font-size="16">(Cloudline AI-CLS6)</text>')
    # Mains cord
    a('<path d="M260,370 C260,450 200,470 140,520" fill="none" stroke="#111" stroke-width="7"/>')
    a('<rect x="104" y="510" width="52" height="40" rx="6" fill="#222"/>')
    a('<rect x="114" y="550" width="6" height="16" fill="#bbb"/><rect x="140" y="550" width="6" height="16" fill="#bbb"/>')
    a('<text x="170" y="545" class="lbl">mains cord (to outlet)</text>')
    # Control lead + Molex
    a('<path d="M470,370 C480,440 560,470 660,470" fill="none" stroke="#111" stroke-width="6"/>')
    a('<g filter="url(#sh)"><path d="M660,452 h56 l8,8 v20 l-8,8 h-56 z" fill="#f2f2ec" stroke="#555"/></g>')
    a('<g filter="url(#sh)"><path d="M724,454 h52 v32 h-52 z" fill="#fbfbf6" stroke="#555"/></g>')
    a('<text x="690" y="520" class="lbl" text-anchor="middle">fan control lead,</text>')
    a('<text x="690" y="538" class="lbl" text-anchor="middle">4-pin Molex</text>')
    a('<text x="780" y="520" class="lbl">J1 (cut extension cable)</text>')
    # Cable to controller (4 wires bundled)
    a('<path d="M776,470 C860,470 880,420 940,400" fill="none" stroke="#1b1b1b" stroke-width="10"/>')
    a('<path d="M776,470 C860,470 880,420 940,400" fill="none" stroke="#3a3a3a" stroke-width="6"/>')

    # Controller box
    bx, by = 930, 150
    a(f'<g filter="url(#sh)"><rect x="{bx}" y="{by}" width="440" height="330" rx="18" fill="url(#box)"/></g>')
    a(f'<rect x="{bx + 12}" y="{by + 10}" width="416" height="10" rx="5" fill="#5a606a" opacity="0.6"/>')
    # OLED window with the real main screen layout (simplified)
    sx, sy, sw, sh = bx + 40, by + 45, 256, 128
    a(f'<rect x="{sx - 8}" y="{sy - 8}" width="{sw + 16}" height="{sh + 16}" rx="6" fill="#0d0f12"/>')
    a(f'<rect x="{sx}" y="{sy}" width="{sw}" height="{sh}" fill="#000"/>')
    a(f'<rect x="{sx}" y="{sy}" width="{sw}" height="28" fill="#8fe3ff"/>')
    a(f'<text x="{sx + 6}" y="{sy + 21}" font-size="20" font-weight="bold" fill="#000" '
      f'font-family="Menlo, Consolas, monospace">FAN ON</text>')
    a(f'<text x="{sx + sw - 6}" y="{sy + 20}" font-size="15" fill="#000" text-anchor="end" '
      f'font-family="Menlo, Consolas, monospace">897 RPM</text>')
    for i, (lab, pct) in enumerate((("SET", 50), ("ACT", 49))):
        y = sy + 38 + i * 34
        a(f'<text x="{sx}" y="{y + 18}" font-size="15" class="scr">{lab}</text>')
        a(f'<rect x="{sx + 44}" y="{y + 2}" width="160" height="22" fill="none" stroke="#8fe3ff" stroke-width="2"/>')
        a(f'<rect x="{sx + 48}" y="{y + 6}" width="{152 * pct // 100}" height="14" fill="#8fe3ff"/>')
        a(f'<text x="{sx + sw}" y="{y + 18}" font-size="15" class="scr" text-anchor="end">{pct}%</text>')
    a(f'<text x="{sx}" y="{sy + 122}" font-size="14" class="scr">Turn knob: speed</text>')
    # Knob
    kx, ky = bx + 375, by + 110
    a(f'<circle cx="{kx}" cy="{ky}" r="38" fill="#111" />')
    a(f'<circle cx="{kx}" cy="{ky}" r="32" fill="url(#knob)" stroke="#444" stroke-width="2"/>')
    a(f'<line x1="{kx}" y1="{ky - 28}" x2="{kx}" y2="{ky - 12}" stroke="#333" stroke-width="4" stroke-linecap="round"/>')
    a(f'<text x="{kx}" y="{ky + 62}" text-anchor="middle" fill="#cfd6dd" font-size="13">turn = speed</text>')
    a(f'<text x="{kx}" y="{ky + 78}" text-anchor="middle" fill="#cfd6dd" font-size="13">press = on/off</text>')
    # Small keys
    for i, lab in enumerate(("CON", "BAK")):
        x = bx + 60 + i * 90
        a(f'<rect x="{x}" y="{by + 230}" width="56" height="30" rx="6" fill="#2c3036" stroke="#666"/>')
        a(f'<text x="{x + 28}" y="{by + 250}" text-anchor="middle" fill="#cfd6dd" font-size="13">{lab}</text>')
    a(f'<text x="{bx + 220}" y="{by + 305}" text-anchor="middle" fill="#9aa4ad" font-size="13">'
      'Nano + interface board inside; USB reachable for updates</text>')
    a(f'<circle cx="{bx + 10}" cy="{by + 250}" r="9" fill="#555" stroke="#222"/>')
    a('<text x="1150" y="540" class="lbl" text-anchor="middle">controller (Arduino Nano + 1.3" OLED/EC11 module)</text>')
    a('<text x="1150" y="560" class="lbl" text-anchor="middle">powered from the fan\'s +10 V</text>')
    a("</svg>")

    here = os.path.dirname(os.path.abspath(__file__))
    path = os.path.join(here, "overview.svg")
    with open(path, "w") as f:
        f.write("\n".join(o) + "\n")
    print(f"wrote {path}")
    svg_to_png(path, os.path.splitext(path)[0] + ".png", W)


if __name__ == "__main__":
    main()
