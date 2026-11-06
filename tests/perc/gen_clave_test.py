#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
# COQUÍ: GENERATE in the clave (firmware/src/gen.c: NOCHE, CALLE, FIESTA), host mode, its notes out over USB-MIDI (SEQ
# MIDI OUT on). T1 (VA), PLAY at 120 BPM (a sixteenth 125 ms), GENERATE on for T1 (FX held + F#3), its MODE stepped with
# FX + D#4: every note on the sequencer's sixteenths; NOCHE's mostly on the son clave's strokes; FIESTA busier than CALLE,
# CALLE than NOCHE.
#   python3 tests/perc/gen_clave_test.py build/host/coqui
import os
import subprocess
import sys
from pathlib import Path

EXE = sys.argv[1] if len(sys.argv) > 1 else "build/host/coqui"
OUT = Path("build/host/gen_clave")
OUT.mkdir(parents=True, exist_ok=True)
PLAY, END = 2000, 2000 + 32000                  # (32 s: 16 clave cycles)
CLAVE = {4, 8, 16, 22, 28}                      # (2-3 son clave: its strokes, in sixteenths of two bars)
bad = 0


def check(what, ok, detail=""):
    global bad
    print(f"  {'ok' if ok else 'FAIL':4s}  gen clave: {what}" + (f" ({detail})" if detail and not ok else ""))
    bad += not ok


def settings():                                 # (version 3's settings: SEQ MIDI OUT on; as tests/quick_test.py)
    b = bytearray(170)
    b[5], b[167] = 1, 3
    f = OUT / "set.bin"
    f.write_bytes(bytes(b))
    return dict(os.environ, GRIMM_SETTINGS_IN=str(f))


def fx_key(t, bit):                             # FX held, a key tapped, FX let go
    return [(t, "down 2"), (t + 20, f"keys {1 << bit:#x}"), (t + 40, "keys 0"), (t + 60, "up 2")]


def run(name, steps):
    ev = [(PLAY, "btn 12")] + fx_key(PLAY + 100, 1)          # PLAY; T1 generating (F#3)
    for i in range(steps):                                    # MODE: D#4 (key 10), once a step
        ev += fx_key(PLAY + 300 + 200 * i, 10)
    path = OUT / f"{name}.txt"
    path.write_text("".join(f"{t} {e}\n" for t, e in sorted(ev, key=lambda x: x[0])) + f"{END} end\n")
    out = subprocess.run([EXE, str(OUT / name), str(path)], capture_output=True, text=True, check=True,
                         env=settings()).stdout
    ons = []
    for ln in out.splitlines():
        w = ln.split()
        if "MIDI" in w and w[w.index("MIDI") + 2] == "90" and w[-1] != "0" and int(w[0]) > PLAY + 2000:
            ons.append(int(w[0]) - PLAY)
    return ons


got = {}
for steps, name in ((0, "NOCHE"), (1, "CALLE"), (2, "FIESTA")):
    ons = run(name.lower(), steps)
    on_grid = all(abs(t - round(t / 125) * 125) <= 12 for t in ons)
    pos = [round(t / 125) % 32 for t in ons]
    on_clave = sum(p in CLAVE for p in pos)
    got[name] = len(ons)
    check(f"{name}: {len(ons)} notes, every one on the sequencer's sixteenths", bool(ons) and on_grid,
          [t for t in ons if abs(t - round(t / 125) * 125) > 12][:5])
    if name == "NOCHE":
        check(f"NOCHE: mostly on the clave's strokes ({on_clave} of {len(ons)})", ons and on_clave >= 0.6 * len(ons))
check(f"busier as it goes: NOCHE {got['NOCHE']} < CALLE {got['CALLE']} < FIESTA {got['FIESTA']}",
      got["NOCHE"] < got["CALLE"] < got["FIESTA"])
print("gen clave test: passed" if not bad else "gen clave test: FAILED")
sys.exit(1 if bad else 0)
