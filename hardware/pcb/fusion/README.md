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

The full walkthrough is in [`../README.md`](../README.md#the-walkthrough-session-by-session).
It goes from opening these files to a finished board in separate, resumable
sessions:

- S3 blank;
- S4–S6 in Fusion: import, setup and operations, simulate and post;
- S7–S10 on the machine: prepare and zero, isolation, drilling, outline;
- S11 inspection and test.

## Feeds and speeds

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

See sessions S11 (clean, inspect, test for shorts) and S12–S17 (assembly)
in [`../README.md`](../README.md#s11-clean-inspect-and-test-the-bare-board).

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
