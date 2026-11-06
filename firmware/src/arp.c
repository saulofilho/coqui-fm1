/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 GR!MM */
/* GR!MM's arpeggiator: a track's ARP (ARP held: on / off; MODE, RATE, OCTAVES, GATE: the track's, saved with the project
 * and the slots, not with sounds).
 *   ARP on: the track's keys and MIDI notes (after SCALE; chord mode on: each its whole chord) go into its list instead
 *   of sounding; the arp plays them a step at a time in MODE's order (UP, DOWN, UP-DOWN, RANDOM, AS PLAYED; CHORD: all
 *   of them each step), OCTAVES times an octave higher, each note GATE of its step long
 *   LATCH OFF: a key's notes leave the list when it is let go; LATCH ON: they stay, the key again takes them out; an
 *   empty list stops it (its note released), as does ARP off
 *   the clock: the sequencer's when it plays (its steps on the sequencer's grid), else the tempo's, from the first note
 *   held (that note at once)
 *   REC on and the sequencer playing: the selected track's arp notes are live-recorded into its sequence
 * Its notes: coqui.c note_on / note_off as SRC_ARP (their origin: the note), the track's source and ENV, no chord of
 * their own. Audio ISR; the main loop reads the list (the screen, the keys' lights).
 * (COQUÍ) The Latin modes, TUMBAO .. BACHATA: a two-bar line in sixteenths (ARP_LAT) over the chord held, its tones low
 * to high: R the root, 3 the next, 5 the next (a tone missing: a major third, a fifth over the root), O the root an
 * octave up, L an octave down; '-' ties the note on, '.' rests. RATE is the line's own; OCTAVES the register (1 an octave
 * under what is held: a bass; 2 as held; 3, 4 higher); GATE a share of each note's length; MONTUNO plays octaves. A new
 * chord takes over at the next note, the rhythm going on; on the sequencer's bars while it plays. */

enum { ARP_UP, ARP_DOWN, ARP_UPDN, ARP_RND, ARP_PLAYED, ARP_CHORD, ARP_TUMBAO, ARP_MONTUNO, ARP_GUAJEO, ARP_MERENGUE,
       ARP_BACHATA, NARPMODE };   /* (COQUÍ: TUMBAO .. BACHATA, the Latin lines) */
#define NARPRATE 9
#define ARP_N 16u                               /* notes in a track's list at most */
#define ARP_PN (2u * ARP_N * 4u)                /* a pattern's steps at most (UP-DOWN over 4 octaves) */
static const uint16_t ARP_RATE_T[NARPRATE] = {384, 192, 96, 48, 24, 12, 64, 32, 16};   /* RATE: ticks a step */
static const char *const ARP_RATE_NAME[NARPRATE] = {"1/1", "1/2", "1/4", "1/8", "1/16", "1/32", "1/4T", "1/8T", "1/16T"};
static const char *const ARP_MODE_NAME[NARPMODE] = {"UP", "DOWN", "UP-DOWN", "RANDOM", "AS PLAYED", "CHORD", "TUMBAO",
                                                    "MONTUNO", "GUAJEO", "MERENGUE", "BACHATA"};
static const char *const ARP_MODE_SHORT[NARPMODE] = {"UP", "DOWN", "UPDN", "RAND", "PLAYED", "CHORD", "TUMBAO", "MONTNO",
                                                     "GUAJEO", "MRENGE", "BACHTA"};   /* (slabs) */
#define ARP_NLAT (NARPMODE - ARP_TUMBAO)
static const char *const ARP_LAT[ARP_NLAT] = {   /* |1 . . . 2 . . . 3 . . . 4 . . . |1 . . . 2 . . . 3 . . . 4 . . . | */
    "------5-----R---------5-----R---",   /* TUMBAO: the fifth on 2-and, the root anticipating on 4 */
    "..5.3.5.R.5.3.5...5.3.5.O.5.3.O.",   /* MONTUNO: off-beat eighths in octaves, 2-3 */
    "..R.3.5.O.5.3.R...R.3.5.O.5.3.5.",   /* GUAJEO: the tres's line */
    "R...5...R...5...R...5...R.5.R.5.",   /* MERENGUE: the bass on 1 and 3, a push into the next */
    "R-----R-5-------R-----R-5-------",   /* BACHATA: the bass, long, its 2-and */
};

