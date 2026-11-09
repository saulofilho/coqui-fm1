# COQUÍ — 10s promo

A short motion-graphics intro for COQUÍ (for a YouTube channel / social).

- **The video:** [coqui-promo-10s.mp4](coqui-promo-10s.mp4) — 10 s, 1920×1080, with a groove from the firmware itself as the soundtrack.

It runs five beats: the coquí singing under the moon → an equalizer and "a Latin groovebox" → the 22 / 14 / 5 counts → the GROOVE grid with the step sweeping ("it plays itself") → the call to action.

## How it was made

- `anim.html` — a deterministic animation driven by `renderAt(t)` (seconds): the coquí sprite (from the firmware's own `cq_frog`) singing and blinking, twinkling stars, a sound-bar equalizer, the number cards popping in, a sweep over the GROOVE screen, and a glowing wordmark.
- `capture_build.py` — launches headless Chrome, renders each of the 300 frames (30 fps) through the DevTools protocol, then assembles them with ffmpeg and adds the soundtrack.

Rebuild (needs Google Chrome, and Python with `websocket-client` and `imageio-ffmpeg`):

```
cp ../../hub/audio/band.mp3 .
python3 capture_build.py
```

The GROOVE screen comes from `img/` and the soundtrack from [`hub/audio/band.mp3`](../../hub/audio/band.mp3) (the firmware recorded in host mode).
