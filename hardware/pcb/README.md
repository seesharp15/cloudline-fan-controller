# Single-sided PCB

A printed circuit board for the controller, as an alternative to the
perfboard build. It has **one copper layer** (on the bottom), and **no wire
links**. All parts are through-hole and sit on the top. It carries every part
in [the schematic](../schematic.svg), plus a 4-way screw terminal for the fan
cable and two pin headers for the OLED module. The board is 100 × 75 mm with
four M3 mounting holes.

| Top (parts side) | Bottom copper (as seen from below) |
|---|---|
| ![Top](fab/render_top.png) | ![Bottom](fab/render_bottom_mirrored.png) |

> **Status: designed and checked, not yet manufactured or assembled.** The
> files pass KiCad's design-rule check, an independent copper-connectivity
> check and an independent review (below), but no board has been made from
> them. Build the first one expecting to find something. The fan-side
> unknowns in the main README (pinout, tach) apply here unchanged: do the
> [pre-connection verification](../../README.md#pre-connection-verification)
> first.

## Files

| File | What it is |
|---|---|
| [`fab/cloudline_controller_RevB_gerbers_drill.zip`](fab/cloudline_controller_RevB_gerbers_drill.zip) | **Upload this to the board house.** Gerbers (RS-274X), Excellon drill (mm), and `FAB_NOTES.txt` stating the layer setup. |
| `fab/gerbers/` | The same files unzipped, plus a drill map (PDF) and KiCad's job file (left out of the zip on purpose, see below) |
| [`fab/pcb_bom.csv`](fab/pcb_bom.csv) | Parts by reference, value and footprint, generated from the board. Part numbers and prices: [`../bom.csv`](../bom.csv). |
| [`fab/assembly_top.pdf`](fab/assembly_top.pdf) | Assembly drawing, 1:1, black and white: every part outline, reference and pad |
| [`fab/bottom_copper_toner_transfer_1to1.pdf`](fab/bottom_copper_toner_transfer_1to1.pdf) | Bottom copper at 1:1, drawn as seen from the top (correct for toner transfer: the print goes face-down on the copper), for etching a board at home |
| `fab/render_*.png`, `fab/gerbv_*.png` | Renders. The `gerbv_*` ones are drawn from the Gerber and drill files themselves by gerbv, an independent viewer: this is what the board house will see. |
| [`fab/drc_report.txt`](fab/drc_report.txt) | KiCad design-rule check report |
| `cloudline_controller.kicad_pcb`, `.kicad_pro`, `fp-lib-table`, `cloudline.pretty/` | The KiCad 7 project, to open and edit in KiCad |
| [`make_pcb.py`](make_pcb.py) | Generates the board: placement, netlist, single-layer router |
| [`make_fab.py`](make_fab.py) | Runs the checks and produces everything in `fab/` |
| [`drc.py`](drc.py), [`check_pcb.py`](check_pcb.py) | The checks (also run by `make_fab.py`) |

## Ordering

Upload the zip and choose these options (names vary between board houses):

| Option | Value |
|---|---|
| Layers | **1** |
| Dimensions | 100 × 75 mm |
| Material / thickness | FR-4, 1.6 mm |
| Copper weight | 1 oz (35 µm) |
| Min. track / spacing | 0.5 mm / 0.35 mm (power tracks 0.8 mm), well within any fab's limits |
| Holes | 0.75 – 1.3 mm, plus four 3.2 mm mounting holes; **all non-plated** (single-sided) |
| Copper side | **Bottom** (`B_Cu.gbl`) |
| Solder mask | **Bottom only** (`B_Mask.gbs`). The top has no copper, so it needs no mask; if the fab insists on one, any colour, no openings. |
| Silkscreen | **Top only** (`F_Silkscreen.gto`), on the side **without** copper |
| Surface finish | HASL (lead-free) is fine |

> **Tell the board house which side is which.** Budget fabs often build
> 1-layer boards with copper, mask and legend all on one side. Paste this
> into the order remark (it is also `FAB_NOTES.txt` in the zip):
> *"Single-sided: copper and solder mask on the BOTTOM (B_Cu.gbl, B_Mask.gbs,
> viewed from top, do not mirror); silkscreen on the TOP (F_Silkscreen.gto);
> all holes non-plated."* If their online viewer shows the legend on the
> copper side, ask before paying.

