#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
# Copyright (C) 2026 COQUÍ
"""COQUÍ's hub page (hub/): its data and its sounds, made from the firmware itself.

  hub/coqui-data.js   the styles (firmware/src/groove.c: names, steps, lanes, fills), the keys (perc.c), the version
  hub/audio/style-NN.mp3   each GROOVE style, four bars on its factory kit at its own tempo, as host mode plays it
  hub/audio/keys.mp3       the 27 keys of KIT SALSA one after another (coqui-data.js: where each starts)

Needs a host build (build/host/coqui: tests/run_host.sh makes it) and ffmpeg (FFMPEG, or the venv's imageio-ffmpeg).
  python3 tools/gen_hub.py [--version 0.1]
"""
import argparse
import json
import os
import re
import shutil
import subprocess
import sys
import tempfile
import wave
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SRC = ROOT / "firmware" / "src"
HUB = ROOT / "hub"
BIN = ROOT / "build" / "host" / "coqui"
FS = 44100
BOOT_MS = 6000                                  # (the boot screen: inputs after it)

# each style: its tempo and the factory kit it sounds best on (PRESETS' place: tests/... enc 6 N)
STYLE_PLAY = {
    "DEMBOW": (95, 38), "PERREO": (92, 31), "SALSA 2-3": (100, 30), "SALSA 3-2": (100, 30), "SON": (90, 30),
    "MAMBO": (106, 36), "CHACHA": (118, 30), "BOMBA SICA": (108, 32), "BOMBA YUBA": (100, 39), "PLENA": (104, 33),
    "MERENGUE": (128, 34), "BACHATA": (124, 37), "CUMBIA": (96, 35), "SOCA": (126, 31),
}
KIT_STYLE = {30: 3, 31: 2, 32: 8, 33: 10, 34: 11, 35: 13, 36: 6, 37: 12, 38: 1, 39: 9}   # a kit's own STYLE (store.c)


def ffmpeg():
    ff = os.environ.get("FFMPEG") or shutil.which("ffmpeg")
    if ff:
        return ff
    try:
        import imageio_ffmpeg
        return imageio_ffmpeg.get_ffmpeg_exe()
    except ImportError:
        sys.exit("gen_hub: no ffmpeg (set FFMPEG, or pip install imageio-ffmpeg)")


def parse_styles():
    g = (SRC / "groove.c").read_text()
    keys = re.search(r"enum \{ (GK_KICK.*?) \};", g, re.S).group(1)
    gk = [k.strip() for k in keys.replace("\n", " ").split(",") if k.strip()]
    body = g[g.index("static const gr_style_t GR_STYLE[] = {"):g.index("#define NSTYLE")]
    styles = []
    for m in re.finditer(r'\{"([^"]+)", (\d+), (\d+), (\d+),(.*?)\}\}\},?\s*(?=\{"|\};)', body, re.S):
        name, steps, tp, feel, rest = m.group(1), int(m.group(2)), int(m.group(3)), int(m.group(4)), m.group(5)
        main, _, fill = rest.partition("}},")
        main, fill = main + "}", fill + "}"     # (the last lane's brace, taken by the partition and the match)
        lane = re.compile(r'\{(GK_\w+),\s*"([.goxX]+)"\}')
        styles.append(dict(name=name, steps=steps, tp=tp, feel=feel,
                           lanes=[[gk.index(k), s] for k, s in lane.findall(main)],
                           fill=[[gk.index(k), s] for k, s in lane.findall(fill)]))
    short = re.search(r"GR_SHORT\[\] = \{(.*?)\};", g, re.S).group(1)
    for s, n in zip(styles, re.findall(r'"([^"]+)"', short)):
        s["short"] = n
    return styles


def parse_keys():
    p = (SRC / "perc.c").read_text()
    names = re.search(r"PKEY_NAME\[PK_N\] = \{(.*?)\};", p, re.S).group(1)
    return re.findall(r'"([^"]+)"', names)


def host(name, events, end_ms, out):
    script = out / f"{name}.txt"
    script.write_text("".join(f"{t} {e}\n" for t, e in sorted(events, key=lambda x: x[0])) + f"{end_ms} end\n")
    subprocess.run([str(BIN), str(out / name), str(script)], check=True, capture_output=True,
                   env=dict(os.environ, GRIMM_MASTER="1022"))
    with wave.open(str(out / f"{name}.wav")) as w:
        return w.getnchannels(), w.readframes(w.getnframes())


