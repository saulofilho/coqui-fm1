# COQUÍ 1.0 on a real FM-1: test results

Tested on 2026-11-06 on a real M-VAVE FM-1, on USB to a Windows laptop, following [TESTE-FM1.md](TESTE-FM1.md) (the test
plan, in Portuguese), with the 1.0 package `coqui.fwsc` (identity `FM-1_923`). Tools: `tools/fm1_install.py` and
`tools/fm1_cpu.py` (Python 3.12, mido, python-rtmidi).

## Part A: what the FM-1 Firmware Hub asks for

| # | Step | Result |
|---|---|---|
| 1 | Start from the official firmware | ✅ the FM-1 answered `FM-1_015` (M-VAVE's V15) |
| 2 | Install from V15 | ✅ installed, restarted into COQUÍ |
| 3 | Identity over USB | ✅ `FM-1_923` (port "Felucca 0") |
| 4 | Leave COQUÍ | ✅ GR!MM (`FM-1_966`) installed over it from the Firmware Hub, and it started |
| 5 | Install from another firmware | ✅ from GR!MM (`FM-1_966`) to COQUÍ: "done: the FM-1 runs FM-1_923" |
| 6 | An interrupted update | ✅ the cable pulled out mid-write; the installer found the loader (`ota-FM-1_900`, "in update mode: the write will be finished"), finished the write, `FM-1_923` |
| 7 | USB rescue (OCT- at power-on) | ✅ (reported by the tester: all correct) |
| 8 | Back to the official firmware | not done (optional: the way out is shown by step 4) |

Installer output, step 5 (from GR!MM):

```
package: FM-1_923  (coqui.fwsc)
device:  FM-1_966  (Felucca 0)
starting the update on FM-1_966
checking the package (18 reads)
switching to update mode
writing 100%
restarting
done: the FM-1 runs FM-1_923
```

Step 6 (resuming the interrupted write):

```
package: FM-1_923  (coqui.fwsc)
device:  ota-FM-1_900  (Felucca Update 0)  in update mode: the write will be finished
writing 100%
restarting
done: the FM-1 runs FM-1_923
```

## Part B: COQUÍ itself

Played by ear: the boot screen, the grooves, the ARP lines (TUMBAO BASS, MONTUNO PIANO), the factory sounds, GENERATE
and AMBIENTE "all seem fine". No defect reported.

## Part C: the processor

The heaviest scene we could build (T1 SALSA DURA with its GROOVE, T2 TUMBAO BASS and T3 MONTUNO PIANO latched, T4 MAMBO
BRASS with GENERATE in FIESTA, AMBIENTE on DUB, PLAY), `fm1_cpu.py --once` after ~20 s:

```
NOW  31.8%  PEAK  47.3%  |  VOICES 16.9  MIX  3.5  GRAIN  0.4  DELAY  0.2  SPACE  7.9  TAPE  0.0  PERFORM  0.6  MASTER  1.1  DRIVE  0.0  SCAN  1.1  REST  0.0  |  USB REC DROPS 0
```

More than half of the 5.8 ms audio block to spare. Health (`--health`, the FM-1 idle): the watchdog's longest gap 617.7 ms
of its ~8000, no late key-scan ticks, no long interrupts-off times from the flash, no USB losses, 0 unexpected restarts,
0 faults.

## Note

On Windows the installer needs Python 3.12: python-rtmidi has no ready-made package for newer versions (the test plan says
so).
