#!/usr/bin/env python3
"""Cross-check every pin assignment across the firmware, the schematic, the
wiring view, PINOUT.md and README.md. Exits non-zero on any mismatch.

    python3 test/check_pins.py        (from the repository root)
"""

import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def read(rel):
    with open(os.path.join(ROOT, rel), encoding="utf-8") as f:
        return f.read()


# Arduino Nano physical pin numbers (header pin 1 = D1/TX).
NANO_PHYS = {"D1": 1, "D0": 2, "GND_A": 4, "D2": 5, "D3": 6, "D4": 7, "D5": 8, "D6": 9,
             "D7": 10, "D8": 11, "D9": 12, "D10": 13, "D11": 14, "D12": 15, "D13": 16,
             "3V3": 17, "A4": 23, "A5": 24, "5V": 27, "GND_B": 29, "VIN": 30}

# signal: (config.h macro or None, Nano pin, module label or None, wiring.py net)
SIGNALS = [
    ("TACH", "PIN_TACH", "D2", None, "TACH_D2"),
    ("ENC_A", "PIN_ENC_A", "D3", "TRA", "TRA"),
    ("ENC_B", "PIN_ENC_B", "D4", "TRB", "TRB"),
    ("ENC_SW", "PIN_ENC_SW", "D5", "PSH", "PSH"),
    ("KEY_CON", "PIN_KEY_CON", "D6", "CON", "CON"),
    ("KEY_BAK", "PIN_KEY_BAK", "D7", "BAK", "BAK"),
    ("FAN_PWM", "PIN_FAN_PWM", "D9", None, "GATE"),
    ("STATUS_LED", "PIN_STATUS_LED", "D13", None, None),
    ("SDA", None, "A4", "SDA", "SDA"),
    ("SCL", None, "A5", "SCL", "SCL"),
    ("VIN", None, "VIN", None, "VIN"),
    ("+5V", None, "5V", None, "+5V"),
]

rows = []
failures = 0


def check(name, source, ok, detail=""):
    global failures
    rows.append((name, source, "ok" if ok else "MISMATCH", detail))
    if not ok:
        failures += 1


cfg = read("firmware/cloudline_controller/config.h")
sch = read("hardware/schematic.py")
wir = read("hardware/wiring.py")
pin = read("PINOUT.md")
rd = read("README.md")

# Schematic: Nano pins drawn with pin_l/pin_r(x, y, "LABEL", physical), the
# variable name they are assigned to, and the module pins they are wired to.
sch_nano = {}
for var, label, num in re.findall(r'(n_\w+) = pin_[lr]\([^,]+, \d+, "([^"]+)", (\d+)\)', sch):
    sch_nano[var] = (label, int(num))
sch_mod = dict(re.findall(r'(m_\w+) = pin_l\([^,]+, \d+, "([^"]+)"', sch))
sch_wires = re.findall(r'wire\((n_\w+), (m_\w+)\)', sch)

# Wiring view: wire(net, [(px[("T"|"B", "PIN")] ...  and OLED pins by net name.
wir_pins = {}
# (a) loops over ("NET", "PIN") pairs; (b) wire("NET", [... px[("T", "PIN")] ...])
for net, p in re.findall(r'\("([A-Z+_0-9]+)", "([AD]\d+|5V|GND|VIN)"\)', wir):
    wir_pins.setdefault(net, set()).add(p)
for net, p in re.findall(r'wire\("([^"]+)", \[[^;]*?px\[\("[TB]", "([^"]+)"\)\]', wir, re.S):
    wir_pins.setdefault(net, set()).add(p)

