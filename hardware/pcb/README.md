# Single-sided PCB

A printed circuit board for the controller, as an alternative to the
perfboard build. It has **one copper layer** (on the bottom); all parts are
through-hole and sit on the top. It carries every part in
[the schematic](../schematic.svg), plus a 4-way screw terminal for the fan
cable and two pin headers for the OLED module.

| Top (parts side) | Bottom copper (as seen from below) |
|---|---|
| ![Top](fab/render_top.png) | ![Bottom](fab/render_bottom_mirrored.png) |

> **Status: designed and checked, not yet manufactured or assembled.** The
> Gerbers pass KiCad's design-rule check and an independent copper
> connectivity check (below), but no board has been made from them yet.
> Build the first one expecting to find something. The fan-side unknowns in
> the main README (pinout, tach) apply here unchanged: do the
> [pre-connection verification](../../README.md#pre-connection-verification)
> first.

## Files

| File | What it is |
|---|---|
| [`fab/cloudline_controller_RevB_gerbers_drill.zip`](fab/cloudline_controller_RevB_gerbers_drill.zip) | **Upload this to the board house.** Gerbers (RS-274X) + Excellon drill (mm). |
| `fab/gerbers/` | The same files unzipped, plus a drill map (PDF) and KiCad's job file (left out of the zip on purpose, see below) |
| [`fab/pcb_bom.csv`](fab/pcb_bom.csv) | Parts by reference, value and footprint (generated from the board). Part numbers and prices: [`../bom.csv`](../bom.csv). |
| [`fab/assembly_top.pdf`](fab/assembly_top.pdf) | Assembly drawing, 1:1, black and white: where each part goes |
| [`fab/bottom_copper_toner_transfer_1to1.pdf`](fab/bottom_copper_toner_transfer_1to1.pdf) | Bottom copper at 1:1 (as seen from the top), for etching a board at home by toner transfer |
| `fab/render_*.png`, `fab/gerbv_*.png` | Renders. The `gerbv_*` ones are drawn from the Gerber and drill files themselves by gerbv, an independent viewer: this is what the fab will see. |
| [`fab/drc_report.txt`](fab/drc_report.txt) | KiCad design-rule check report |
| `cloudline_controller.kicad_pcb`, `.kicad_pro`, `fp-lib-table`, `cloudline.pretty/` | The KiCad 7 project, to open and edit in KiCad |
| [`make_pcb.py`](make_pcb.py) | Generates the board: placement, netlist, single-layer router |
| [`make_fab.py`](make_fab.py) | Runs the checks and produces everything in `fab/` |
| [`drc.py`](drc.py), [`check_pcb.py`](check_pcb.py) | The checks (also run by `make_fab.py`) |

## Ordering

Upload the zip and choose these options (names vary slightly between board
houses):

| Option | Value |
|---|---|
| Layers | **1** |
| Dimensions | 100 × 75 mm |
| Material / thickness | FR-4, 1.6 mm |
| Copper weight | 1 oz (35 µm) |
| Min. track / spacing | 0.5 mm / 0.35 mm (power tracks 0.8 mm), well within any fab's limits |
| Min. hole | 0.71 mm; largest 3.2 mm (mounting holes) |
| Plated holes | No: holes on single-sided boards are not plated. The drill file labels them "plated" (KiCad's default for through-hole pads); a 1-layer order ignores that. |
| Solder mask | **Bottom (copper side) only**. The top has no copper, so it needs no mask; if the fab insists on one, any colour, no openings. |
| Silkscreen | **Top** only (`F_Silkscreen.gto`) |
| Surface finish | HASL (lead-free) is fine |

The zip contains exactly: `B_Cu.gbl` (copper), `B_Mask.gbs` (mask),
`F_Silkscreen.gto` (legend), `Edge_Cuts.gm1` (outline) and the `.drl` drill
file. KiCad's `.gbrjob` job file is left out because it would describe two
copper layers: KiCad keeps a top copper layer (F.Cu) in the design, used
here as a **jumper layer** to represent the wire link W1. F.Cu is never
fabricated.

## Assembly

Same order as the main README's build steps, with the same checks. Solder
on the bottom (copper) side. The silkscreen shows each reference, the
diode bands (K), the electrolytic's + and its shaded − side, the flat side
of each TO-92 part, and the pin functions of J2, J3 and J4.

| README step | Fit on the PCB | Then check (README) |
|---|---|---|
| 2. Power conditioning | **W1** (bare wire, top side, lowest part first), **J2** screw terminal (wire entries facing the board edge), **F1**, **D1** (band = K), **D2** (band = K), **C1** (+ at the top, shaded side −), **C2**, the two **1×15 female headers** for the Nano | Step 2 checks; pad numbers on the assembly drawing |
| 3. Power test, no Nano | — | Step 3: VIN = supply − 0.05…0.3 V on the Nano socket's VIN hole (bottom row, right end) |
| 4. Regulator, level shifter, OLED headers | **U2** (flat side as on the silkscreen), **C3**, **C4**, **R1**, **R2** (standing up, body over the round silk mark), **U4** on its male headers with the **HV row towards the Nano** (labels on the silkscreen), **J3**, **J4** | Step 4 checks |
| 5. Flash, USB only | Nano into the socket, **USB end at the board edge**: the Nano's VIN pin goes into the hole at the right end of the bottom row (next to C2) | Step 5 checks |
| 6. Fan interface | **Q1** (flat side as on the silkscreen), **R3**, **R4**, **R6**, **R7**, **D3**, **D4** (bands as on the silkscreen), **C5**; **R5** only if check M3 said so | Step 6 checks |
| 7. First power-up | Fan cable (J1) into **J2** by function, see below | Step 7 |

**Fan cable into J2.** J2's pins are labelled on the board; their order is
not the same as the Molex pin numbers:

| J2 terminal (silkscreen) | J2 pin | Fan Molex pin (J1) |
|---|---|---|
| +10V | 1 (bottom) | 4 |
| TACH | 2 | 1 |
| GND | 3 | 3 |
| PWM | 4 (top) | 2 |

**If the pre-connection checks asked for a change:**
- **M3 (no internal pull-up):** fit R5; it has its place on the board.
- **M4 opposite polarity:** R4's +5V end is a fixed track on the PCB. Cut
  the track at R4's lower pad (pad 2, the end away from the D9 track) and
  run a short wire from that pad to GND. Also set `FAN_PWM_INVERTED 0`.
- **M4b RC filter:** there is no place for it on the PCB. Fit the series
  resistor in the PWM wire of the fan cable and the capacitor across J2's
  PWM and GND terminals.
- **M5 (more than 12 V on the tach pin):** leave the fan's TACH wire out of J2.

**OLED module.** Ten female-female jumper wires, by label: J3 `BAK CON PSH
TRB TRA` to the module's pins of the same name; J4 `1 SDA 2 SCL 3 VCC
4 GND` likewise.

## Design notes

- **One copper layer, one wire link.** The router (in `make_pcb.py`) runs
  on a 0.25 mm grid, eight directions, one net at a time: short straight
  nets first, the power nets last. Where it can't get past an earlier track
  it may place a wire link; this board needs one, **W1** on GND (7.5 mm).
  The placement uses parts as bridges where a crossing is unavoidable: R4
  stands across the tach and GND tracks, and the encoder/key header sits
  directly above the Nano pins it connects to.
- **Pull-ups standing up.** R1 and R2 stand vertically between neighbouring
  columns of the level shifter, so neither crosses another signal.
- **One ground pin per module.** Only the Nano's GND pin 29 and U4's GND pin
  on the LV row are wired; each module joins its own GND pins on its board.
- **R5** (not fitted by default) pulls the PWM line up to the fan's +10 V
  *before* F1 (J2 pin 1). The schematic and PINOUT show the same.
- **Pads** are at least the hole + 0.9 mm and never under 1.7 mm, i.e. at
  least 0.45 mm of copper around each hole (the hole is unplated, so the
  copper ring is what holds the lead). On the 2.54 mm-pitch rows (Nano
  socket, headers) that gives 1.9 mm pads with 0.64 mm between them, so no
  track runs between adjacent pins there.
- **Tracks:** 0.8 mm for GND, +5V, +10V and the fused/unfused fan supply;
  0.5 mm for signals; 0.35 mm clearance; 0.8 mm from the board edge.
- **Footprints** come from KiCad 7's standard library, except two in
  `cloudline.pretty`:
  - `SparkFun_BOB-12009`: pad positions read from SparkFun's own Eagle
    board file (two rows of six, 2.54 mm pitch, rows 10.16 mm apart, board
    15.24 × 12.7 mm);
  - `WireLink_P7.50mm`.

  F1 uses the Bel Fuse 0ZRE footprint (5.1 mm lead pitch, which also fits
  the Bourns MF-R020). The ceramic capacitors need **5 mm lead pitch**;
  `hardware/bom.csv` lists TDK FG28-series parts with that spacing.

## Verification

All of this is re-run by `python3 hardware/pcb/make_fab.py`.

| Check | Result |
|---|---|
| KiCad DRC (`drc.py`): clearance, track width, hole sizes, courtyards, silkscreen, board edge, unconnected items | **0 errors, 0 unconnected.** 23 warnings, all `lib_footprint_mismatch`: every library footprint differs from its library copy because the pads were deliberately enlarged and moved to the bottom layer only. |
| Copper connectivity vs. PINOUT.md (`check_pcb.py`): connections computed from the copper shapes themselves, not from KiCad's net names | **20 nets, 72 pads, 0 opens, 0 shorts.** Mutation-tested: deleting one track reports an open; a track bridging +5V and GND reports a short. |
| Gerber/drill files read back by gerbv | Renders in `fab/gerbv_*.png` match the design |
| Toner-transfer PDF scale | Board outline measures 99.8 × 75.2 mm on the PDF rendered at 100 dpi (1:1) |
| Pin numbers (`test/check_pins.py`) | The PCB uses the same Nano pins as config.h, PINOUT and the schematic |

**Not verified:**
- Nothing has been manufactured or assembled.
- Mechanical fit was not checked against real parts. In particular, check
  the BOB-12009 against its footprint, the lead pitch of each capacitor,
  and the clearance of the Nano's USB plug at the board edge, before
  ordering in quantity.
- KiCad 7's command-line tool has no 3D/STEP export without the 3D model
  package, so there is no 3D view.

## Regenerating

```sh
sudo apt install kicad kicad-footprints gerbv librsvg2-bin   # KiCad 7
python3 hardware/pcb/make_pcb.py    # placement + routing -> cloudline_controller.kicad_pcb
python3 hardware/pcb/make_fab.py    # checks + fab/ package
```

Routing is deterministic: the same script gives the same board. To change
the placement, edit `PARTS` in `make_pcb.py`; to change the routing order,
`ROUTE_ORDER`.
