#ifndef Z_BOSS_DODONGO_H
#define Z_BOSS_DODONGO_H

#include <libultraship/libultra.h>
#include "global.h"

struct BossDodongo;

typedef void (*BossDodongoActionFunc)(struct BossDodongo*, PlayState*);

typedef struct {
    /* 0x00 */ Vec3f unk_00;
    /* 0x0C */ Vec3f unk_0C;
    /* 0x18 */ Vec3f unk_18;
    /* 0x24 */ u8 unk_24;
    /* 0x25 */ u8 unk_25;
    /* 0x26 */ Color_RGB8 color;
    /* 0x2A */ s16 alpha;
    /* 0x2C */ f32 unk_2C;
    /* 0x30 */ u32 epoch;
} BossDodongoEffect; // Size = 0x34

typedef struct BossDodongo {
    /* 0x0000 */ Actor actor;
    /* 0x014C */ SkelAnime skelAnime;
    /* 0x0190 */ BossDodongoActionFunc actionFunc;
    /* 0x0194 */ s16 health;
    /* 0x0196 */ s16 unk_196;
    /* 0x0198 */ s16 unk_198;
    /* 0x019A */ s16 unk_19A;
    /* 0x019C */ s16 csState;
    /* 0x019E */ s16 unk_19E;
    /* 0x01A0 */ s16 unk_1A0;
    /* 0x01A2 */ s16 unk_1A2;
    /* 0x01A4 */ s16 unk_1A4;
    /* 0x01A6 */ s16 unk_1A6;
    /* 0x01A8 */ s16 numWallCollisions;
    /* 0x01AA */ s16 unk_1AA;
    /* 0x01AC */ s16 unk_1AC;
    /* 0x01AE */ s16 unk_1AE;
    /* 0x01B0 */ s16 unk_1B0;
    /* 0x01B2 */ char unk_1B2[0x2];
    /* 0x01B4 */ s16 cutsceneCamera;
    /* 0x01B6 */ s16 unk_1B6;
    /* 0x01B8 */ s16 playerYawInRange;
    /* 0x01BA */ s16 playerPosInRange;
    /* 0x01BC */ s16 unk_1BC;
    /* 0x01BE */ s16 unk_1BE;
    /* 0x01C0 */ s16 unk_1C0;
    /* 0x01C2 */ s16 unk_1C2;
    /* 0x01C4 */ s16 unk_1C4; // Some kind of angle
    /* 0x01C6 */ s16 unk_1C6;
    /* 0x01C8 */ s16 unk_1C8;
    /* 0x01CA */ char unk_1CA[0x2];
    /* 0x01CC */ s16 unk_1CC;
    /* 0x01CE */ char unk_1CE[0xC];
    /* 0x01DA */ s16 unk_1DA;
    /* 0x01DC */ s16 unk_1DC;
    /* 0x01DE */ s16 dodojrSpawnTimer;
    /* 0x01E0 */ s16 dodojrSpawnedThisCycle;
    /* 0x01E2 */ u8 unk_1E2;
    /* 0x01E3 */ u8 lightningActive;
    /* 0x01E4 */ s16 rollingRockTimer;
    /* 0x01E6 */ s16 lightningTimer;
    /* 0x01E8 */ f32 unk_1E4;
    /* 0x01EC */ f32 unk_1E8;
    /* 0x01F0 */ f32 unk_1EC;
    /* 0x01F4 */ char unk_1F0[0x8];
    /* 0x01FC */ f32 unk_1F8;
    /* 0x0200 */ f32 unk_1FC;
    /* 0x0204 */ f32 unk_200;
    /* 0x0208 */ f32 unk_204;
    /* 0x020C */ f32 unk_208;
    /* 0x0210 */ f32 unk_20C;
    /* 0x0214 */ f32 colorFilterR;
    /* 0x0218 */ f32 colorFilterG;
    /* 0x021C */ f32 colorFilterB;
    /* 0x0220 */ f32 colorFilterMin;
    /* 0x0224 */ f32 colorFilterMax;
    /* 0x0228 */ f32 unk_224;
    /* 0x022C */ f32 unk_228;
    /* 0x0230 */ f32 unk_22C;
    /* 0x0234 */ f32 unk_230;
    /* 0x0238 */ f32 unk_234;
    /* 0x023C */ f32 unk_238;
    /* 0x0240 */ f32 unk_23C;
    /* 0x0244 */ f32 unk_240;
    /* 0x0248 */ f32 unk_244;
    /* 0x024C */ f32 rollingLightPulseTimer;
    /* 0x0250 */ f32 rollingFogStrength;
    /* 0x0254 */ f32 rollingFogTarget;
    /* 0x0258 */ u8 rollingEnvApplied;
    /* 0x0259 */ char unk_259[0x3];
    /* 0x025C */ f32 rollingFogNearOffset;
    /* 0x0260 */ f32 unk_25C[50];
    /* 0x0328 */ f32 unk_324[50];
    /* 0x03F0 */ Vec3f vec;
    /* 0x03FC */ Vec3f firePos;
    /* 0x0408 */ Vec3f unk_404;
    /* 0x0414 */ Vec3f unk_410;
    /* 0x0420 */ Vec3f mouthPos;
    /* 0x042C */ Vec3f cameraEye;
    /* 0x0438 */ Vec3f cameraAt;
    /* 0x0440 */ ColliderJntSph collider;
    /* 0x0460 */ ColliderJntSphElement items[19];
    /* 0x0920 */ BossDodongoEffect effects[80];
} BossDodongo; // size = 0x1820

#endif
