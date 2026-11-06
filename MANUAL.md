# COQUÍ manual

Version 0.1 · for the M-VAVE FM-1

COQUÍ is built on [GR!MM](https://github.com/GRiMMBIENT/grimm) 2.0.2. This manual covers what COQUÍ adds and changes.
Everything else (the four tracks, HOME and the mixer, SEQ, ARP, SEL's scales and chords, GLO's song mode, FX and
PERFORM, GENERATE, SYSTEM, SAVE) works as
[GR!MM 2.0's manual](https://github.com/GRiMMBIENT/grimm/blob/v2.0/MANUAL.md) describes it, apart from the VOICE source
and its choirs, which COQUÍ does not have.

![PERC on the SOUND page](docs/screens/perc.png)

## 1. PERC: the percussion source

A track plays PERC when its source is PERC: **EDIT**, then **EDIT** again for SOURCE, **KNOB 1** to PERC. Or turn
**PRESETS** to one of the PERC kits (category PERC).

On a PERC track every key is a drum, not a note:

| White keys | F3 | G3 | A3 | B3 | C4 | D4 | E4 | F4 | G4 | A4 | B4 | C5 | D5 | E5 | F5 | G5 |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| | KICK | SNARE | CLAP | CASCARA | COWBELL | CLAVE | GUIRO | TUMBA | CONGA | QUINTO | BONGO LO | BONGO HI | TIMBAL LO | TIMBAL HI | BOOM | TAMBORA |

| Black keys | F#3 | G#3 | A#3 | C#4 | D#4 | F#4 | G#4 | A#4 | C#5 | D#5 | F#5 |
|---|---|---|---|---|---|---|---|---|---|---|---|
| | HAT | OPEN HAT | MARACAS | BELL | WOOD | SLAP | MUTE | GUIRO LONG | BONGO SLAP | PAILA | RIM |

- The lower half is the kit and the salsa section (cowbell, clave, güiro); the upper half the hand drums.
- The conga has three strokes: **G4** open, **G#4** mute, **F#4** slap. Each one cuts the one before, as a hand does.
- The hats cut each other (HAT stops OPEN HAT), and so do the two güiros.
- **OCT-** and **OCT+** fold back to the same map: a PERC track plays the same drums at any octave.
- MIDI notes 53 (F3) to 79 (G5) play these drums; others fold in by two octaves.
- The SOUND page shows the hand drums; the one last struck lights up, and its name is on the right of the caption.

### The SOUND page's knobs

| KNOB 1 | KNOB 2 | KNOB 3 | KNOB 4 |
|---|---|---|---|
| KIT | TUNE | DECAY | HUMAN |

- **KIT**:
  - SALSA: the drums as made;
  - PERREO: a long low kick, a tight snare, drier and brighter congas;
  - BOMBA: deeper, longer barriles with less slap;
  - TIPICO: tighter and higher, for merengue and cumbia.
- **TUNE**: every drum up or down by up to 12 semitones (0 in the middle).
- **DECAY**: every drum shorter or longer, from 25 % to 400 % (100 % in the middle).
- **HUMAN**: each hit a little off, at random: up to ~9 ms late, up to a third softer, slightly off-pitch. 0 is a
  machine; around 25 % a good hand; 100 % a loose one.

### How PERC plays

- A PERC track's hits do not use the six voices the other tracks share. Up to ten hits sound at once over all PERC
  tracks; an eleventh takes the quietest.
- ENV, LATCH, GLIDE, chord mode and ARP's chords do not apply to PERC: a key is a hit. ARP and GENERATE still play it
  (one drum at a time).
- HOME's VOLUME, CUTOFF, RESONANCE and FX, the mixer's sends and PAN work on a PERC track as on any other.
- Played from the keys or the sequencer, a hit goes out over USB-MIDI as a short note (note on, note off at once).

## 2. GROOVE: the styles

![The GROOVE page: SALSA 2-3](docs/screens/groove.png)

A PERC track can play a whole Latin or Caribbean rhythm by itself while the sequencer runs (**PLAY**), on the same tempo
and beside the track's own steps. On a PERC track, **EDIT** goes SOUND → **GROOVE** → SOURCE.

| KNOB 1 | KNOB 2 | KNOB 3 | KNOB 4 |
|---|---|---|---|
| STYLE | PARTS | FILL | FEEL |

- **STYLE** (OFF, then 14 styles; two bars each):

  | Style | What plays |
  |---|---|
  | DEMBOW | reggaetón: the kick on every beat, the snare's dembow, hats, a clap |
  | PERREO | old-school reggaetón: the 808 boom, the rim's dembow, an open hat |
  | SALSA 2-3 | son clave 2-3, cowbell, güiro, the conga's tumbao, the bongó's martillo |
  | SALSA 3-2 | clave 3-2 and the timbal's cáscara (the verse feel) |
  | SON | son montuno: martillo, maracas, clave, a lighter tumbao |
  | MAMBO | the mambo bell, the bongo bell, the full tumbao, a hot kick |
  | CHACHA | cha-cha-chá on the güiro and the bell |
  | BOMBA SICA | bomba sicá: the cuá's sticks, the buleador, the subidor |
  | BOMBA YUBA | bomba yubá in 6/8 |
  | PLENA | the seguidor, the punteador on the off-beats, the requinto |
  | MERENGUE | the güira, the tambora's stick and hand |
  | BACHATA | the bongó with its fourth beat, the güira |
  | CUMBIA | the guacharaca, the llamador, the tambora |
  | SOCA | four on the floor, the iron, the snare's two and four |

- **PARTS**: which drums play: TODO (all), KIT (kick, snare, hats, clap, rim, boom), MANOS (congas, bongos, timbales,
  tambora), CAMPANA (bells, clave, wood, güiro, maracas, cáscara), or two of them (KIT+MAN, MAN+CAM, KIT+CAM). Put the
  same STYLE on two PERC tracks with different PARTS to give the hands and the bells their own level, pan and effects.
  On the page, the drums PARTS leaves out are dimmed.
- **FILL**: OFF, or the style's fill in the last bar of every 2, 4 or 8 bars (counted from PLAY).
- **FEEL**: the swing of the off-beat sixteenths (triplets in YUBA). 0 is the style's own feel (bomba, plena, son and
  cumbia lean back a little); turn it down for straighter, up for looser.

The page shows the style's drums, a row each, their two bars of strokes (accents in the accent colour), and the step
playing. **HUMAN** on the SOUND page loosens the groove's hits too. The groove's strokes are a little softer than a key
struck, so a busy style stays clean. With SEQ MIDI OUT on (SYSTEM 2), its hits go out over USB-MIDI too.

The factory kits (PRESETS, category PERC) each bring a style: SALSA DURA (SALSA 2-3), PERREO 2004 (PERREO), BOMBA SICA,
PLENA CALLE (PLENA), MERENGUE, CUMBIA, MAMBO, BACHATA, DEMBOW, BARRIL LENTO (BOMBA YUBA), all with FILL every 4 bars.
A sound saved from a PERC track keeps its GROOVE; so do the project and the slots.

## 3. The lines: tumbao, montuno, guajeo

The ARP plays Latin lines over the chord you hold, on the sequencer's bars (the groove's), on any sound: hold **ARP** to
switch it on, tap **ARP** for its page, turn **MODE** past CHORD:

