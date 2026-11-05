/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 COQUÍ */
/* COQUÍ's GROOVE: a PERC track's own rhythm, a Latin or Caribbean style played on the sequencer's clock while PLAY
 * runs, beside the track's own steps.
 *   a style: lanes, each a drum (a key of perc.c's map) and its strokes over two bars (32 sixteenths; a 6/8 style: 24
 *   eighth-note triplets), and a one-bar FILL whose drums take the place of the same drums' lanes in a phrase's last bar
 *   strokes: '.' none, 'g' a ghost, 'o' soft, 'x' played, 'X' accented (perc_hit's velocity)
 *   PARTS: which of the drums play: TODO (all), KIT (kick, snare, hats, clap, rim, boom), MANOS (the hand drums and
 *   timbales), CAMPANA (bells, clave, wood, güiro, maracas, cáscara), or two of them: split a style over two PERC
 *   tracks (their own levels, FX and pans)
 *   FILL: OFF, or the style's fill in the last bar of every 2, 4 or 8 bars (counted from PLAY)
 *   FEEL: the swing of the off sixteenths (or triplets), -64 .. +63: 0 the style's own; less straighter, more looser
 *   Rhythms as played in the styles (the claves, the tumbao, the martillo, the dembow, the bomba and plena drums);
 *   written for COQUÍ, simplified to what a two-bar loop holds.
 * Audio ISR only (groove_tick from seq_tick); the main loop sets the track's grv_* (32-bit volatiles). */

enum { GK_KICK, GK_HAT, GK_SNARE, GK_OHAT, GK_CLAP, GK_MARAC, GK_CASC, GK_COWB, GK_BELL, GK_CLAVE, GK_WOOD, GK_GUIRO,
       GK_TUMBA, GK_SLAP, GK_CONGA, GK_MUTE, GK_QUINTO, GK_GUIROL, GK_BONGL, GK_BONGH, GK_BSLAP, GK_TIMBL, GK_PAILA,
       GK_TIMBH, GK_BOOM, GK_RIM, GK_TAMBORA };   /* (perc.c's keys, F3 .. G5) */
_Static_assert(GK_TAMBORA + 1 == PK_N, "a key a drum");

#define GR_LANES 10u                            /* a style's lanes at most */
#define GR_FLANES 5u                            /* its fill's */
typedef struct { uint8_t k; const char *s; } gr_lane_t;
typedef struct {
    const char *name;
    uint8_t steps, tp;                          /* its steps (32: sixteenths, 24: 6/8 triplets) and ticks a step (24, 32) */
    uint8_t feel;                               /* its own swing, 0..127 (FEEL 0) */
    gr_lane_t l[GR_LANES];                      /* (k 0 s 0: no lane) */
    gr_lane_t f[GR_FLANES];                     /* the fill: one bar (16 or 12 steps) */
} gr_style_t;

