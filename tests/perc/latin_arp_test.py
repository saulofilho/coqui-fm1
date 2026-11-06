#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
# COQUÍ: the ARP's Latin lines (firmware/src/arp.c: TUMBAO .. BACHATA), host mode, their notes out over USB-MIDI (SEQ
# MIDI OUT on). T1 (VA), ARP held on, its MODE turned to the line, C major held (C4 E4 G4 from MIDI), PLAY at 120 BPM (a
# sixteenth 125 ms): each line's notes on its steps, the chord's degrees, OCTAVES 1 an octave under; a new chord (F) taken
# at the next note, the rhythm on; MONTUNO in octaves.
#   python3 tests/perc/latin_arp_test.py build/host/coqui
import os
import subprocess
import sys
from pathlib import Path

EXE = sys.argv[1] if len(sys.argv) > 1 else "build/host/coqui"
OUT = Path("build/host/latin_arp")
OUT.mkdir(parents=True, exist_ok=True)
PLAY = 2000                                     # (ms: PLAY pressed; the line's step s at PLAY + 125 s, within a control step)
LINES = {                                       # (as arp.c's ARP_LAT)
    6: ("TUMBAO", "------5-----R---------5-----R---"),
    7: ("MONTUNO", "..5.3.5.R.5.3.5...5.3.5.O.5.3.O."),
    8: ("GUAJEO", "..R.3.5.O.5.3.R...R.3.5.O.5.3.5."),
    9: ("MERENGUE", "R...5...R...5...R...5...R.5.R.5."),
    10: ("BACHATA", "R-----R-5-------R-----R-5-------"),
}
bad = 0


def check(what, ok, detail=""):
    global bad
    print(f"  {'ok' if ok else 'FAIL':4s}  latin arp: {what}" + (f" ({detail})" if detail and not ok else ""))
    bad += not ok


def settings():                                 # (the settings, version 3: SEQ MIDI OUT on; as tests/quick_test.py)
    b = bytearray(170)
    b[5], b[167] = 1, 3
    f = OUT / "set.bin"
    f.write_bytes(bytes(b))
    return dict(os.environ, GRIMM_SETTINGS_IN=str(f))


def run(name, mode, chord_change=False, end=PLAY + 4100):
    ev = [(300, "down 10"), (1100, "up 10"), (1200, "btn 10"), (1300, f"enc 2 {mode}")]   # ARP on; its page; MODE
    ev += [(1500, f"midi 0x90 {n} 100") for n in (60, 64, 67)] + [(PLAY, "btn 12")]
    if chord_change:                            # F major from bar 2 (the first chord let go just before)
        ev += [(PLAY + 1990, f"midi 0x80 {n} 0") for n in (60, 64, 67)]
        ev += [(PLAY + 1995, f"midi 0x90 {n} 100") for n in (65, 69, 72)]
    path = OUT / f"{name}.txt"
    path.write_text("".join(f"{t} {e}\n" for t, e in sorted(ev, key=lambda x: x[0])) + f"{end} end\n")
    out = subprocess.run([EXE, str(OUT / name), str(path)], capture_output=True, text=True, check=True,
                         env=settings()).stdout
    ons = []
    for ln in out.splitlines():
        w = ln.split()
        if "MIDI" in w and w[w.index("MIDI") + 2] == "90" and w[-1] != "0":
            t = int(w[0])
            if t >= PLAY:
                ons.append((t - PLAY, int(w[w.index("MIDI") + 3])))
    return ons


DEG = {"R": 48, "3": 52, "5": 55, "O": 60, "L": 36}   # (C major held, OCTAVES 1: an octave under)
for mode, (name, line) in LINES.items():
    ons = run(name.lower(), mode)
    want = []
    for s in range(32):
        c = line[s]
        if c in DEG:
            want.append((s * 125, DEG[c]))
            if name == "MONTUNO":
                want.append((s * 125, DEG[c] + 12))
    got = [(round(t / 125) * 125, n) for t, n in ons if t < 4000]
    ok = sorted(got) == sorted(want) and all(abs(t - round(t / 125) * 125) <= 10 for t, _ in ons)
    check(f"{name}: its notes on its steps, the chord's degrees an octave under ({len(got)} notes)", ok,
          f"got {got[:8]}..., want {want[:8]}...")

ons = run("tumbao_f", 6, chord_change=True)
bar2 = [(round(t / 125) * 125, n) for t, n in ons if 2000 <= t < 4000]
check("TUMBAO: F major from bar 2: its fifth on 2-and, its root on 4 (C on, then F)",
      bar2 == [(2750, 60), (3500, 53)], bar2)
print("latin arp test: passed" if not bad else "latin arp test: FAILED")
sys.exit(1 if bad else 0)