| MODE | What it plays (two bars of sixteenths) |
|---|---|
| TUMBAO | the salsa bass: the fifth on 2-and, the root anticipated on 4, tied over the bar |
| MONTUNO | the piano montuno: off-beat eighths in octaves (2-3) |
| GUAJEO | the tres's line: root, third, fifth and octave in off-beats |
| MERENGUE | the merengue bass: 1 and 3, a push into the next bar |
| BACHATA | the bachata bass: long notes and its 2-and |

- The line takes the **chord you hold**: three keys, or one key with chord mode on (SEL held), or even **one key
  alone** (its major chord: the third and fifth it lacks are filled in). A new chord takes over at the line's next note,
  the rhythm going on, so a chord change a moment before the bar lands the anticipations.
- **OCTAVES** is the register in these modes: 1 an octave under what you hold (a bass), 2 as held, 3 and 4 higher.
- **GATE** shortens each note (100 %: as written, ties included). **RATE** is the line's own.
- Stopped, the line runs on the tempo from the first chord; playing, on the sequencer's bars.

The factory sounds that bring a line switch the ARP on in its mode when loaded (another factory sound switches a line
off): TUMBAO BASS, SALSA UPRIGHT (TUMBAO), MERENGUE BASS, BACHATA BASS, MONTUNO PIANO, GUAJEO PIANO, CUATRO GUAJEO.
Turn PRESETS to TUMBAO BASS, hold C, press PLAY: a salsa bass. A sound you save keeps the timbre, not the ARP (as in
GR!MM: the ARP is the project's).

## 4. The factory sounds

| Category | Sounds |
|---|---|
| LEAD | DEMBOW LEAD, PERREO PLUCK, TROPICAL SYNTH |
| PAD | NOCHE PAD, BRISA, MAR CALMO, MALECON PAD |
| BRASS | MAMBO BRASS, TROMBONE SALSA, TRUMPET STAB, SAX MERENGUE |
| ORGAN | BOOGALOO ORGAN, SALSA ORGAN, SKANK ORGAN |
| KEYS | MONTUNO PIANO, SALON PIANO, GUAJEO PIANO, BOLERO EP |
| BASS | TUMBAO BASS, BABY BASS, MERENGUE BASS, BACHATA BASS, 808 BOOM, SALSA UPRIGHT |
| CUERDAS | CUATRO, CUATRO GUAJEO, TRES CUBANO, REQUINTO, BACHATA GUITAR |
| PERC | SALSA DURA, PERREO 2004, BOMBA SICA, PLENA CALLE, MERENGUE, CUMBIA, MAMBO, BACHATA, DEMBOW, BARRIL LENTO |
| OTHER | STEEL PAN, MARIMBA, VIBRAFONO, SILBIDO, FLAUTA CHARANGA |

The FM source has four new patches for them (PATCH on its SOUND page): BRASS, PIANO, CUATRO and BASS, beside BELL,
ORGAN, SOFT PAD and REED.

## 5. The mood: HOME, GENERATE, AMBIENTE

![HOME: the coquí sings, the congas burn](docs/screens/home.png)

**HOME** is a night: the stars brighten with the sound, the moon, the coquí (it sings its CO-QUÍ! at a loud hit and now
and then while you play; ~4 s of silence and it falls asleep, the first sound wakes it), and a conga a track with a
neon flame as tall as the track's sound.

**GENERATE** (hold FX and tap a black key: F#3 .. C#4 for T1 .. T4, D#4 the MODE) plays in the clave: every note on a
sixteenth of a 2-3 son clave's two bars (the sequencer's bars while it plays), each sixteenth's chance its place in it:

| MODE | What it plays |
|---|---|
| NOCHE | a few long soft notes with slow attacks, on the clave's strokes: a bolero's night |
| CALLE | a melody walking the scale in eighths, the clave's strokes accented |
| FIESTA | dense syncopated sixteenths, short, the clave loud: a montuno's |

![AMBIENTE: DUB](docs/screens/ambiente.png)

**AMBIENTE** (FX's last page, after DRIVE): one knob, a scene for all the effects at once, with the tracks' sends where it
needs them. Each value stays yours to turn on the effects' own pages; the boxes show which effects the scene left on.

| SCENE | What it sets |
|---|---|
| SECO | dry: no room, no echo; TAPE, MASTER, DRIVE off; the sends down |
| QUINTAL | a backyard: a short warm room |
| CALLE | the street: a slapback echo off the walls, a medium room |
| PLAYA | the beach: an open bright space, a soft echo |
| NOCHE | the night: a long dark reverb, a slow echo |
| DUB | a long dark echo feeding back, a low-pass over it all, the delay sends up |
| RADIO AM | an old radio: a band-pass, a clipping drive, the tape's wobble and hiss on every track |

## 6. What else is different from GR!MM

- **Identity**: COQUÍ is `FM-1_923`. Installers and the Firmware Hub tell it apart from GR!MM by this.
- **Saves**: COQUÍ keeps its own sounds, slots, settings and samples, and never reads another firmware's. Back up
  before you install it over another firmware.
- **Newer FM-1s**: the flash chip of FM-1s sold since autumn 2026 (UC25HQ80) works too: saves stay, and updates install.
- **Colours**: SYSTEM's COLOUR offers ROSA (the default), MANGO, MAR and LIMA over a night-blue palette.
- **Boot**: the coquí sings under the moon for a few seconds.
- **The web editor** (the hub's EDITOR, Chrome or Edge, the FM-1 on USB, or TRY WITHOUT AN FM-1): sounds, the SOUND
  EDITOR live on the tracks (a PERC sound: its GROOVE too), the SEQUENCER, the samples, FX, SETTINGS and a BACKUP,
  as GR!MM's editor works, in COQUÍ's colours and formats.