/*                                              |1 . . . 2 . . . 3 . . . 4 . . . |1 . . . 2 . . . 3 . . . 4 . . . | */
static const gr_style_t GR_STYLE[] = {
    {"DEMBOW", 32, 24, 0,                       /* reggaetón: the kick on every beat, the snare's 3-3-2 */
     {{GK_KICK,  "X...x...X...x...X...x...X...x..."},
      {GK_SNARE, "...x..X....x..X....x..X....x..X."},
      {GK_HAT,   "o.x.o.x.o.x.o.x.o.x.o.x.o.x.o.x."},
      {GK_CLAP,  "......x.............x.........x."},
      {GK_TIMBH, "...............................X"}},
     {{GK_SNARE, "...x..x....x..x."}, {GK_CLAP, "........x.x.xxXX"}, {GK_TIMBL, "............X.X."}}},
    {"PERREO", 32, 24, 0,                       /* the old-school one: the 808 boom, the rim's dembow, an open hat */
     {{GK_BOOM,  "X.......x.......X.......x......."},
      {GK_KICK,  "....x.......x.......x.......x..."},
      {GK_RIM,   "...x..X....x..X....x..X....x..X."},
      {GK_HAT,   "x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x."},
      {GK_OHAT,  "..............x...............x."},
      {GK_SNARE, "......X...............X........."}},
     {{GK_RIM, "x.x.x.x.x.xxxxXX"}, {GK_BOOM, "X..............."}}},
    {"SALSA 2-3", 32, 24, 0,                    /* son clave 2-3, the bell, the güiro, the conga's tumbao, martillo */
     {{GK_CLAVE, "....X...X.......X.....X.....X..."},
      {GK_COWB,  "X...x...X...x...X...x...X...x..."},
      {GK_GUIRO, "X.xxX.xxX.xxX.xxX.xxX.xxX.xxX.xx"},
      {GK_MUTE,  "g.g...g.g.g...g.g.g...g.g.g...g."},
      {GK_SLAP,  "....X...............X..........."},
      {GK_CONGA, "............x.x.............x.x."},
      {GK_TUMBA, "..............X...............X."},
      {GK_BONGH, "x.o.x.o.x.o.x.o.x.o.x.o.x.o.x.o."},
      {GK_BONGL, "..............x...............x."},
      {GK_KICK,  "......o.......x.......o.......x."}},
     {{GK_TIMBH, "x.x.x.xxx.x.X..."}, {GK_TIMBL, "..........x.X.X."}, {GK_PAILA, "x.x.x.x........."},
      {GK_BONGH, "................"}}},
    {"SALSA 3-2", 32, 24, 0,                    /* clave 3-2 and the timbal's cáscara (the verse) */
     {{GK_CLAVE, "X.....X.....X.......X...X......."},
      {GK_CASC,  "X.x.xX.x.X.xX.x.X.x.xX.x.X.xX.x."},
      {GK_GUIRO, "X.xxX.xxX.xxX.xxX.xxX.xxX.xxX.xx"},
      {GK_MUTE,  "g.g...g.g.g...g.g.g...g.g.g...g."},
      {GK_SLAP,  "....X...............X..........."},
      {GK_CONGA, "............x.x.............x.x."},
      {GK_TUMBA, "..............X...............X."},
      {GK_BONGH, "x.o.x.o.x.o.x.o.x.o.x.o.x.o.x.o."},
      {GK_KICK,  "......o.......x.......o.......x."}},
     {{GK_TIMBH, "x.x.x.x.xxxxX..."}, {GK_TIMBL, "............X.X."}, {GK_CASC, "................"}}},
    {"SON", 32, 24, 14,                         /* son montuno: slower, the bongó's martillo, maracas, no bell */
     {{GK_CLAVE, "....X...X.......X.....X.....X..."},
      {GK_MARAC, "x.o.x.o.x.o.x.o.x.o.x.o.x.o.x.o."},
      {GK_BONGH, "X.g.x.g.X.g.x.g.X.g.x.g.X.g.x.g."},
      {GK_BONGL, "..........x...X...........x...X."},
      {GK_BSLAP, "......x...............x........."},
      {GK_MUTE,  "g.....g.g.....g.g.....g.g.....g."},
      {GK_CONGA, "............x.x.............x.x."},
      {GK_GUIRO, "X...x.x.X...x.x.X...x.x.X...x.x."}},
     {{GK_BONGH, "x.x.x.x.xxx.X..."}, {GK_BONGL, "..........x.X.X."}}},
    {"MAMBO", 32, 24, 0,                        /* the mambo bell (2-3), the bongo bell, the full tumbao, a hot kick */
     {{GK_CLAVE, "....X...X.......X.....X.....X..."},
      {GK_BELL,  "X.x.X.xxX.x.xX.xX.x.X.xxX.x.xX.x"},
      {GK_COWB,  "X.......X.......X.......X......."},
      {GK_GUIRO, "X.xxX.xxX.xxX.xxX.xxX.xxX.xxX.xx"},
      {GK_MUTE,  "g.g...g.g.g...g.g.g...g.g.g...g."},
      {GK_SLAP,  "....X...............X..........."},
      {GK_CONGA, "............x.x.............x.x."},
      {GK_TUMBA, "..............X...............X."},
      {GK_TIMBL, "...............X...............X"},
      {GK_KICK,  "......x.......X.......x.......X."}},
     {{GK_TIMBH, "x.x.xxx.x.x.X..."}, {GK_TIMBL, "......x.xx.xX.X."}, {GK_BELL, "................"}}},
    {"CHACHA", 32, 24, 0,                       /* cha-cha-chá: one, two, cha-cha-chá on the güiro and the bell */
     {{GK_GUIRO, "X...X...X...X.x.X...X...X...X.x."},
      {GK_COWB,  "X...x...X...x...X...x...X...x..."},
      {GK_CLAVE, "....X...X.......X.....X.....X..."},
      {GK_MUTE,  "g...g...g.......g...g...g......."},
      {GK_SLAP,  "....X...............X..........."},
      {GK_CONGA, "............x.x.............x.x."},
      {GK_TIMBL, "............x.x.............x.x."}},
     {{GK_TIMBH, "x.x.x.x.x.x.X..."}, {GK_TIMBL, "............x.X."}}},
    {"BOMBA SICA", 32, 24, 22,                  /* bomba sicá: the cuá's sticks, the buleador's low call, the subidor */
     {{GK_WOOD,  "X.x.X.xxX.x.X.xxX.x.X.xxX.x.X.xx"},
      {GK_TUMBA, "X.....x.X.....x.X.....x.X.....x."},
      {GK_MUTE,  "...g.g.....g.g.....g.g.....g.g.."},
      {GK_MARAC, "x.o.x.o.x.o.x.o.x.o.x.o.x.o.x.o."},
      {GK_QUINTO, "..........X...X.....x.....X.X..."},
      {GK_SLAP,  "............X...............X..."}},
     {{GK_QUINTO, "X.x.X.x.X.xxX.X."}, {GK_TUMBA, "X.......X......."}}},
    {"BOMBA YUBA", 24, 32, 18,                  /* bomba yubá in 6/8 (a step: an eighth-note triplet; two bars of 6) */
     /*                                            |1 . . 2 . . |1 . . 2 . . |1 . . 2 . . |1 . . 2 . . | */
     {{GK_WOOD,  "X.xX.xX.xX.xX.xX.xX.xX.x"},
      {GK_TUMBA, "X..x..X..x..X..x..X..x.."},
      {GK_MUTE,  ".g...g.g...g.g...g.g...g"},
      {GK_MARAC, "x.xx.xx.xx.xx.xx.xx.xx.x"},
      {GK_QUINTO, "......X..x........X.X..."}},
     {{GK_QUINTO, "X.xX.xX.xXXX"}, {GK_TUMBA, "X.....X....."}}},
    {"PLENA", 32, 24, 16,                       /* plena: the seguidor, the punteador on the off-beats, the requinto */
     {{GK_TUMBA, "X.....x.X.....x.X.....x.X.....x."},
      {GK_MUTE,  "..x...x...x...x...x...x...x...x."},
      {GK_CONGA, "......x.......x.......x.......x."},
      {GK_QUINTO, "x..x..x.....x.x.....x..x..X....."},
      {GK_GUIRO, "X.x.X.x.X.x.X.x.X.x.X.x.X.x.X.x."}},
     {{GK_QUINTO, "x.x.xxX.x.x.X.X."}, {GK_TUMBA, "X.......X...X..."}}},
    {"MERENGUE", 32, 24, 0,                     /* merengue: the güira's scrape, the tambora's stick and hand */
     {{GK_HAT,   "X.xxX.xxX.xxX.xxX.xxX.xxX.xxX.xx"},
      {GK_CASC,  "x.x.x.xxx.x.x.xxx.x.x.xxx.x.x.xx"},
      {GK_TAMBORA, "....x.......X.X.....x.......X.X."},
      {GK_KICK,  "X.......x.......X.......x......."},
      {GK_COWB,  "X...x...X...x...X...x...X...x..."}},
     {{GK_TAMBORA, "x.x.x.x.x.xxX.X."}, {GK_CASC, "xxxxxxxxxxxx...."}}},
    {"BACHATA", 32, 24, 0,                      /* bachata: the bongó's martillo with its fourth beat, the güira */
     {{GK_BONGH, "X.x.x.x.X.x.x.x.X.x.x.x.X.x.x.x."},
      {GK_BONGL, "............X...............X..."},
      {GK_BSLAP, "..............x...............x."},
      {GK_HAT,   "x.xxx.xxx.xxx.xxx.xxx.xxx.xxx.xx"},
      {GK_KICK,  "X.......X.......X.......X......."}},
     {{GK_BONGH, "x.x.x.x.x.xxX..."}, {GK_BONGL, "............x.X."}}},
    {"CUMBIA", 32, 24, 12,                      /* cumbia: the guacharaca, the llamador's off-beats, the tambora */
     {{GK_GUIRO, "X.xxX.xxX.xxX.xxX.xxX.xxX.xxX.xx"},
      {GK_CONGA, "....X.......X.......X.......X..."},
      {GK_MUTE,  "..g...g...g...g...g...g...g...g."},
      {GK_TUMBA, "X.......x.......X.......x......."},
      {GK_MARAC, "x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x."}},
     {{GK_TUMBA, "X...X...X.x.X.X."}, {GK_CONGA, "....x.......x.xx"}}},
    {"SOCA", 32, 24, 0,                         /* soca: four on the floor, the iron (bell) driving, the snare's two-four */
     {{GK_KICK,  "X...X...X...X...X...X...X...X..."},
      {GK_SNARE, "....x.......x.......x.......x..."},
      {GK_COWB,  "x.xxx.xxx.xxx.xxx.xxx.xxx.xxx.xx"},
      {GK_OHAT,  "..x...x...x...x...x...x...x...x."},
      {GK_CONGA, "......x.......x.x.....x.......x."}},
     {{GK_SNARE, "x.x.x.x.xxxxXXXX"}, {GK_TIMBH, "............X..."}}},
};
#define NSTYLE (sizeof GR_STYLE / sizeof GR_STYLE[0])
static const char *const GR_SHORT[] = {"DEMBOW", "PERREO", "SAL2-3", "SAL3-2", "SON", "MAMBO", "CHACHA", "SICA", "YUBA",
                                       "PLENA", "MRENGE", "BACHTA", "CUMBIA", "SOCA"};   /* (the knob's slab: 6 letters) */
