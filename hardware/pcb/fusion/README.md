# Milling the PCB with Fusion 360 on a Stepcraft M.1000

These files let you mill the single-sided board from a copper-clad blank. They
are made from the same KiCad design as the Gerbers in [`../fab/`](../fab/).
To edit the circuit or board in **Fusion Electronics**, see
[`../eagle/`](../eagle/README.md).

**The setup these files are made for:**

| | |
|---|---|
| Machine | Stepcraft M.1000 with tool changer |
| Control software | UCCNC |
| Spindle | Stepcraft DI 1000, 8 000–20 000 rpm |
| CAM | Fusion 360, **commercial licence**: one program with every tool change, real rapids |
| Blank | FR-4. Thickness and copper weight are unknown, so **measure the thickness** (step 1 below). The cut depth works for both 35 and 70 µm copper. |
| Workholding | Double-sided tape, Z touch plate, **no height map** |

![Copper side up, as it sits on the machine](mill_preview.png)

## Files

| File | Use |
|---|---|
| [`cloudline_pcb_MILL_copper_up.step`](cloudline_pcb_MILL_copper_up.step) | **The model to machine.** It is the board flipped copper side up, the way it lies on the machine. It holds the FR-4 body (`FR4_substrate`, with all 99 holes) and one 0.035 mm copper body per net (`copper_<net>`). The origin is the board's lower-left corner; Z = 0 is the underside and the copper top is at Z = 1.635 mm. |
| [`cloudline_pcb_MILL_copper_up.dxf`](cloudline_pcb_MILL_copper_up.dxf) | The same view in 2D, in mm, with the same origin. Layers: `OUTLINE`, `COPPER` (pad and track outlines), `ISO_PASS1` / `ISO_PASS2` (ready-made isolation tool-centre paths), `DRILL_0p75mm` … `DRILL_3p20mm` (one layer per hole size), `FIDUCIAL` (corner marks). |
| [`cloudline_pcb_tools.json`](cloudline_pcb_tools.json) | Fusion tool library: T1 V-bit, T2–T6 drills, T7 outline end mill, with feeds and speeds for the DI 1000. |
| [`cloudline_pcb_TOPVIEW.step`](cloudline_pcb_TOPVIEW.step) | The board as seen from the component side, for checks or an enclosure model. **Do not machine it.** |
| [`milling_report.txt`](milling_report.txt) | Gaps, hole table, cut widths, and the result of the isolation simulation. |
| [`isolation_preview.png`](isolation_preview.png) | The isolation paths over the copper. |
| `make_fusion.py`, `make_tool_library.py`, `check_fusion.py` | Generate and check everything above (see [Regenerating](#regenerating)). |

**Why the files are mirrored.** KiCad draws the bottom copper as seen through
the board from the top. On the machine you look at the copper directly, so the
`MILL` files are rotated 180° about the board's long axis (x′ = 100 − x,
y′ = 75 − y). The Nano's USB end is on the **right** in the mill files.

## Numbers that matter

- Board: 100 × 75 mm, copper on one side only. It was re-routed for milling:
  - Tracks keep ≥ 0.69 mm from other copper.
  - **The smallest gap anywhere is 0.64 mm.** That is between neighbouring
    2.54 mm-pitch header pads, so no layout can do better.
  - Copper stays ≥ 3.15 mm from the board edge.
- Isolation (T1, 30° V-bit with a 0.1 mm tip):
  - Nominal depth **0.20 mm**, which cuts 0.207 mm wide. There are two passes,
    0.104 and 0.228 mm from the copper, which clears about 0.33 mm around
    every pad and track.
- **Without a height map**, the paths are checked by simulation for a board
  up to **±0.10 mm** off the touch-off height:
  - Where the board sits 0.1 mm low, the cut is 0.10 mm deep. That is still
    through 70 µm copper, and every net stays isolated.
  - Where it sits 0.1 mm high, the cut is 0.30 mm deep. Copper edges lose
    0.027 mm, which is harmless.
- Holes: 0.75 mm ×4, 0.80 ×32, 1.00 ×51, 1.10 ×2, 1.20 ×2, 1.30 ×4,
  3.20 ×4 (mounting). That is 99 holes, all non-plated.

## Step by step

### 1. Prepare the blank

1. **Measure the blank thickness** with calipers at all four corners and
   note the largest value. FR-4 is usually 1.5–1.6 mm. Copper weight doesn't
   need to be known.
2. Cut the blank to at least **110 × 85 mm**, which leaves 5 mm around the
   board. A 100 × 160 Eurocard blank also works.
3. **Surface the spoilboard** where the blank goes. Use a large flat end mill
   about 0.2 mm deep.
4. Clean the back of the blank. Cover the **whole** back with thin
   double-sided tape and press it down hard, especially in the middle. Tape
   only at the edges lets the middle bow up.
5. **Flatness check** (this replaces the height map):
   - Zero Z with the touch plate at the board centre.
   - Probe the copper height at the four corners.
   - The spread must stay within **±0.10 mm**. If it doesn't, re-tape the blank
     or re-surface the spoilboard first.

### 2. Fusion: import

1. Upload `cloudline_pcb_MILL_copper_up.step` and open it.
2. Import the tool library: in **Manufacture → Tool Library**, right-click
   a local library folder and choose *Import tool libraries…*, then pick
   `cloudline_pcb_tools.json`. Change the **tool numbers** to match the slots
   in your tool changer if they differ from T1–T7.

   The JSON follows Fusion's tool library format. It has not been tested in
   Fusion. If the import fails, create the seven tools by hand from the
   [feeds and speeds table](#feeds-and-speeds).
3. Create a sketch on an **offset plane 1.635 mm** above the XY plane. In it,
   use *Insert → Insert DXF* to place `cloudline_pcb_MILL_copper_up.dxf` at
   origin 0,0 in mm. The `ISO_PASS1` / `ISO_PASS2` curves now lie exactly on
   the copper surface.

### 3. Fusion: setup (Manufacture → Setup)

- **Machine:** your Stepcraft M.1000 machine definition, if you have one.
  Otherwise use a generic 3-axis mill.
- **Stock:** *Fixed size box*.
  - X × Y = your blank size.
  - Z = the **thickness you measured**.
  - Model position: X *offset from left* 5 mm, Y *offset from front* 5 mm,
    Z *offset from top* **0**, so the copper top is the stock top.
- **WCS:** Z up. Origin *Stock box point*, **top, front-left corner** of the
  stock.
- **On the machine:**
  - X/Y zero is the front-left corner of the blank.
  - Z zero is the copper surface. Use the touch plate with **T1** at the
    board centre.

  Your tool changer's tool-length measurement must then refer every other
  tool to that Z zero. Use the Stepcraft/UCCNC tool-changer routine for this.

### 4. Fusion: operations (all in one setup, in this order)

**Op 1: Isolation, T1. 2D → Trace, twice.**

- **Geometry:** chain-select every curve on `ISO_PASS1`. Use a second
  Trace for `ISO_PASS2`.
- **Heights:**
  - *Bottom height:* *Selected contour(s)*, **offset −0.20 mm**.
  - *Top height:* 0.5 mm above. *Retract:* 2 mm.
  - No multiple depths. No compensation: the paths are already tool-centre
    lines.
- **Simulate with stock** and compare the result with
  [`isolation_preview.png`](isolation_preview.png).
- Don't use these paths with another tool. For a different V-bit, see
  [Regenerating](#regenerating).

**Op 2: Drilling, T2–T6. Drilling → Drill, one operation per tool.**

| Tool | Holes | How to select |
|---|---|---|
| T2 0.8 mm | 0.75 mm ×4 and 0.80 mm ×32 | Click one hole of each size, then *Select same diameter* |
| T3 1.0 mm | 1.00 mm ×51 | |
| T4 1.2 mm | 1.10 mm ×2 and 1.20 mm ×2 | |
| T5 1.3 mm | 1.30 mm ×4 (J2 screw terminal) | |
| T6 3.2 mm | 3.20 mm ×4 (M3 mounting) | |

You have every drill size, so you can also put a 0.75 mm and a 1.1 mm drill on
those holes. That gives a better fit, but takes two more tool changes.

- *Cycle:* **Chip breaking – partial retract**, peck 0.5 mm. For T6, use
  *Deep drilling – full retract*.
- *Bottom height:* **Stock bottom**, with *Drill tip through bottom* and a
  break-through depth of 0.5 mm.
- *Hole order:* *Optimize order*.

**Op 3: Outline, T7. 2D → 2D Contour.**

- *Geometry:* the bottom outer edge of `FR4_substrate`. The tool goes
  outside.
- *Bottom height:* **Stock bottom − 0.2 mm**.
- *Multiple depths:* maximum step-down 0.5 mm.
- *Tabs:* 4 tabs, 3 mm wide, 0.6 mm thick. Or no tabs, as long as the tape
  covers the whole board.
- The 0.8 mm end mill also works for the outline, but use step-down 0.2 mm
  and feed 250 mm/min.

### 5. Post and run

- **Post processor:** search the Fusion post library for **UCCNC** and use
  Autodesk's *UCCNC* post. Output units mm.
  - The program changes tools with `T# M6`.
  - UCCNC runs its M6 macro, which drives the Stepcraft tool changer.
  - The first time, check that the M6 macro measures tool lengths the way
    your machine is set up.
- Post **all operations into one program**. The order is T1 → T2…T6 → T7.
- **First run:** do a dry run at Z +5 mm (offset the work Z in UCCNC), then
  run on scrap before the real blank.

### Feeds and speeds

These values are also in the tool library. They are starting values for
carbide tools in FR-4, and every speed is within the DI 1000's
8 000–20 000 rpm.

| Tool | rpm | Feed | Plunge | Step-down |
|---|---|---|---|---|
| T1 V-bit 30° / 0.1 mm | 20 000 | 400 mm/min | 100 mm/min | single pass, 0.20 mm |
| T2 drill 0.8 mm | 20 000 | — | 100 mm/min | peck 0.5 mm |
| T3 drill 1.0 mm | 20 000 | — | 120 mm/min | peck 0.5 mm |
| T4 drill 1.2 mm | 20 000 | — | 140 mm/min | peck 0.5 mm |
| T5 drill 1.3 mm | 20 000 | — | 140 mm/min | peck 0.5 mm |
| T6 drill 3.2 mm | 12 000 | — | 200 mm/min | full retract |
| T7 end mill 2 mm, 2 flutes | 18 000 | 500 mm/min | 100 mm/min | 0.5 mm |

Small carbide drills break if they wobble. Make sure the collet is clean and
the drill sits fully in it.

## After milling

- Scrub the copper with a fine abrasive pad.
- Check the gaps with a loupe against [`isolation_preview.png`](isolation_preview.png).
- With a continuity tester, check that neighbouring header pads are **not**
  connected. These are the pairs listed in `milling_report.txt`.
- A milled board has no solder mask and no silkscreen:
  - Print [`../fab/assembly_top.pdf`](../fab/assembly_top.pdf) at 100 % as the
    placement guide.
  - Solder-through flux lacquer protects the copper.
- Assemble as in [`../README.md`](../README.md). The parts go on the side
  **without** copper.

## Regenerating

```sh
pip install shapely ezdxf build123d matplotlib
python3 hardware/pcb/fusion/make_fusion.py        # STEP, DXF, previews, report
python3 hardware/pcb/fusion/make_tool_library.py  # Fusion tool library
python3 hardware/pcb/fusion/check_fusion.py       # re-reads the STEP files and checks them
```

`make_fusion.py` reads `../cloudline_controller.kicad_pcb` and does not need
KiCad. For a different V-bit, set these at the top of the script and run it
again:

- `VBIT_ANGLE`
- `VBIT_TIP`
- `CUT_DEPTH`
- `Z_TOL`
- `N_PASSES`
- `PASS_OVERLAP`

The script stops with an error in any of these cases:

- the cut can't fit between two nets;
- the simulated cut would join two nets or cut into copper;
- at the shallowest depth (`CUT_DEPTH − Z_TOL`) the cut would no longer
  isolate the nets or get through 70 µm copper.

If you edit the board in Fusion Electronics instead (see `../eagle/`), these
files are **not** updated. Then machine the board you edited, for example
from its 3D PCB.
