#ifndef Z_BG_SST_FLOOR_H
#define Z_BG_SST_FLOOR_H

#include <libultraship/libultra.h>
#include "global.h"

struct BgSstFloor;

typedef struct BgSstFloor {
    /* 0x0000 */ DynaPolyActor dyna;
    /* 0x0164 */ s16 drumPhase;
    /* 0x0166 */ s16 drumAmp;
    /* 0x0168 */ s16 drumHeight;
    /* 0x016A */ s16 visualPulse;
} BgSstFloor; // size = 0x016C

typedef enum {
    /* 0 */ BONGOFLOOR_REST,
    /* 1 */ BONGOFLOOR_HIT,
    /* 2 */ BONGOFLOOR_HIT_LIGHT,
    /* 3 */ BONGOFLOOR_HIT_TAP,
    /* 4 */ BONGOFLOOR_HIT_PULSE,
    /* 5 */ BONGOFLOOR_HIT_SKYBOUND
} BgSstFloorParams;

#endif