_Static_assert(sizeof GR_SHORT / sizeof GR_SHORT[0] == NSTYLE, "a short name a style");
#define GR_NSTYLE (NSTYLE + 1u)                 /* STYLE's values: 0 OFF, then the styles */

enum { GP_TODO, GP_KIT, GP_MANOS, GP_CAMP, GP_KM, GP_MC, GP_KC, GP_N };
static const char *const GP_NAME[GP_N] = {"TODO", "KIT", "MANOS", "CAMPANA", "KIT+MAN", "MAN+CAM", "KIT+CAM"};
static const uint8_t GP_MASK[GP_N] = {7, 1, 2, 4, 3, 6, 5};   /* bit 0 KIT, 1 MANOS, 2 CAMPANA */
static const uint8_t GR_FILLN[4] = {0, 2, 4, 8};   /* FILL: OFF, every 2, 4, 8 bars */
static uint32_t gr_sect(uint32_t k)             /* a key's part: 1 KIT, 2 MANOS, 4 CAMPANA */
{
    switch (k) {
    case GK_KICK: case GK_HAT: case GK_SNARE: case GK_OHAT: case GK_CLAP: case GK_BOOM: case GK_RIM: return 1u;
    case GK_MARAC: case GK_CASC: case GK_COWB: case GK_BELL: case GK_CLAVE: case GK_WOOD: case GK_GUIRO: case GK_GUIROL:
    case GK_PAILA: return 4u;
    default: return 2u;
    }
}
static uint32_t gr_vel(char c)                  /* a stroke's velocity, Q8 for perc_hit (0: none): ~3.5 dB under a key's, so
                                                 * a busy style's drums together stay under full scale (tests/perc) */
{
    return c == 'X' ? 170u : c == 'x' ? 130u : c == 'o' ? 90u : c == 'g' ? 50u : 0u;
}

