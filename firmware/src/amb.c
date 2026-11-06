/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 COQUÍ */
/* COQUÍ's AMBIENTE: a scene for the effects there are (no new DSP, no pool: SPACE, DELAY, TAPE, MASTER, DRIVE), set at
 * once with one knob (FX's last page, after DRIVE), the tracks' sends with it where the scene needs them (a dub needs its echoes
 * fed). Each value can be turned on the effects' own pages afterwards; the scene is a starting point, not a lock.
 *   SECO      dry: the returns and the sends down, TAPE, MASTER and DRIVE off
 *   QUINTAL   a backyard: a short warm room
 *   CALLE     the street: a slapback echo off the walls, a medium room
 *   PLAYA     the beach: an open bright space, a soft echo
 *   NOCHE     the night: a long dark reverb, a slow echo
 *   DUB       a dub: a long dark echo feeding back, a low-pass over it all
 *   RADIO AM  an old radio: a band-pass, a clipping drive, the tape's wobble and hiss on every track
 * Main loop only (amb_apply from knob_turn); the values are the effects' knobs, 0..127, as their pages show them. */

enum { AMB_HAND, AMB_SECO, AMB_QUINTAL, AMB_CALLE, AMB_PLAYA, AMB_NOCHE, AMB_DUB, AMB_RADIO, NAMB };
static const char *const AMB_NAME[NAMB] = {"-", "SECO", "QUINTAL", "CALLE", "PLAYA", "NOCHE", "DUB", "RADIO AM"};
static const char *const AMB_WHAT[NAMB][2] = {
    {"THE EFFECTS AS YOU SET THEM", "TURN TO PICK A SCENE"},
    {"DRY: NO ROOM, NO ECHO", "TAPE, MASTER, DRIVE OFF"},
    {"A BACKYARD: A SHORT", "WARM ROOM"},
    {"THE STREET: A SLAPBACK", "OFF THE WALLS, A ROOM"},
    {"THE BEACH: AN OPEN BRIGHT", "SPACE, A SOFT ECHO"},
    {"THE NIGHT: A LONG DARK", "REVERB, A SLOW ECHO"},
    {"DUB: A LONG DARK ECHO", "FEEDING BACK, LOW-PASS"},
    {"AN OLD RADIO: BAND-PASS,", "DRIVE, TAPE ON EVERY TRACK"},
};
static volatile int32_t amb_scene;              /* AMBIENTE's knob: the scene last set (AMB_HAND at power-on) */
/* a scene: SPACE (SIZE, DAMP, MIX, PRE), DELAY (TIME, FEEDBACK, WEAR, MIX), TAPE (WOW, SATURATION, TONE, HISS), MASTER
 * (CUTOFF, RESONANCE, TYPE, POSITION), DRIVE (AMOUNT, TYPE, TONE, MIX), then the tracks' REV, DLY and TAPE sends (-1:
 * each track's own, left alone) */
static const int8_t AMB_SET[NAMB][23] = {
    {0},
    {80, 48, 0, 16, 88, 64, 50, 0, 0, 0, 0, 0, 127, 0, FT_LP, MP_END, 0, DT_SOFT, 127, 127, 0, 0, 0},
    {25, 70, 90, 6, 88, 64, 50, 0, 0, 0, 0, 0, 127, 0, FT_LP, MP_END, 0, DT_SOFT, 127, 127, 60, -1, 0},
    {48, 50, 80, 25, 42, 22, 35, 90, 0, 0, 0, 0, 127, 0, FT_LP, MP_END, 0, DT_SOFT, 127, 127, 55, 45, 0},
    {58, 28, 90, 10, 62, 30, 20, 60, 0, 0, 0, 0, 127, 0, FT_LP, MP_END, 0, DT_SOFT, 127, 127, 60, 30, 0},
    {84, 66, 100, 45, 79, 45, 40, 70, 0, 0, 0, 0, 127, 0, FT_LP, MP_END, 0, DT_SOFT, 127, 127, 90, 25, 0},
    {70, 40, 90, 20, 72, 96, 85, 110, 0, 0, 0, 0, 100, 20, FT_LP, MP_END, 0, DT_SOFT, 127, 127, 40, 70, 0},
    {30, 64, 30, 6, 88, 64, 50, 0, 25, 70, 30, 55, 75, 45, FT_BP, MP_END, 45, DT_CLIP, 70, 127, 30, 0, 127},
};
static void amb_apply(uint32_t sc)              /* main loop: scene sc onto the effects (and the tracks' sends) */
{
    const int8_t *v = AMB_SET[sc < NAMB ? sc : 0u];
    uint32_t t;
    if (sc == AMB_HAND || sc >= NAMB) return;
    rv_size = v[0], rv_damp = v[1], rv_mix = v[2], rv_pre = v[3];
    dl_time = v[4], dl_fb = v[5], dl_wear = v[6], dl_mix = v[7];
    tf_wow = v[8], tf_sat = v[9], tf_tone = v[10], tf_hiss = v[11];
    mf_cut = v[12], mf_res = v[13], mf_type = v[14], mf_pos = v[15];
    dr_amt = v[16], dr_type = v[17], dr_tone = v[18], dr_mix = v[19];
    for (t = 0; t < NT; t++) {
        if (v[20] >= 0) trk[t].rev = v[20];
        if (v[21] >= 0) trk[t].dly = v[21];
        if (v[22] >= 0) trk[t].tap = v[22];
    }
}