The zip contains exactly `B_Cu.gbl`, `B_Mask.gbs`, `F_Silkscreen.gto`,
`Edge_Cuts.gm1`, the `.drl` file and `FAB_NOTES.txt`. The Gerber and drill
attributes are set to say 1 copper layer and non-plated holes. KiCad's
`.gbrjob` job file is left out, because it would describe two copper layers:
KiCad keeps a top layer (F.Cu) in every design. Here it is marked as a
*jumper* layer and holds nothing.

## Enclosure

The board mounts on four M3 standoffs (about 6 mm) through its own holes;
drill matching holes in the floor of the box. Choose a box whose **inside**
is at least **115 × 90 × 35 mm**: 100 × 75 mm of board, plus room for the
box's corner screw posts, plus the height of the socketed Nano (about 20 mm
above the board). Box makers publish drawings with inside sizes and post
positions; this design hasn't been fitted to a particular box. Leave an
opening at the board's left edge for the Nano's USB plug, and a cable gland
near J2.

## Assembly

Same order as the main README's build steps, with the same checks. Solder
on the bottom (copper) side.

The silkscreen shows each reference, the diode bands (K), the electrolytic's
+ and shaded − side, and the flat side of each TO-92 part. It also labels
the pin functions of J2, J3 and J4.

**TO-92 parts (Q1, U2):** the footprint has 2.54 mm between leads. If your
parts have straight 1.27 mm leads, bend the two outer leads out by about
1.3 mm (standard practice).

| README step | Fit on the PCB | Then check (README) |
|---|---|---|
| 2. Power conditioning | **J2** screw terminal (wire entries facing the board edge), **F1**, **D1** (band = K), **D2** (band = K), **C1** (+ at the top, shaded side −), **C2**, the two **1×15 female headers** for the Nano | Step 2 checks; every pad is on the assembly drawing |
| 3. Power test, no Nano | — | Step 3: VIN = supply − 0.05…0.3 V on the Nano socket's VIN hole (bottom row, right end, next to C2) |
| 4. Regulator, level shifter, OLED headers | **U2** (flat side as on the silkscreen), **C3**, **C4**, **R1**, **R2** (standing up, body over the round silk mark), **U4** on its male headers with the **HV row towards the Nano** (labels on the silkscreen; its GND pins stay unconnected, which is correct), **J3**, **J4** | Step 4 checks |
| 5. Flash, USB only | Nano into the socket, **USB end at the board edge**: the Nano's VIN pin goes into the hole at the right end of the bottom row (next to C2) | Step 5 checks |
| 6. Fan interface | **Q1** (flat side as on the silkscreen), **R3**, **R4**, **R6**, **R7**, **D3**, **D4** (bands as on the silkscreen), **C5**; **R5** only if check M3 said so | Step 6 checks |
| 7. First power-up | Fan cable (J1) into **J2** by function, see below | Step 7 |

**Fan cable into J2.** J2's terminals are labelled on the board; their
order is not the same as the Molex pin numbers:

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

**OLED module:** nine female-female jumper wires, matched by label. J3's
`BAK CON PSH TRB TRA` go to the module's pins of the same name. J4's
`SDA SCL VCC GND` likewise.

## Design notes

- **One copper layer, no wire links.** The router in `make_pcb.py` works
  on a 0.25 mm grid in eight directions, one net at a time: short, straight
  nets first, the power nets last. It may bridge an earlier track with a
  wire link if it has to; with this placement it needs none. The placement
  is what makes that possible:
  - The OLED key header J3 sits directly above the Nano pins it connects to.
  - The I²C lines run straight from the Nano to the level shifter and on
    to J4.
  - The fan terminal's pin order (PWM, GND, TACH, +10V from the top) matches
    where each signal goes on the board.
- **Pull-ups standing up.** R1 and R2 stand vertically under the level
  shifter, each body over its +3V3 lead, so neither crosses another signal.
- **Ground.** Only the Nano's GND pin 29 is wired; the Nano joins its two
  GND pins on its own board. **U4's GND pins are not wired at all.** SparkFun's
  board file shows that on the BOB-12009 they connect only to each other;
  the module's MOSFETs and resistors use only HV and LV. (The schematic
  still shows U4's GND, which is harmless on a perfboard.)