typedef struct {
    uint8_t note[ARP_N], id[ARP_N], root[ARP_N];   /* the list: the notes, where they came from (lat_id), the note
                                                 * played (before its chord) */
    volatile uint32_t n;                        /* .. how many */
    uint32_t on[4];                             /* the notes it has on now (bit: the note) */
    uint32_t ft, since;                         /* its own clock (ticks since its first note); ticks since its step */
    volatile uint32_t pos;                      /* the step to come */
    volatile int32_t last;                      /* the step played last (the screen; -1: none) */
    uint32_t was;                               /* ARP on, as last seen */
    uint32_t glen;                              /* (COQUÍ: a Latin line's note: its ticks, GATE's share; 0: none) */
} arp_t;
static arp_t arp[NT];
static volatile int32_t arp_rec = -1;           /* main loop: the track live-recording its arp (REC, playing), or -1 */
static uint32_t arp_acc;                        /* audio: the clock's samples (Q16) while the sequencer stands */

/* track t's steps into p (at most ARP_PN), MODE's order over OCTAVES (oct); CHORD: the list's notes, low to high, once */
static uint32_t arp_pattern(uint32_t t, uint8_t *p, uint32_t oct)
{
    const arp_t *A = &arp[t];
    uint8_t b[ARP_N];
    uint32_t n = 0, i, j, k = 0, o, cnt = A->n < ARP_N ? A->n : ARP_N, mode = (uint32_t)clamp(trk[t].arp_mode, 0, NARPMODE - 1);
    for (i = 0; i < cnt; i++) {                 /* each note once (AS PLAYED: in the order they came) */
        for (j = 0; j < n && b[j] != A->note[i]; j++)
            ;
        if (j == n) b[n++] = A->note[i];
    }
    if (mode != ARP_PLAYED)                     /* low to high */
        for (i = 1; i < n; i++)
            for (j = i; j > 0 && b[j - 1] > b[j]; j--) {
                uint8_t x = b[j];
                b[j] = b[j - 1], b[j - 1] = x;
            }
    if (mode == ARP_CHORD) oct = 1;
    for (o = 0; o < oct; o++)
        for (i = 0; i < n; i++)
            if (b[i] + 12u * o <= 127u) p[k++] = (uint8_t)(b[i] + 12u * o);
    if (mode == ARP_DOWN)
        for (i = 0; i < k / 2u; i++) {
            uint8_t x = p[i];
            p[i] = p[k - 1u - i], p[k - 1u - i] = x;
        }
    else if (mode == ARP_UPDN && k > 2u) {      /* up, then down (the ends once) */
        for (i = k - 1u, j = k; i-- > 1u;)
            p[j++] = p[i];
        k = j;
    }
    return k;
}

