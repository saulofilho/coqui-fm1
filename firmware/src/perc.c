/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 COQUÍ (sine_i, noise32: Felucca's dsp.c, Leo Kuroshita, Hügelton Instruments, as GR!MM had them) */
/* COQUÍ's PERC source: Latin and Caribbean percussion, synthesised (no samples, no flash, no pool).
 *   A PERC track's notes are hits, not voices: each key a drum (perc_key: the default octave's 27 keys, F3 .. G5, hold the
 *   whole kit; notes outside fold in by two octaves), played by its own pool of NPH hits, never the six voices the other
 *   tracks share. A hit: a tone (a sine, a second partial, a pitch drop at the start) and a noise (a band- or high-pass,
 *   its own decay), each decaying on its own; a few drums have their own trick (the clap's three bursts, the güiro's
 *   ratchet, the maracas' soft attack). A drum struck again, or its other stroke (the conga's open, mute and slap; the
 *   hats), cuts the one sounding (perc_hit: the hand on the skin).
 *   Knobs (SOUND): KIT (SALSA, PERREO, BOMBA, TIPICO: tuning, decays and skin for the hand drums, the kick, the snare),
 *   TUNE (-12 .. +12 semitones, all of it), DECAY (x 0.25 .. x 4, every decay), HUMAN (each hit a little later, softer,
 *   off-pitch, at random: 0 none .. 100 % a loose hand).
 *   Audio ISR only: perc_hit from note_on (coqui.c, in ctl_hook), perc_block from voices_render a control step. */

static inline int32_t sine_i(uint32_t ph)       /* sin, Q15, a full cycle over 2^32 (Felucca's dsp.c, SINE[1025]) */
{
    uint32_t i = ph >> 22;
    int32_t a = SINE[i], b = SINE[(i + 1u) & 1023u];
    return a + (((b - a) * (int32_t)((ph >> 7) & 0x7FFFu)) >> 15);
}
static inline uint32_t noise32(int32_t *st)     /* xorshift32 (Felucca's dsp.c) */
{
    uint32_t s = (uint32_t)*st;
    s ^= s << 13;
    s ^= s >> 17;
    s ^= s << 5;
    *st = (int32_t)s;
    return s;
}

enum { PI_KICK, PI_BOOM, PI_SNARE, PI_RIM, PI_CLAP, PI_HATC, PI_HATO, PI_MARAC, PI_CASC, PI_COWB, PI_CLAVE, PI_WOOD,
       PI_GUIRS, PI_GUIRL, PI_TUMBA, PI_CONGO, PI_CONGM, PI_CONGS, PI_BONGH, PI_BONGL, PI_TIMBH, PI_TIMBL, NPI };
enum { PS_NONE, PS_CLAP, PS_GUIRO, PS_SOFT };   /* a drum's own trick */
enum { PC_OTHER, PC_HAND, PC_KICK, PC_SNARE };  /* what KIT changes */
typedef struct {
    uint16_t p16;                               /* the tone's pitch, 1/16 semitones (MIDI x 16; 0: no tone) */
    uint16_t r2;                                /* the second partial's ratio, Q8 (0: none) */
    uint16_t l2;                                /* .. its level, Q15 */
    uint16_t pdep;                              /* the pitch drop: the start's ratio - 1, Q8 (0: none) */
    uint16_t pdec;                              /* .. its time (ms, 60 dB) */
    uint16_t tdec;                              /* the tone's decay (ms, 60 dB) */
    uint16_t nlvl;                              /* the noise's level, Q15 */
    uint16_t ndec;                              /* .. its decay (ms; the güiro: its length) */
    uint16_t nf;                                /* .. its filter: the state-variable filter's f, Q15 (2 sin(pi fc / fs)) */
    uint8_t nt;                                 /* .. 0 high-pass, 1 band-pass */
    uint8_t sq;                                 /* the tone square (the cowbell), softened */
    uint8_t sp;                                 /* PS_ */
    uint8_t cls;                                /* PC_ */
    uint16_t gain;                              /* its level in the kit, Q8 (tests/perc/perc_test.c: each drum's peak at
                                                 * full velocity, KIT SALSA, to its place in the mix) */
} pinst_t;
/* the drums (tools: computed once on a computer from Hz, ratios and semitones; see PLAN.md) */
static const pinst_t PINST[NPI] = {
    { 528,   0,     0,  768,  28, 300, 11468,   5, 11610, 1, 0, 0, PC_KICK, 133},    /* KICK: 55 Hz, a 2-octave drop, 2.5 kHz click */
    { 466,   0,     0,  319,  55, 900,  4915,   4,  9306, 1, 0, 0, PC_KICK, 131},    /* BOOM: 44 Hz, long (the 808's) */
    { 871, 415, 14745,   86,  18, 110, 27852, 170, 14811, 1, 0, 0, PC_SNARE,  68},   /* SNARE: 190 Hz, the wires at 3.2 kHz */
    {1470, 376, 16384,    0,   1,  22, 11468,   9, 18423, 0, 0, 0, PC_SNARE,  49},   /* RIM: 1.65 kHz, a crack */
    {   0,   0,     0,    0,   1,   1, 31129, 190,  6061, 1, 0, PS_CLAP, PC_SNARE, 445},   /* CLAP: three bursts at 1.3 kHz */
    {   0,   0,     0,    0,   1,   1, 24575,  45, 31343, 0, 0, 0, PC_OTHER,  50},   /* HATC: over 7 kHz, short */
    {   0,   0,     0,    0,   1,   1, 21299, 360, 29273, 0, 0, 0, PC_OTHER,  59},   /* HATO: over 6.5 kHz, long */
    {   0,   0,     0,    0,   1,   1, 22937,  70, 23726, 0, 0, PS_SOFT, PC_OTHER,  63},   /* MARAC: over 5.2 kHz, a soft start */
    {1358, 614,  9830,    0,   1,  32,  9830,   9, 13900, 1, 0, 0, PC_OTHER,  52},   /* CASC: the timbal's shell (cáscara) */
    {1172, 385, 29490,    0,   1, 190,  1638,   5, 13900, 1, 1, 0, PC_OTHER,  63},   /* COWB: 562 + 845 Hz squares */
    {1583,   0,     0,    0,   1,  48,  3932,   3, 18423, 1, 0, 0, PC_OTHER,  83},   /* CLAVE: 2.5 kHz, a ping */
    {1393, 691,  9830,    0,   1,  38,  4915,   5, 13900, 1, 0, 0, PC_OTHER,  61},   /* WOOD: 1.25 kHz block */
    {   0,   0,     0,    0,   1,   1, 26214,  95, 20651, 1, 0, PS_GUIRO, PC_OTHER,  64},   /* GUIRS: a short scrape */
    {   0,   0,     0,    0,   1,   1, 26214, 330, 19317, 1, 0, PS_GUIRO, PC_OTHER,  57},   /* GUIRL: a long scrape */
    { 880, 384, 11468,   31,  16, 280,  8192,  16,  6990, 1, 0, 0, PC_HAND,  90},    /* TUMBA: 196 Hz */
    { 960, 384, 11468,   31,  14, 230,  8192,  14,  7917, 1, 0, 0, PC_HAND,  81},    /* CONGO: 262 Hz, open */
    { 960, 384,  6553,   15,   8,  42,  9830,  10,  8381, 1, 0, 0, PC_HAND,  64},    /* CONGM: muted */
    {1024, 563, 16384,   31,   8,  55, 29490,  45, 11610, 1, 0, 0, PC_HAND,  68},    /* CONGS: the slap */
    {1190, 384,  9830,   31,  10, 110,  9830,  12, 16171, 1, 0, 0, PC_HAND,  62},    /* BONGH: macho, 600 Hz */
    {1098, 384,  9830,   31,  10, 140,  9830,  12, 14811, 1, 0, 0, PC_HAND,  72},    /* BONGL: hembra, 430 Hz */
    {1150, 591, 16384,   15,  10, 380, 11468,  30, 13900, 1, 0, 0, PC_HAND,  55},    /* TIMBH: 520 Hz, ringing */
    {1071, 591, 16384,   15,  10, 420, 11468,  30, 12986, 1, 0, 0, PC_HAND,  62},    /* TIMBL: 390 Hz */
};
/* the keys, F3 .. G5 (MIDI 53 .. 79): the drum, its pitch (semitones from the drum's own), its choke group (a hit cuts
 * the one sounding in its group, on its track). White keys from F3: KICK SNARE CLAP CASC COWB CLAVE GUIRO | TUMBA CONGA
 * QUINTO BONGO-L BONGO-H TIMBAL-L TIMBAL-H BOOM TAMBORA; black: HAT-C HAT-O MARACAS BELL WOOD | SLAP MUTE GUIRO-L SLAP
 * PAILA RIM */
#define PK_LO 53u
#define PK_N 27u
static const struct { uint8_t i; int8_t st; uint8_t g; } PKEY[PK_N] = {
    {PI_KICK, 0, 1},   {PI_HATC, 0, 2},   {PI_SNARE, 0, 3},  {PI_HATO, 0, 2},  {PI_CLAP, 0, 4},   {PI_MARAC, 0, 5},
    {PI_CASC, 0, 6},   {PI_COWB, 0, 7},   {PI_COWB, 7, 8},   {PI_CLAVE, 0, 9}, {PI_WOOD, 0, 10},  {PI_GUIRS, 0, 11},
    {PI_TUMBA, 0, 12}, {PI_CONGS, 0, 13}, {PI_CONGO, 0, 13}, {PI_CONGM, 0, 13}, {PI_CONGO, 5, 14}, {PI_GUIRL, 0, 11},
    {PI_BONGL, 0, 15}, {PI_BONGH, 0, 16}, {PI_CONGS, 7, 16}, {PI_TIMBL, 0, 17}, {PI_CASC, 5, 18}, {PI_TIMBH, 0, 19},
    {PI_BOOM, 0, 1},   {PI_RIM, 0, 3},    {PI_TIMBL, -7, 20}};
static const char *const PKEY_NAME[PK_N] = {"KICK", "HAT", "SNARE", "OPEN HAT", "CLAP", "MARACAS", "CASCARA", "COWBELL",
                                            "BELL", "CLAVE", "WOOD", "GUIRO", "TUMBA", "SLAP", "CONGA", "MUTE", "QUINTO",
                                            "GUIRO LONG", "BONGO LO", "BONGO HI", "BONGO SLAP", "TIMBAL LO", "PAILA",
                                            "TIMBAL HI", "BOOM", "RIM", "TAMBORA"};
static uint32_t perc_key(uint32_t note)         /* a note's key: 0 .. PK_N - 1 */
{
    int32_t n = (int32_t)note;
    while (n < (int32_t)PK_LO) n += 24;
    while (n >= (int32_t)(PK_LO + PK_N)) n -= 24;
    return (uint32_t)n - PK_LO;
}

/* the kits: the hand drums' tuning (1/16 semitones) and decay (Q8), the kicks', the snares' decay, the hand drums'
 * skin (their noise, Q8) */
#define NKIT 4u
static const char *const KIT_NAME[NKIT] = {"SALSA", "PERREO", "BOMBA", "TIPICO"};
static const struct { int16_t hp, kp; uint16_t hd, kd, sd, hn; } PKIT[NKIT] = {
    {0, 0, 256, 256, 256, 256},                 /* SALSA: as the table */
    {16, -32, 205, 640, 170, 300},              /* PERREO: a long low kick, a tight snare, drier brighter congas */
    {-48, -16, 384, 320, 256, 200},             /* BOMBA: the barriles lower and longer, less slap */
    {24, 0, 230, 256, 220, 280},                /* TIPICO: tighter, higher (merengue, cumbia) */
};

#define NPH 10u                                 /* hits sounding at once (all PERC tracks together) */
#define PH_CLAP_GAP 14u                         /* the clap's bursts: a control step apart this many (~10 ms) */
typedef struct {
    uint8_t on, inst, grp, trk, kill, sp, soft; /* sounding; the drum, its group, its track; out this step; its trick */
    uint16_t wait;                              /* frames still to wait (HUMAN) */
    uint16_t cnt;                               /* control steps since it began (the clap, the güiro, the maracas) */
    uint16_t gate;                              /* the güiro: its length in control steps */
    uint32_t ph1, ph2, inc1;                    /* the tone's phases; its pitch's increment */
    uint32_t r2, pdep;                          /* the second partial's ratio (Q8), the drop (Q8) */
    int32_t l2;                                 /* the second partial's level (Q15) */
    int32_t ta, na, pe;                         /* the tone's, the noise's amplitude (Q24), the pitch drop left (Q16) */
    uint32_t kt, kn, kp;                        /* their decays (env_k: a control step's share) */
    int32_t npk;                                /* the noise's peak (Q24: the clap's bursts, the maracas' attack) */
    int32_t lp, bp, sl;                         /* the noise's filter, the square's smoothing */
    int32_t rs;                                 /* the noise's state */
    uint32_t rph, rinc;                         /* the güiro's ratchet */
    int32_t pulse;                              /* .. its tooth now (Q15) */
    int32_t vel;                                /* its volume, Q14 */
} phit_t;
static phit_t ph_[NPH];
static uint32_t ph_seed = 0x9E3779B9u;          /* HUMAN's dice */
static volatile uint8_t perc_lit[NT];           /* the last key struck on each track + 1 (the SOUND page's picture) */

static uint32_t perc_rnd(void) { return noise32((int32_t *)&ph_seed); }
/* a decay of ms x dmul (Q16) as env_k's share a control step (60 dB in that time: 6.9 / (t / 0.726 ms), Q16), in
 * 32-bit divisions only (the chip has no 64-bit one: env_k's, inlined here, asked for __udivdi3) */
static uint32_t ph_k(uint32_t ms, uint32_t dmul)
{
    uint32_t t = (ms * (dmul >> 6)) >> 4;       /* (ms x dmul, Q6: up to 900 x 4 x 64) */
    uint32_t k = t ? 21025264u / t : 52429u;    /* (328516 x 65536 / 2^10 over the time / 2^10: Q16) */
    return k > 52429u ? 52429u : k ? k : 1u;
}

/* audio: track t's note struck, at vel (Q8, 0: full) */
static void perc_hit(uint32_t t, uint32_t note, uint32_t vel)
{
    const volatile int32_t *p = trk[t].p[ENG_PERC];
    uint32_t k = perc_key(note), i, best = 0, kit = (uint32_t)clamp(p[0], 0, NKIT - 1);
    uint32_t human = (uint32_t)clamp(p[3], 0, 127), dmul = pow2_q16(clamp(p[2], 0, 127) * 6) >> 2;
    const pinst_t *I = &PINST[PKEY[k].i];
    int32_t lv = 0x7FFFFFFF, p16;
    phit_t *h;
    for (i = 0; i < NPH; i++)                   /* its group on its track: cut (it fades out this step) */
        if (ph_[i].on && ph_[i].trk == t && ph_[i].grp == PKEY[k].g) ph_[i].kill = 1;
    for (i = 0; i < NPH && ph_[i].on; i++)
        ;
    if (i == NPH)                               /* none free: the quietest goes at once */
        for (i = 0, best = 0; i < NPH; i++)
            if (ph_[i].ta + ph_[i].na < lv) lv = ph_[i].ta + ph_[i].na, best = i;
    h = &ph_[i < NPH ? i : best];
    h->inst = PKEY[k].i, h->grp = PKEY[k].g, h->trk = (uint8_t)t, h->kill = 0, h->sp = I->sp, h->cnt = 0;
    h->vel = (int32_t)(((vel ? vel * 64u : 16384u) * I->gain) >> 8);   /* (Q14; the clap's gain past 1: at most ~1.7) */
    h->wait = 0;
    p16 = I->p16 + PKEY[k].st * 16 + (clamp(p[1], 0, 127) - 64) * 3;
    if (I->cls == PC_HAND) p16 += PKIT[kit].hp;
    if (I->cls == PC_KICK) p16 += PKIT[kit].kp;
    if (human) {                                /* HUMAN: later (up to ~8.6 ms), softer (up to -35 %), off-pitch (+-0.4) */
        uint32_t r = perc_rnd();
        h->wait = (uint16_t)((r & 0xFFFFu) % (human * 3u + 1u));
        h->vel -= (int32_t)(((uint32_t)h->vel * human * ((r >> 16) & 255u)) >> 16) * 45 / 64;
        p16 += (int32_t)(((r >> 24) & 15u) - 7u) * (int32_t)human / 127;
    }
    h->inc1 = I->p16 ? pitch_inc((uint32_t)clamp(p16, 0, 2047)) : 0u;
    h->r2 = I->r2, h->l2 = I->l2, h->pdep = I->pdep;
    h->ph1 = h->ph2 = 0;
    {
        uint32_t tm = dmul, nm = dmul;
        if (I->cls == PC_HAND) tm = (tm >> 8) * PKIT[kit].hd;
        if (I->cls == PC_KICK) tm = (tm >> 8) * PKIT[kit].kd;
        if (I->cls == PC_SNARE) tm = (tm >> 8) * PKIT[kit].sd, nm = (nm >> 8) * PKIT[kit].sd;
        h->kt = ph_k(I->tdec, tm);
        h->kn = ph_k(I->ndec, nm);
        h->kp = ph_k(I->pdec, 65536u);
        h->gate = (uint16_t)((I->ndec * (nm >> 8) / 256u) * 44u / 32u);   /* (the güiro: its length, control steps) */
    }
    h->ta = I->p16 ? 1 << 24 : 0;
    h->npk = (int32_t)(((int64_t)I->nlvl << 9) * (I->cls == PC_HAND ? PKIT[kit].hn : 256u) >> 8);   /* (Q15 -> Q24) */
    h->na = I->sp == PS_SOFT ? 0 : h->npk;
    h->pe = 65536;
    h->lp = h->bp = h->sl = 0;
    h->rs = (int32_t)(perc_rnd() | 1u);
    h->rph = 0, h->rinc = I->ndec > 200u ? 3311313u : 4382620u, h->pulse = 0;   /* (the güiro: 34 or 45 teeth a second) */
    h->on = 1;
    perc_lit[t] = (uint8_t)(k + 1u);
}

/* audio: every hit's next control step (CTL frames from b) into tb[its track]; used: the tracks heard (their buffers
 * zeroed at their first sound, as voices_render does) */
static void perc_block(int32_t (*tb)[VB], uint32_t b, uint32_t n, uint32_t *used)
{
    uint32_t k, i;
    for (k = 0; k < NPH; k++) {
        phit_t *h = &ph_[k];
        const pinst_t *I;
        int32_t ta0, ta1, na0, na1, dta, dna, *o;
        uint32_t i0 = 0, inc, inc2;
        if (!h->on) continue;
        if (h->wait >= CTL) { h->wait = (uint16_t)(h->wait - CTL); continue; }
        i0 = h->wait, h->wait = 0;
        I = &PINST[h->inst];
        ta0 = h->ta, na0 = h->na;
        if (h->kill) ta1 = na1 = 0;             /* (cut: out this step) */
        else {
            ta1 = ta0 - mulq16(ta0, h->kt);
            if (h->sp == PS_CLAP) {             /* three bursts, then its tail */
                if (h->cnt == PH_CLAP_GAP || h->cnt == 2u * PH_CLAP_GAP) na0 = h->npk * 7 / 8;
                na1 = na0 - mulq16(na0, h->cnt < 2u * PH_CLAP_GAP ? 7000u : h->kn);
            } else if (h->sp == PS_GUIRO)       /* held for its length, then out in ~15 ms */
                na1 = h->cnt < h->gate ? na0 : na0 - mulq16(na0, 3000u);
            else if (h->sp == PS_SOFT && h->cnt < 6u)   /* a soft start: up in ~4 ms */
                na1 = h->npk / 6 * (int32_t)(h->cnt + 1u);
            else
                na1 = na0 - mulq16(na0, h->kn);
        }
        h->cnt++;
        h->pe -= mulq16(h->pe, h->kp);
        inc = h->inc1 + (h->pdep ? (uint32_t)(((uint64_t)h->inc1 * h->pdep * (uint32_t)(h->pe >> 8)) >> 16) : 0u);
        inc2 = h->r2 ? (inc >> 8) * h->r2 : 0u;
        if (!(*used >> h->trk & 1u)) {
            *used |= 1u << h->trk;
            for (i = 0; i < n; i++) tb[h->trk][i] = 0;
        }
        o = tb[h->trk] + b;
        dta = (ta1 - ta0) >> CTL_LOG2, dna = (na1 - na0) >> CTL_LOG2;
        ta0 += dta * (int32_t)i0, na0 += dna * (int32_t)i0;
        for (i = i0; i < CTL; i++, ta0 += dta, na0 += dna) {
            int32_t s = 0, x;
            if (h->inc1) {                      /* the tone */
                if (I->sq) {
                    x = (h->ph1 & 0x80000000u ? -20000 : 20000) + (h->ph2 & 0x80000000u ? -h->l2 : h->l2) * 20000 / 32768;
                    h->sl += (x - h->sl) >> 2;  /* (softened: ~1.8 kHz) */
                    x = h->sl;
                } else {
                    x = SINE[h->ph1 >> 22];
                    if (h->r2) x += (SINE[h->ph2 >> 22] * h->l2) >> 15;
                }
                s = (x * (ta0 >> 10)) >> 14;
                h->ph1 += inc, h->ph2 += inc2;
            }
            if (na0 > 0) {                      /* the noise, filtered */
                int32_t w = (int32_t)noise32(&h->rs) >> 17, hp;
                h->lp += (I->nf * h->bp) >> 15;
                hp = w - h->lp - h->bp;
                h->bp += (I->nf * hp) >> 15;
                x = I->nt ? h->bp : hp;
                if (h->sp == PS_GUIRO) {        /* a tooth: a click decaying ~3 ms */
                    uint32_t r = h->rph;
                    h->rph += h->rinc;
                    if (h->rph < r) h->pulse = 32767;
                    h->pulse -= h->pulse >> 7;
                    x = (x * h->pulse) >> 14;
                }
                s += (x * (na0 >> 10)) >> 14;
            }
            s = clamp(s, -70000, 70000);   /* (x vel, at most ~30000: under 2^31) */
            o[i] += (s * h->vel) >> (14 - PB);
        }
        h->ta = ta1, h->na = na1;
        if (h->kill || (ta1 < 256 && na1 < 256 && !(h->sp == PS_CLAP && h->cnt <= 2u * PH_CLAP_GAP) &&
                        !(h->sp == PS_GUIRO && h->cnt < h->gate) && !(h->sp == PS_SOFT && h->cnt < 6u)))
            h->on = 0;
    }
}
static int perc_busy(void)                      /* a hit sounds */
{
    uint32_t k;
    for (k = 0; k < NPH; k++)
        if (ph_[k].on) return 1;
    return 0;
}

/* the engine as the tracks see it: no voices (note_on hands its notes to perc_hit); its knobs' words */
static void perc_begin(voice_t *v) { (void)v; }
static void perc_vblock(voice_t *v, const volatile int32_t *p, int32_t *mono, int32_t amp0, int32_t amp1)
{
    (void)v, (void)p, (void)mono, (void)amp0, (void)amp1;
}
static void perc_text(const volatile int32_t *p, uint32_t k, char *s)
{
    int32_t v = p[k];
    switch (k) {
    case 0: put_s(s, KIT_NAME[clamp(v, 0, NKIT - 1)]); return;
    case 1: {                                   /* TUNE: semitones */
        int32_t st = (clamp(v, 0, 127) - 64) * 3;
        st = st < 0 ? -((-st + 8) / 16) : (st + 8) / 16;
        if (st < 0) put_u(put_s(s, "-"), (uint32_t)-st);
        else put_u(put_s(s, st ? "+" : ""), (uint32_t)st);
        return;
    }
    case 2: put_s(put_u(s, (pow2_q16(clamp(v, 0, 127) * 6) >> 2) * 100u >> 16), "%"); return;   /* DECAY: x 25 .. 400 % */
    default: put_s(put_u(s, (uint32_t)clamp(v, 0, 127) * 100u / 127u), "%"); return;   /* HUMAN */
    }
}
static const engine_t PERC_ENGINE = {"PERC", {"KIT", "TUNE", "DECAY", "HUMAN"}, {NKIT - 1, 127, 127, 127},
                                     perc_begin, perc_vblock, perc_text};