def cut(raw, ch, a_ms, b_ms):
    fr = 2 * ch
    return raw[a_ms * FS // 1000 * fr:b_ms * FS // 1000 * fr]


def mp3(ff, raw, ch, dst, fade_ms=600):
    """raw 16-bit PCM (ch channels) -> a mono 22.05 kHz MP3, one gain for all of it: its RMS to -18 dB, its peak at most
    -3 dB (every clip about as loud as the others, nothing squashed), faded out at its end"""
    import array
    import math
    a = array.array("h", raw)
    pk = max(1, max(abs(x) for x in a))
    rms = max(1.0, math.sqrt(sum(x * x for x in a[::7]) / max(1, len(a[::7]))))
    gain_db = min(20 * math.log10(32767 * 10 ** (-18 / 20) / rms), 20 * math.log10(32767 * 10 ** (-3 / 20) / pk))
    with tempfile.NamedTemporaryFile(suffix=".wav", delete=False) as t:
        with wave.open(t.name, "wb") as w:
            w.setnchannels(ch), w.setsampwidth(2), w.setframerate(FS), w.writeframes(raw)
        secs = len(raw) / (2 * ch * FS)
        af = f"volume={gain_db:.2f}dB"
        if fade_ms:
            af += f",afade=t=out:st={max(0.0, secs - fade_ms / 1000):.3f}:d={fade_ms / 1000:.3f}"
        subprocess.run([ff, "-y", "-loglevel", "error", "-i", t.name, "-ac", "1", "-ar", "22050", "-af", af,
                        "-codec:a", "libmp3lame", "-b:a", "64k", str(dst)], check=True)
    os.unlink(t.name)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--version", default="0.1")
    ap.add_argument("--no-audio", action="store_true", help="the data only")
    a = ap.parse_args()
    styles, keys = parse_styles(), parse_keys()
    assert len(styles) == 14 and len(keys) == 27, (len(styles), len(keys))
    out = ROOT / "build" / "hub"
    out.mkdir(parents=True, exist_ok=True)
    (HUB / "audio").mkdir(parents=True, exist_ok=True)
    key_at = []
    if not a.no_audio:
        if not BIN.exists():
            sys.exit("gen_hub: no build/host/coqui (run tests/run_host.sh first)")
        ff = ffmpeg()
        for i, s in enumerate(styles):
            bpm, kit = STYLE_PLAY[s["name"]]
            bar_ms = 4 * 60000 // bpm
            t0 = BOOT_MS + 1200
            ev = [(BOOT_MS, f"enc 0 {bpm - 120}"), (BOOT_MS + 100, f"enc 6 {kit}"), (BOOT_MS + 300, "btn 6"),
                  (BOOT_MS + 400, "btn 6"), (BOOT_MS + 500, f"enc 2 {i + 1 - KIT_STYLE[kit]}"), (t0, "btn 12")]
            ch, raw = host(f"style{i + 1:02d}", ev, t0 + 4 * bar_ms + 1500, out)
            mp3(ff, cut(raw, ch, t0, t0 + 4 * bar_ms + 1200), ch, HUB / "audio" / f"style-{i + 1:02d}.mp3")
            s["bpm"] = bpm
            print(f"style {i + 1:2d} {s['name']:<11} {bpm} BPM -> hub/audio/style-{i + 1:02d}.mp3")
        gap = 700                               # (a key every 0.7 s: its tail, its room's)
        t0 = BOOT_MS + 400
        ev = [(BOOT_MS, "enc 6 30")]            # (SALSA DURA: KIT SALSA, a little HUMAN)
        for k in range(27):
            ev += [(t0 + k * gap, f"midi 0x90 {53 + k} 120"), (t0 + k * gap + 40, f"midi 0x80 {53 + k} 0")]
        ch, raw = host("keys", ev, t0 + 27 * gap + 500, out)
        mp3(ff, cut(raw, ch, t0, t0 + 27 * gap), ch, HUB / "audio" / "keys.mp3", fade_ms=0)
        key_at = [round(k * gap / 1000, 3) for k in range(27)]
        print("keys -> hub/audio/keys.mp3")
    else:
        for s in styles:
            s["bpm"] = STYLE_PLAY[s["name"]][0]
    data = dict(version=a.version, identity="FM-1_923", styles=styles, keys=keys, keyAt=key_at, keyLen=0.7)
    (HUB / "coqui-data.js").write_text("/* GENERATED by tools/gen_hub.py from firmware/src/groove.c and perc.c: do not edit */\n"
                                       f"window.COQUI = {json.dumps(data, ensure_ascii=False, separators=(',', ':'))};\n")
    print("data -> hub/coqui-data.js")


if __name__ == "__main__":
    main()