static void arp_release(uint32_t t)             /* audio: its notes on, off */
{
    uint32_t w, b;
    for (w = 0; w < 4u; w++)
        for (b = 0; arp[t].on[w] && b < 32u; b++)
            if (arp[t].on[w] >> b & 1u) {
                note_off(t, w * 32u + b, SRC_ARP, 0);
                arp[t].on[w] &= ~(1u << b);
            }
}
static void arp_clear(uint32_t t)               /* audio: the list emptied (its keys' echoes off), its note off */
{
    uint32_t i, j;
    for (i = 0; i < arp[t].n && i < ARP_N; i++) {
        for (j = 0; j < i && arp[t].id[j] != arp[t].id[i]; j++)
            ;
        if (j == i && arp[t].id[i] < 27u) ec_put(0x80, t, arp[t].root[i]);   /* (a key's: echoed on as it came in) */
    }
    arp_release(t);
    arp[t].n = 0, arp[t].pos = 0, arp[t].last = -1;
}
static void arp_play(uint32_t t, uint32_t note)
{
    if (note > 127u) return;
    note_on(t, note, note, SRC_ARP, 0, 0);      /* (no chord of its own) */
    arp[t].on[note >> 5] |= 1u << (note & 31u);
}
static void arp_step(uint32_t t)                /* audio: the step due: its note (CHORD: its notes) on */
{
    arp_t *A = &arp[t];
    uint8_t p[ARP_PN];
    uint32_t oct = (uint32_t)clamp(trk[t].arp_oct, 1, 4), len = arp_pattern(t, p, oct), i, first;
    arp_release(t);
    if (!len) return;
    if (clamp(trk[t].arp_mode, 0, NARPMODE - 1) == ARP_CHORD) {   /* all of them, an octave up a step */
        uint32_t o = A->pos % oct;
        for (i = 0; i < len; i++) arp_play(t, p[i] + 12u * o);
        first = p[0] + 12u * o, A->last = (int32_t)o, A->pos = o + 1u;
    } else {
        i = clamp(trk[t].arp_mode, 0, NARPMODE - 1) == ARP_RND ? seq_rnd() % len : A->pos % len;
        arp_play(t, p[i]);
        first = p[i], A->last = (int32_t)i, A->pos = i + 1u;
    }
    if (arp_rec == (int32_t)t && seq_on && first <= 127u) {   /* REC: into the sequence, as a key's live note */
        int32_t s = seq_nearest(t);
        if (s >= 0) seq_write(t, s, first, 0u);   /* (its notes: notes, not chord steps) */
    }
}
/* (COQUÍ) a Latin line's tone for degree c over the chord held (b: its notes low to high, n of them), or -1 */
static int32_t arp_deg(char c, const uint8_t *b, uint32_t n)
{
    int32_t r = b[0];
    switch (c) {
    case 'R': return r;
    case '3': return n > 1u ? b[1] : r + 4;
    case '5': return n > 2u ? b[2] : n > 1u ? b[1] + 3 : r + 7;
    case 'O': return r + 12;
    case 'L': return r - 12;
    default: return -1;
    }
}
static void arp_lat_tick(uint32_t t, uint32_t tk)   /* audio: a tick (tk: the line's clock) of a Latin mode */
{
    arp_t *A = &arp[t];
    const char *L = ARP_LAT[clamp(trk[t].arp_mode, ARP_TUMBAO, NARPMODE - 1) - ARP_TUMBAO];
    uint8_t b[ARP_PN];
    uint32_t s, n, len;
    int32_t v, sh = (clamp(trk[t].arp_oct, 1, 4) - 2) * 12;
    if (A->glen && ++A->since >= A->glen) arp_release(t), A->glen = 0;   /* (GATE: its end) */
    if (tk % 24u) return;                       /* (a sixteenth) */
    s = (tk / 24u) % 32u;
    if (L[s] == '-') return;                    /* (tied on) */
    arp_release(t), A->glen = 0, A->last = (int32_t)s;
    if (L[s] == '.') return;
    n = arp_pattern(t, b, 1u);                  /* (the chord: low to high, each once) */
    if (!n || (v = arp_deg(L[s], b, n)) < 0) return;
    v += sh;
    if (v >= 0 && v <= 127) arp_play(t, (uint32_t)v);
    if (clamp(trk[t].arp_mode, 0, NARPMODE - 1) == ARP_MONTUNO && v + 12 >= 0 && v + 12 <= 127) arp_play(t, (uint32_t)(v + 12));
    for (len = 1u; len < 32u && L[(s + len) % 32u] == '-'; len++)
        ;
    A->glen = len * 24u * (uint32_t)clamp(trk[t].arp_gate, 10, 100) / 100u, A->since = 0;
    if (A->glen < 2u) A->glen = 2u;
}
static void arp_tick(uint32_t t, int locked)    /* audio: a tick of track t's arp (locked: the sequencer's tick) */
{
    arp_t *A = &arp[t];
    uint32_t r, g, due;
    gen_tick(t);                                /* (V2: GENERATE, on the same clock: gen.c) */
    if (!A->n) return;
    if (clamp(trk[t].arp_mode, 0, NARPMODE - 1) >= ARP_TUMBAO) {   /* (COQUÍ: a Latin line, on the bars) */
        arp_lat_tick(t, locked ? seq_ticks - 1u : A->ft);
        A->ft++;
        return;
    }
    r = ARP_RATE_T[clamp(trk[t].arp_rate, 0, NARPRATE - 1)];
    g = r * (uint32_t)clamp(trk[t].arp_gate, 10, 100) / 100u;
    if (++A->since >= (g ? g : 1u)) arp_release(t);   /* GATE: the note's end */
    due = locked ? (seq_ticks - 1u) % r == 0u : A->ft % r == 0u;
    A->ft++;
    if (due) {
        A->since = 0;
        arp_step(t);
    }
}

