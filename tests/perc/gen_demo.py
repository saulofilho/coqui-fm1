#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
# COQUÍ: a host-mode script for PERC (firmware/src/perc.c): T1 switched to PERC, then a salsa groove (clave 2-3,
# conga tumbao, cowbell, güiro, bongo martillo) and a dembow, as MIDI notes on channel 1. Writes the script to argv[1].
import sys

N = dict(KICK=53, HAT=54, SNARE=55, OHAT=56, CLAP=57, MARAC=58, CASC=59, COWB=60, BELL=61, CLAVE=62, WOOD=63,
         GUIRO=64, TUMBA=65, SLAP=66, CONGA=67, MUTE=68, QUINTO=69, GUIROL=70, BONGL=71, BONGH=72, BSLAP=73,
         TIMBL=74, PAILA=75, TIMBH=76, BOOM=77, RIM=78, TAMBORA=79)
ev = [(300, "btn 6"), (400, "btn 6"), (500, "enc 2 3"), (700, "btn 6")]   # EDIT: SOUND, SOURCE; KNOB 1: PERC; SOUND

def bar(t0, step_ms, pattern, vel=100):
    for name, steps in pattern.items():
        for i, c in enumerate(steps):
            if c in "xX":
                v = 120 if c == "X" else vel
                ev.append((t0 + i * step_ms, f"midi 0x90 {N[name]} {v}"))
                ev.append((t0 + i * step_ms + 40, f"midi 0x80 {N[name]} 0"))

salsa = {                      # one 2-3 clave cycle, 16 eighths... as 16ths of two bars: 32 steps
    "CLAVE":  "....X...X.......X.....X...X.....",
    "COWB":   "X...x...X...x...X...x...X...x...",
    "GUIRO":  "X.x.X.x.X.x.X.x.X.x.X.x.X.x.X.x.",
    "MUTE":   "X.......X.......X.......X.......",
    "SLAP":   "....X.......X.......X.......X...",
    "CONGA":  "............X.x.............X.x.",
    "TUMBA":  "..............X...............X.",
    "BONGH":  "..x.x...x.x...x...x.x...x.x...x.",
    "KICK":   "......X.......X.......X.......X.",
}
dembow = {
    "KICK":   "X...X...X...X...X...X...X...X...",
    "SNARE":  "...X..X....X..X....X..X....X..X.",
    "HAT":    "x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.",
    "OHAT":   "......x.......x.......x.......x.",
    "TIMBH":  "...............X...............X",
}
t = 1000
for _ in range(3):
    bar(t, 125, salsa); t += 32 * 125           # 120 BPM, 16ths
ev.append((t + 200, "btn 6")); ev.append((t + 300, "btn 6"))   # (EDIT round: back to SOUND)
ev.append((t + 400, "enc 2 1"))                                # KIT: PERREO
t += 1000
for _ in range(3):
    bar(t, 158, dembow); t += 32 * 158          # 95 BPM
ev.append((t + 1500, "end"))
with open(sys.argv[1], "w") as f:
    for tm, e in sorted(ev, key=lambda x: x[0]):
        f.write(f"{tm} {e}\n")