for sig, macro, npin, mod, wnet in SIGNALS:
    num = int(npin[1:]) if npin.startswith("D") else None
    phys = NANO_PHYS.get(npin)
    # 1. firmware
    if macro:
        m = re.search(r"#define\s+%s\s+(\d+)" % macro, cfg)
        check(sig, "config.h", bool(m) and int(m.group(1)) == num,
              f"{macro} = {m.group(1) if m else 'missing'} (expected {num})")
    # 2. schematic: a Nano pin whose label starts with the pin name and whose
    #    physical number matches; and, for module signals, a wire to that label.
    hits = [(v, l, n) for v, (l, n) in sch_nano.items()
            if l.split(" / ")[0].replace("+", "") in (npin, npin.replace("D", "D"))
            or l.startswith(npin + " ")]
    if npin == "5V":
        hits = [(v, l, n) for v, (l, n) in sch_nano.items() if l == "+5V"]
    if sig != "STATUS_LED":
        ok = any(n == phys for _, _, n in hits)
        check(sig, "schematic", ok, f"{[(l, n) for _, l, n in hits]} (expected {npin} = pin {phys})")
        if mod and sig in ("SDA", "SCL"):
            # through the level shifter U4: Nano pin -> U4 HVn, U4 LVn -> module pin
            y = {"SDA": 460, "SCL": 500}[sig]
            nv = [v for v, _, _ in hits][0] if hits else "?"
            ok = (f"wire({nv}, (ux4, {y}))" in sch and
                  f"wire((ux4 + uw4, {y}), m_{sig.lower()})" in sch and
                  sch_mod.get(f"m_{sig.lower()}", "").startswith(mod))
            check(sig, "schematic wire", ok, f"{npin} -> U4 -> {mod}")
        elif mod:
            nvars = {v for v, _, _ in hits}
            wired = [sch_mod.get(m, "?") for nv, m in sch_wires if nv in nvars]
            check(sig, "schematic wire", any(w.startswith(mod) for w in wired),
                  f"{npin} wired to {wired} (expected {mod})")
    # 3. wiring view
    if wnet:
        label = {"5V": "5V"}.get(npin, npin)
        check(sig, "wiring.svg", label in wir_pins.get(wnet, set()),
              f"net {wnet} -> {sorted(wir_pins.get(wnet, set()))} (expected {label})")
    # 4. PINOUT.md section 3 table row: "| phys | **NAME** ..."
    if phys:
        pname = {"5V": r"\+5V", "VIN": "VIN"}.get(npin, npin)
        row = re.search(r"^\| %d \| \*\*%s\*\*[^\n]*" % (phys, pname), pin, re.M)
        if sig == "STATUS_LED":
            row = re.search(r"^\| %d \| %s [^\n]*" % (phys, npin), pin, re.M)
        ok = bool(row)
        if ok and macro:
            ok = f"`{macro} {num}`" in row.group(0)
        if ok and mod:
            ok = f"**{mod}**" in row.group(0)
        check(sig, "PINOUT.md", ok, row.group(0)[:70] if row else f"no row for pin {phys} {npin}")
    # 5. README build step 4 table (module wiring) and step 6 text
    if mod:
        if sig in ("SDA", "SCL"):  # via the level shifter U4
            r = re.search(r"^ *\| %s \| U4 LV\d [^|]*socket %s \(pin %d\)" % (mod, npin, phys), rd, re.M)
        else:
            r = re.search(r"^ *\| %s \| socket %s \(pin %d\)" % (mod, npin, phys), rd, re.M)
        check(sig, "README.md", bool(r), r.group(0) if r else f"no '| {mod} | socket {npin} (pin {phys})' row")
    elif sig in ("FAN_PWM", "TACH"):
        r = re.search(r"socket \*\*%s\*\* \(pin %d\)" % (npin, phys), rd)
        check(sig, "README.md", bool(r), r.group(0) if r else f"no 'socket **{npin}** (pin {phys})'")
    elif sig in ("VIN", "+5V"):
        word = {"VIN": r"\*\*VIN\*\* pin \(30\)", "+5V": r"\*\*\+5V\*\* pin \(27\)"}[sig]
        r = re.search(word, rd)
        check(sig, "README.md", bool(r), r.group(0) if r else f"no {word}")

# J1 pin functions: schematic, wiring view, PINOUT.md
j1 = {4: "+10V", 3: "GND", 2: "PWM", 1: "TACH"}
for n, f in j1.items():
    check(f"J1.{n}", "schematic", re.search(r'pin_r\([^)]*"%s", %d,' % (re.escape(f), n), sch) is not None, f)
    check(f"J1.{n}", "wiring.svg", re.search(r'%d: \("%s", \d+\)' % (n, re.escape(f)), wir) is not None, f)
    check(f"J1.{n}", "PINOUT.md", re.search(r"^\| \*\*%d\*\* \|" % n, pin, re.M) is not None, f)

w = max(len(r[0]) for r in rows)
for name, src, status, detail in rows:
    print(f"{name:<{w}}  {src:<15} {status:<8} {detail}")
print(f"\npin cross-check: {len(rows)} checks, {failures} mismatches")
sys.exit(1 if failures else 0)