/* audio: a key or a MIDI note (origin id, note after SCALE) of a track with ARP on: its notes (its chord) into the
 * list (in it already: anew), out of it */
static int arp_has(uint32_t t, uint32_t id)
{
    uint32_t i;
    for (i = 0; i < arp[t].n; i++)
        if (arp[t].id[i] == id) return 1;
    return 0;
}
static int32_t arp_root(uint32_t t, uint32_t note)   /* (V2) the origin of the list's notes played as note (a key's
                                                 * or MIDI's), -1 none */
{
    uint32_t i;
    for (i = 0; i < arp[t].n; i++)
        if (arp[t].root[i] == note) return arp[t].id[i];
    return -1;
}
static void arp_take(uint32_t t, uint32_t id, int echo)
{
    arp_t *A = &arp[t];
    uint32_t i, k = 0, root = 255u;
    for (i = 0; i < A->n; i++)
        if (A->id[i] != id) {
            A->note[k] = A->note[i], A->id[k] = A->id[i], A->root[k] = A->root[i];
            k++;
        } else
            root = A->root[i];
    if (root <= 127u && echo) ec_put(0x80, t, root);
    A->n = k;
    if (!k) arp_clear(t);                       /* (none left: it stops) */
}
static void arp_add(uint32_t t, uint32_t id, uint32_t note, int echo)
{
    arp_t *A = &arp[t];
    uint8_t ns[CH_MAX];
    uint32_t n = 1, i;
    if (note > 127u) return;
    if (arp_has(t, id)) arp_take(t, id, echo);
    ns[0] = (uint8_t)note;
    if (trk[t].chord) n = chord_notes(t, note, ns);
    if (!A->n) A->ft = 0, A->since = 0, A->pos = 0;   /* the first: it starts (its own clock: at once) */
    for (i = 0; i < n && A->n < ARP_N; i++) {
        A->note[A->n] = ns[i], A->id[A->n] = (uint8_t)id, A->root[A->n] = (uint8_t)note;
        A->n++;
    }
    if (echo) ec_put(0x90, t, note);
}
static void arp_watch(void)                     /* audio: ARP switched on or off: the list starts empty (on: the track's
                                                 * key notes released first) */
{
    uint32_t t, on;
    for (t = 0; t < NT; t++)
        if ((on = trk[t].arp != 0) != arp[t].was) {
            arp[t].was = on;
            arp_clear(t);
            if (on) release_keys(t);
        }
}
static void arp_ctl(void)                       /* audio, a control block: the clock's ticks while the sequencer stands */
{
    uint32_t tick = SEQ_TICK_Q16 / (uint32_t)clamp(seq_bpm, 20, 300), t;
    if (seq_on) { arp_acc = 0; return; }        /* (playing: seq_ctl's ticks) */
    arp_acc += (uint32_t)CTL << 16;
    while (arp_acc >= tick) {
        arp_acc -= tick;
        for (t = 0; t < NT; t++)
            arp_tick(t, 0);
    }
}