- **R5** (not fitted by default) pulls the PWM line up to the fan's +10 V
  *before* F1 (J2 pin 1). The schematic and PINOUT show the same.
- **Pads** are at least the hole + 0.9 mm and never under 1.7 mm, i.e. at
  least 0.45 mm of copper around each hole (the hole is unplated, so the
  copper ring is what holds the lead). On the 2.54 mm-pitch rows (Nano
  socket, headers) that gives 1.9 mm pads with 0.64 mm between them, so no
  track runs between adjacent pins there. **J2's pads are 3.3 mm** so they
  survive the screw torque; still route the cable through a gland so it
  can't pull on the terminal.
- **Tracks:** 0.8 mm for GND, +5V and the fused and unfused fan supply;
  0.5 mm for signals; 0.35 mm clearance; 0.8 mm from the board edge.
- **Silkscreen:** text at least 1.0 mm high with a 0.16 mm stroke (the title
  is larger).
- **Footprints** come from KiCad 7's standard library, except two in
  `cloudline.pretty`:
  - `SparkFun_BOB-12009`: pad positions read from SparkFun's own Eagle
    board file (two rows of six, 2.54 mm pitch, rows 10.16 mm apart, board
    15.24 × 12.7 mm, HV row and LV row as labelled);
  - `PTC_Radial_P5.10mm`: two pads 5.1 mm apart in a straight line, for
    the Bourns MF-R020.

  The ceramic capacitors need **5 mm lead pitch**; `hardware/bom.csv` lists
  TDK FG28-series parts with that spacing.

## Verification

All of this is re-run by `python3 hardware/pcb/make_fab.py`.

| Check | Result |
|---|---|
| KiCad DRC (`drc.py`): clearance, track width, hole sizes, courtyards, silkscreen, board edge, unconnected items | **0 errors, 0 unconnected.** 22 warnings, all `lib_footprint_mismatch`: the library footprints differ from their library copies because the pads were deliberately enlarged and moved to the bottom layer only. |
| Copper connectivity vs. PINOUT.md (`check_pcb.py`): connections computed from the copper shapes themselves, not from KiCad's net names | **20 nets, 71 pads, 0 opens, 0 shorts.** Mutation-tested: deleting one track reports an open; a track bridging +5V and GND reports a short. |
| Gerber/drill files read back by gerbv | The renders in `fab/gerbv_*.png` match the design |
| Toner-transfer PDF scale | The board outline measures 99.8 × 75.2 mm on the PDF rendered at 100 dpi, i.e. 1:1 |
| Independent review (a separate agent, 2026-11-03) | No blockers. Findings fixed: enclosure fit, fab notes, removed wire link, F1 footprint, TO-92 lead note, silkscreen size, J4 labels, J2 pads, documentation errors (VERIFICATION.md §1.7) |

The Nano pin assignment on the PCB comes from the same table as everything
else (`config.h`, PINOUT.md). The PCB itself is checked by `check_pcb.py`
against PINOUT.md's point-to-point list. `test/check_pins.py` covers the
schematic, the wiring view, PINOUT and README, not the PCB.

**Not verified:**
- Nothing has been manufactured or assembled.
- Mechanical fit was not checked against real parts. Before ordering in
  quantity, check:
  - the BOB-12009 against its footprint;
  - the lead pitch of each capacitor;
  - the TO-92 lead bend;
  - the wire-entry direction of the screw terminal (from the footprint
    drawing; there is no 3D model);
  - the clearance of the Nano's USB plug at the board edge.
- KiCad 7's command-line tool has no 3D/STEP export without the 3D model
  package, so there is no 3D view.

## Regenerating

```sh
sudo apt install kicad kicad-footprints gerbv librsvg2-bin poppler-utils   # KiCad 7
python3 hardware/pcb/make_pcb.py    # placement + routing -> cloudline_controller.kicad_pcb
python3 hardware/pcb/make_fab.py    # checks + fab/ package
```

Routing is deterministic: the same script produces the same tracks. To
change the placement, edit `PARTS` in `make_pcb.py`; to change the routing
order, `ROUTE_ORDER`.
