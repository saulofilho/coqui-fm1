#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
# COQUÍ: a PERC track's GROOVE is saved and comes back (host mode). SALSA DURA loaded (its GROOVE: SALSA 2-3, FILL /4), then
# on the GROOVE page STYLE +2 (SON), FILL +1 (/8), FEEL -10; the autosave; a new start from those saves: all as left.
#   python3 tests/perc/groove_save_test.py build/host/coqui
import os
import re
import subprocess
import sys
from pathlib import Path

BIN = sys.argv[1] if len(sys.argv) > 1 else "build/host/coqui"
OUT = Path("build/host/groove_save")
OUT.mkdir(parents=True, exist_ok=True)


def run(name, events, end, env=None):
    script = OUT / f"{name}.txt"
    script.write_text("".join(f"{t} {e}\n" for t, e in events) + f"{end} end\n")
    r = subprocess.run([BIN, str(OUT / name), str(script)], capture_output=True, text=True, env=env or os.environ)
    return re.findall(r"eng (\d+).*? grv (-?\d+),(-?\d+),(-?\d+),(-?\d+)", r.stdout)


bad = 0


def check(what, ok, got):
    global bad
    bad += not ok
    print(f"  {'ok  ' if ok else 'FAIL'}  groove save: {what}" + ("" if ok else f" ({got})"))


saves = OUT / "saves.bin"
p = run("set", [(6000, "enc 6 30"), (6200, "probe"), (6400, "btn 6"), (6500, "btn 6"), (6600, "enc 2 2"), (6700, "enc 4 1"),
                (6800, "enc 5 -10"), (6900, "probe")], 30000, dict(os.environ, GRIMM_SAVE_OUT=str(saves)))
check("SALSA DURA brings its GROOVE (SALSA 2-3, TODO, FILL /4, FEEL 0)", p[:1] == [("3", "3", "0", "2", "0")], p)
check("the GROOVE page's knobs move it (SON, FILL /8, FEEL -10)", p[1:2] == [("3", "5", "0", "3", "-10")], p)
q = run("load", [(6500, "probe")], 6600, dict(os.environ, GRIMM_SAVE_IN=str(saves)))
check("after a new start: PERC, SON, FILL /8, FEEL -10, as left", q == [("3", "5", "0", "3", "-10")], q)
print("groove save test: passed" if not bad else "groove save test: FAILED")
sys.exit(1 if bad else 0)