static uint32_t grv_lit[NT];                    /* audio: the step a track's groove played last + 1 (the GROOVE page) */
static void groove_hit(uint32_t t, uint32_t k, uint32_t v)
{
    if (trk_quiet >> t & 1u) return;            /* (muted, or another soloed) */
    perc_hit(t, PK_LO + k, v);
    if (mo_seq) ec_put(0x90, t, (PK_LO + k) | EC_SEQ), ec_put(0x80, t, (PK_LO + k) | EC_SEQ);   /* (SEQ MIDI OUT) */
}
/* audio: a tick of the sequencer (tk: ticks since PLAY, 96 a quarter note) for track t */
static void groove_tick(uint32_t t, uint32_t tk)
{
    const gr_style_t *G;
    uint32_t st = (uint32_t)clamp(trk[t].grv_style, 0, (int32_t)NSTYLE), tp, sw, loc, s, i, j, mask, fn, fill, fs;
    if (!st || trk[t].eng != ENG_PERC) return;
    G = &GR_STYLE[st - 1u];
    tp = G->tp;
    {                                           /* FEEL: the style's swing, moved (64: as it is) */
        int32_t f = (int32_t)G->feel + clamp(trk[t].grv_feel, -64, 63) * 2;
        sw = (uint32_t)clamp(f, 0, 127) * (tp / 2u) / 127u;   /* (up to half a step late) */
    }
    loc = tk % (2u * tp);
    if (loc == 0u) s = tk / tp;
    else if (loc == tp + sw && sw) s = tk / tp;
    else if (loc == tp && !sw) s = tk / tp;
    else return;
    fs = G->steps / 2u;                         /* (a bar) */
    s %= G->steps;
    fn = GR_FILLN[clamp(trk[t].grv_fill, 0, 3)];
    fill = fn && (tk / SEQ_BAR) % fn == fn - 1u;   /* (the phrase's last bar) */
    mask = GP_MASK[clamp(trk[t].grv_parts, 0, GP_N - 1)];
    grv_lit[t] = s + 1u;
    for (i = 0; i < GR_LANES && G->l[i].s; i++) {
        const gr_lane_t *L = &G->l[i];
        uint32_t v;
        if (!(gr_sect(L->k) & mask)) continue;
        if (fill) {                             /* (a drum the fill plays: the fill's, not the lane's) */
            for (j = 0; j < GR_FLANES && G->f[j].s && G->f[j].k != L->k; j++)
                ;
            if (j < GR_FLANES && G->f[j].s) continue;
        }
        if ((v = gr_vel(L->s[s])) != 0u) groove_hit(t, L->k, v);
    }
    if (fill)
        for (j = 0; j < GR_FLANES && G->f[j].s; j++)
            if ((gr_sect(G->f[j].k) & mask)) {
                uint32_t v = gr_vel(G->f[j].s[s % fs]);
                if (v) groove_hit(t, G->f[j].k, v);
            }
}
static void groove_text(uint32_t k, int32_t v, char *s)   /* the GROOVE page's knob k at v */
{
    switch (k) {
    case 0: put_s(s, v <= 0 ? "OFF" : GR_STYLE[clamp(v, 1, (int32_t)NSTYLE) - 1].name); return;
    case 1: put_s(s, GP_NAME[clamp(v, 0, GP_N - 1)]); return;
    case 2: if (v <= 0) put_s(s, "OFF"); else put_u(put_s(s, "/"), GR_FILLN[clamp(v, 0, 3)]); return;
    default: {
        int32_t d = clamp(v, -64, 63);
        if (d < 0) put_u(put_s(s, "-"), (uint32_t)-d);
        else put_u(put_s(s, d ? "+" : ""), (uint32_t)d);
        return;
    }
    }
}
