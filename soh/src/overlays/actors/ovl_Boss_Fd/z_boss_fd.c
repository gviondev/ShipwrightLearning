/*
 * File: z_boss_fd.c
 * Overlay: ovl_Boss_Fd
 * Description: Volvagia, flying form
 */

#include "z_boss_fd.h"
#include "textures/boss_title_cards/object_fd.h"
#include "objects/object_fd/object_fd.h"
#include "overlays/actors/ovl_En_Vb_Ball/z_en_vb_ball.h"
#include "overlays/actors/ovl_Bg_Vb_Sima/z_bg_vb_sima.h"
#include "overlays/actors/ovl_Boss_Fd2/z_boss_fd2.h"
#include "overlays/actors/ovl_Door_Warp1/z_door_warp1.h"
#include "objects/gameplay_keep/gameplay_keep.h"

#include "soh/frame_interpolation.h"
#include "soh/Enhancements/game-interactor/GameInteractor.h"
#include "soh/Enhancements/game-interactor/GameInteractor_Hooks.h"

#define FLAGS                                                                                 \
    (ACTOR_FLAG_ATTENTION_ENABLED | ACTOR_FLAG_HOSTILE | ACTOR_FLAG_UPDATE_CULLING_DISABLED | \
     ACTOR_FLAG_DRAW_CULLING_DISABLED)

// The arena and entrance platform top out at y = 100; keep the full body route above both.
#define BOSSFD_LOW_CIRCLE_RADIUS_DEFAULT 600.0f
#define BOSSFD_LOW_CIRCLE_SAFE_HEIGHT 170.0f
#define BOSSFD_LOW_CIRCLE_ENTRY_TIME 120
#define BOSSFD_LOW_CIRCLE_TIMER 400
#define BOSSFD_LOW_CIRCLE_SPEED_DEFAULT 11.0f
#define BOSSFD_LOW_CIRCLE_ROCK_TIMER 180
#define BOSSFD_LOW_CIRCLE_ROCK_INTERVAL 30
#define BOSSFD_LOW_CIRCLE_ROCK_DEBRIS_COUNT 8
#define BOSSFD_LOW_CIRCLE_ROCK_HEIGHT 1150.0f
#define BOSSFD_LOW_CIRCLE_FIRE_INTERVAL 60
#define BOSSFD_LOW_CIRCLE_FOG_PULSE_RATE 0x800
#define BOSSFD_FOG_MODE_ERUPTION_IN 11
#define BOSSFD_FOG_MODE_ERUPTION 12
#define BOSSFD_FOG_MODE_ERUPTION_OUT 13
#define BOSSFD_LOW_CIRCLE_HEALTH_THRESHOLD BOSSFD_ENRAGED_HEALTH

#define BOSSFD_ROCKFALL_EMPOWERED_HEALTH_THRESHOLD 36
#define BOSSFD_ROCKFALL_STAGING_HEIGHT 650.0f
#define BOSSFD_ROCKFALL_BARRAGE_HEIGHT 640.0f
#define BOSSFD_ROCKFALL_ROCK_HEIGHT 1050.0f
#define BOSSFD_ROCKFALL_CLIMB_TIME 80
#define BOSSFD_ROCKFALL_WINDUP_TIME 40
#define BOSSFD_ROCKFALL_CHARGE_TIME 35
#define BOSSFD_ROCKFALL_FIRST_WAVE_DELAY 18
#define BOSSFD_ROCKFALL_WAVE_INTERVAL 44
#define BOSSFD_ROCKFALL_RETRY_TIME 4
#define BOSSFD_ROCKFALL_BARRAGE_TIMEOUT 240
#define BOSSFD_ROCKFALL_RECOVERY_TIME 80
#define BOSSFD_ROCKFALL_BASE_WAVE_COUNT 3
#define BOSSFD_ROCKFALL_EMPOWERED_WAVE_COUNT 4
#define BOSSFD_ROCKFALL_ENRAGED_WAVE_COUNT 5
#define BOSSFD_ROCKFALL_SPAWN_DEBRIS_COUNT 10
#define BOSSFD_ROCKFALL_SHATTER_DEBRIS_COUNT 24
#define BOSSFD_ROCKFALL_TEACHING_DISTANCE 170.0f
#define BOSSFD_ROCKFALL_ARENA_FLOOR_MIN 70.0f
#define BOSSFD_ROCKFALL_ARENA_FLOOR_MAX 130.0f

#define BOSSFD_MAX_ACTIVE_ROCK_LOAD 14
#define BOSSFD_ATTACK_LAST_MASK 0x00FF
#define BOSSFD_ATTACK_SEEN_EMPOWERED_ROCKFALL 0x0200
#define BOSSFD_ATTACK_SEEN_LOW_CIRCLE 0x0400

#define BOSSFD_GRAB_STAGE_MIN_TIME 36
#define BOSSFD_GRAB_STAGE_TIMEOUT 70
#define BOSSFD_GRAB_ALIGN_MIN_TIME 8
#define BOSSFD_GRAB_ALIGN_TIMEOUT 22
#define BOSSFD_GRAB_ROAR_TIME 40
#define BOSSFD_GRAB_LUNGE_TIME 38
#define BOSSFD_GRAB_LUNGE_TRACK_TIME 6
#define BOSSFD_GRAB_ASCENT_TIMEOUT 100
#define BOSSFD_GRAB_RECOVERY_TIME 36
#define BOSSFD_GRAB_FIRST_DAMAGE_DELAY 12
#define BOSSFD_GRAB_DAMAGE_INTERVAL 20
#define BOSSFD_GRAB_DAMAGE_BEATS 2
#define BOSSFD_GRAB_DAMAGE_PER_BEAT 4
#define BOSSFD_GRAB_STAGE_DISTANCE 240.0f
#define BOSSFD_GRAB_STAGE_HEIGHT 90.0f
#define BOSSFD_GRAB_STAGE_RANGE 70.0f
#define BOSSFD_GRAB_STAGE_VERTICAL_RANGE 55.0f
#define BOSSFD_GRAB_ALIGN_SPEED 3.0f
#define BOSSFD_GRAB_ALIGN_YAW_TOLERANCE 0x1000
#define BOSSFD_GRAB_BITE_POINT_HEIGHT 38.0f
#define BOSSFD_GRAB_BITE_RADIUS 60.0f
#define BOSSFD_GRAB_LUNGE_HEIGHT_MIN 135.0f
#define BOSSFD_GRAB_LUNGE_OVERSHOOT 150.0f
#define BOSSFD_GRAB_LUNGE_SPEED 14.0f
#define BOSSFD_GRAB_ASCENT_SPEED 8.0f
#define BOSSFD_GRAB_ASCENT_HEIGHT 520.0f
#define BOSSFD_GRAB_ASCENT_CENTER_SCALE 0.72f
#define BOSSFD_GRAB_ASCENT_FORWARD_DISTANCE 160.0f
#define BOSSFD_GRAB_ASCENT_MAX_RADIUS 520.0f
#define BOSSFD_GRAB_THROW_SPEED 13.0f
#define BOSSFD_GRAB_THROW_LIFT 11.0f
#define BOSSFD_GRAB_WAIST_FOOT_OFFSET -46.0f
#define BOSSFD_GRAB_RECOVERY_SPEED 9.0f
#define BOSSFD_GRAB_RECOVERY_DISTANCE 150.0f
#define BOSSFD_GRAB_RECOVERY_RISE 80.0f
#define BOSSFD_GRAB_RECOVERY_MAX_HEIGHT 540.0f

// Reuse unused work indices to track the circular flight path angle and direction
#define BOSSFD_LOW_CIRCLE_ANGLE_IDX BFD_UNK_234
#define BOSSFD_LOW_CIRCLE_DIR_IDX BFD_UNK_236

typedef enum {
    /* 0 */ INTRO_FLY_EMERGE,
    /* 1 */ INTRO_FLY_HOLE,
    /* 2 */ INTRO_FLY_CAMERA,
    /* 3 */ INTRO_FLY_RETRAT
} BossFdIntroFlyState;

typedef enum {
    /* 0 */ MANE_CENTER,
    /* 1 */ MANE_RIGHT,
    /* 2 */ MANE_LEFT
} BossFdManeIndex;

typedef enum {
    /* 0 */ EYE_OPEN,
    /* 1 */ EYE_HALF,
    /* 2 */ EYE_CLOSED
} BossFdEyeState;

typedef enum {
    /* 0 */ BOSSFD_GRAB_STAGE,
    /* 1 */ BOSSFD_GRAB_ALIGN,
    /* 2 */ BOSSFD_GRAB_LUNGE,
    /* 3 */ BOSSFD_GRAB_ASCEND,
    /* 4 */ BOSSFD_GRAB_RECOVER
} BossFdGrabPhase;

typedef enum {
    /* 0 */ BOSSFD_LOW_CIRCLE_RISE,
    /* 1 */ BOSSFD_LOW_CIRCLE_ENTER,
    /* 2 */ BOSSFD_LOW_CIRCLE_ORBIT
} BossFdLowCirclePhase;

typedef enum {
    /* 0 */ BOSSFD_ROCKFALL_CLIMB,
    /* 1 */ BOSSFD_ROCKFALL_WINDUP,
    /* 2 */ BOSSFD_ROCKFALL_CHARGE,
    /* 3 */ BOSSFD_ROCKFALL_BARRAGE,
    /* 4 */ BOSSFD_ROCKFALL_RECOVER
} BossFdRockfallPhase;

void BossFd_Init(Actor* thisx, PlayState* play);
void BossFd_Destroy(Actor* thisx, PlayState* play);
void BossFd_Update(Actor* thisx, PlayState* play);
void BossFd_Draw(Actor* thisx, PlayState* play);

void BossFd_SetupFly(BossFd* this, PlayState* play);
void BossFd_Fly(BossFd* this, PlayState* play);
void BossFd_Wait(BossFd* this, PlayState* play);
void BossFd_UpdateEffects(BossFd* this, PlayState* play);
void BossFd_DrawBody(PlayState* play, BossFd* this);
static bool BossFd_IsCombatActive(BossFd* this);

const ActorInit Boss_Fd_InitVars = {
    ACTOR_BOSS_FD,
    ACTORCAT_BOSS,
    FLAGS,
    OBJECT_FD,
    sizeof(BossFd),
    (ActorFunc)BossFd_Init,
    (ActorFunc)BossFd_Destroy,
    (ActorFunc)BossFd_Update,
    (ActorFunc)BossFd_Draw,
    NULL,
};

#include "z_boss_fd_colchk.c"

static InitChainEntry sInitChain[] = {
    ICHAIN_U8(targetMode, 5, ICHAIN_CONTINUE),
    ICHAIN_S8(naviEnemyId, 0x21, ICHAIN_CONTINUE),
    ICHAIN_F32_DIV1000(gravity, 0, ICHAIN_CONTINUE),
    ICHAIN_F32(targetArrowOffset, 0, ICHAIN_STOP),
};

static s16 BossFd_PickNextFlyAttack(BossFd* this) {
    s16 candidates[4];
    s16 filteredCandidates[4];
    s32 candidateCount = 0;
    s32 filteredCount = 0;
    s32 i;
    s16 tracker = this->work[BFD_ATTACK_TRACKER];
    s16 lastAttack = tracker & BOSSFD_ATTACK_LAST_MASK;
    s16 choice = BOSSFD_FLY_CHASE;

    if ((this->actor.colChkInfo.health <= BOSSFD_LOW_CIRCLE_HEALTH_THRESHOLD) &&
        !(tracker & BOSSFD_ATTACK_SEEN_LOW_CIRCLE)) {
        choice = BOSSFD_FLY_LOW_CIRCLE;
        goto selected;
    }
    if ((this->actor.colChkInfo.health <= BOSSFD_ROCKFALL_EMPOWERED_HEALTH_THRESHOLD) &&
        !(tracker & BOSSFD_ATTACK_SEEN_EMPOWERED_ROCKFALL) && (lastAttack != BOSSFD_FLY_ROCKFALL)) {
        choice = BOSSFD_FLY_ROCKFALL;
        goto selected;
    }
    candidates[candidateCount++] = BOSSFD_FLY_CHASE;
    candidates[candidateCount++] = BOSSFD_FLY_ROCKFALL;
    candidates[candidateCount++] = BOSSFD_FLY_GRAB;
    if (this->actor.colChkInfo.health <= BOSSFD_LOW_CIRCLE_HEALTH_THRESHOLD) {
        candidates[candidateCount++] = BOSSFD_FLY_LOW_CIRCLE;
    }

    for (i = 0; i < candidateCount; i++) {
        if (candidates[i] != lastAttack) {
            filteredCandidates[filteredCount++] = candidates[i];
        }
    }
    if (filteredCount != 0) {
        choice = filteredCandidates[(s32)Rand_ZeroFloat(filteredCount)];
    } else {
        choice = candidates[(s32)Rand_ZeroFloat(candidateCount)];
    }

selected:
    tracker &= ~BOSSFD_ATTACK_LAST_MASK;
    tracker |= choice;
    if (choice == BOSSFD_FLY_LOW_CIRCLE) {
        tracker |= BOSSFD_ATTACK_SEEN_LOW_CIRCLE;
    }
    this->work[BFD_ATTACK_TRACKER] = tracker;
    return choice;
}

static s32 BossFd_GetOwnedRockLoad(PlayState* play, BossFd* this) {
    Actor* actor = play->actorCtx.actorLists[ACTORCAT_BOSS].head;
    s32 load = 0;

    while (actor != NULL) {
        if ((actor->update != NULL) && (actor->id == ACTOR_EN_VB_BALL) && (actor->parent == &this->actor) &&
            (actor->params < 200)) {
            if (actor->params == 100) {
                load += 7;
            } else if (actor->params == 101) {
                load += 3;
            } else {
                load++;
            }
        }
        actor = actor->next;
    }
    return load;
}

static EnVbBall* BossFd_TrySpawnRock(BossFd* this, PlayState* play, f32 x, f32 y, f32 z, s16 scale, s16 params) {
    s32 spawnLoad = (params == 100) ? 7 : ((params == 101) ? 3 : 1);

    if ((BossFd_GetOwnedRockLoad(play, this) + spawnLoad) > BOSSFD_MAX_ACTIVE_ROCK_LOAD) {
        return NULL;
    }
    return (EnVbBall*)Actor_SpawnAsChild(&play->actorCtx, &this->actor, play, ACTOR_EN_VB_BALL, x, y, z, 0, 0,
                                         scale, params);
}

static void BossFd_SetupRockfall(BossFd* this) {
    this->work[BFD_ATTACK_TRACKER] &= ~BOSSFD_ATTACK_LAST_MASK;
    this->work[BFD_ATTACK_TRACKER] |= BOSSFD_FLY_ROCKFALL;
    this->work[BFD_ACTION_STATE] = BOSSFD_FLY_ROCKFALL;
    this->attackPhase = BOSSFD_ROCKFALL_CLIMB;
    this->attackStep = 0;
    this->timers[0] = BOSSFD_ROCKFALL_CLIMB_TIME;
    this->timers[1] = 0;
    this->timers[2] = 0;
    this->targetPosition.x = 0.0f;
    this->targetPosition.y = BOSSFD_ROCKFALL_STAGING_HEIGHT;
    this->targetPosition.z = -220.0f;
    this->fwork[BFD_FLY_SPEED] = 10.0f;
    this->fwork[BFD_TURN_RATE_MAX] = 3200.0f;
    this->fwork[BFD_FLY_WOBBLE_AMP] = 60.0f;
    this->fwork[BFD_CEILING_BOUNCE] = 0.0f;
    this->work[BFD_CEILING_TARGET] = 0;
    this->work[BFD_ROCK_TIMER] = 0;
    this->fireBreathTimer = 0;
}

static void BossFd_SetupRockfallRecovery(BossFd* this) {
    this->attackPhase = BOSSFD_ROCKFALL_RECOVER;
    this->timers[0] = BOSSFD_ROCKFALL_RECOVERY_TIME;
    this->timers[1] = 0;
    this->work[BFD_ROCK_TIMER] = 0;
    this->targetPosition.x = 0.0f;
    this->targetPosition.y = 360.0f;
    this->targetPosition.z = 0.0f;
}

static bool BossFd_HasGroundForm(PlayState* play, BossFd* this) {
    Actor* actor = play->actorCtx.actorLists[ACTORCAT_BOSS].head;

    while (actor != NULL) {
        if ((actor->update != NULL) && (actor->id == ACTOR_BOSS_FD2) && (actor->parent == &this->actor)) {
            return true;
        }
        actor = actor->next;
    }
    return false;
}

static void BossFd_ClearCombatHazards(BossFd* this, PlayState* play) {
    Actor* actor = play->actorCtx.actorLists[ACTORCAT_BOSS].head;
    s32 i;

    while (actor != NULL) {
        Actor* next = actor->next;

        if ((actor->update != NULL) && (actor->id == ACTOR_EN_VB_BALL) && (actor->parent == &this->actor) &&
            (actor->params < 200)) {
            EnVbBall* rock = (EnVbBall*)actor;

            rock->collider.base.atFlags &= ~(AT_ON | AT_HIT | AT_BOUNCED);
            Actor_Kill(actor);
        }
        actor = next;
    }

    for (i = 0; i < ARRAY_COUNT(this->effects); i++) {
        if (this->effects[i].type == BFD_FX_FIRE_BREATH) {
            this->effects[i].type = BFD_FX_NONE;
        }
    }
    this->fireBreathTimer = 0;
    this->work[BFD_ROCK_TIMER] = 0;
}

void BossFd_SpawnEmber(BossFdEffect* effect, Vec3f* position, Vec3f* velocity, Vec3f* acceleration, f32 scale) {
    s16 i;

    for (i = 0; i < 150; i++, effect++) {
        if (effect->type == BFD_FX_NONE) {
            effect->type = BFD_FX_EMBER;
            effect->pos = *position;
            effect->velocity = *velocity;
            effect->accel = *acceleration;
            effect->scale = scale / 1000.0f;
            effect->alpha = 255;
            effect->timer1 = (s16)Rand_ZeroFloat(10.0f);
            effect->epoch++;
            break;
        }
    }
}

void BossFd_SpawnDebris(BossFdEffect* effect, Vec3f* position, Vec3f* velocity, Vec3f* acceleration, f32 scale) {
    s16 i;

    for (i = 0; i < 150; i++, effect++) {
        if (effect->type == BFD_FX_NONE) {
            effect->type = BFD_FX_DEBRIS;
            effect->pos = *position;
            effect->velocity = *velocity;
            effect->accel = *acceleration;
            effect->scale = scale / 1000.0f;
            effect->vFdFxRotX = Rand_ZeroFloat(100.0f);
            effect->vFdFxRotY = Rand_ZeroFloat(100.0f);
            effect->epoch++;
            break;
        }
    }
}

void BossFd_SpawnDust(BossFdEffect* effect, Vec3f* position, Vec3f* velocity, Vec3f* acceleration, f32 scale) {
    s16 i;

    for (i = 0; i < 150; i++, effect++) {
        if (effect->type == BFD_FX_NONE) {
            effect->type = BFD_FX_DUST;
            effect->pos = *position;
            effect->velocity = *velocity;
            effect->accel = *acceleration;
            effect->timer2 = 0;
            effect->scale = scale / 400.0f;
            effect->epoch++;
            break;
        }
    }
}

void BossFd_SpawnFireBreath(BossFdEffect* effect, Vec3f* position, Vec3f* velocity, Vec3f* acceleration, f32 scale,
                            s16 alpha, s16 kbAngle) {
    s16 i;

    for (i = 0; i < 180; i++, effect++) {
        if (effect->type == BFD_FX_NONE) {
            effect->type = BFD_FX_FIRE_BREATH;
            effect->timer1 = 0;
            effect->pos = *position;
            effect->velocity = *velocity;
            effect->accel = *acceleration;
            effect->pos.x -= effect->velocity.x;
            effect->pos.y -= effect->velocity.y;
            effect->pos.z -= effect->velocity.z;
            effect->vFdFxScaleMod = 0.0f;
            effect->alpha = alpha;
            effect->vFdFxYStop = Rand_ZeroFloat(10.0f);
            effect->timer2 = 0;
            effect->scale = scale / 400.0f;
            effect->kbAngle = kbAngle;
            effect->epoch++;
            break;
        }
    }
}

void BossFd_SetCameraSpeed(BossFd* this, f32 speedMod) {
    this->camData.eyeVel.x = fabsf(this->camData.eye.x - this->camData.nextEye.x) * speedMod;
    this->camData.eyeVel.y = fabsf(this->camData.eye.y - this->camData.nextEye.y) * speedMod;
    this->camData.eyeVel.z = fabsf(this->camData.eye.z - this->camData.nextEye.z) * speedMod;
    this->camData.atVel.x = fabsf(this->camData.at.x - this->camData.nextAt.x) * speedMod;
    this->camData.atVel.y = fabsf(this->camData.at.y - this->camData.nextAt.y) * speedMod;
    this->camData.atVel.z = fabsf(this->camData.at.z - this->camData.nextAt.z) * speedMod;
}

void BossFd_UpdateCamera(BossFd* this, PlayState* play) {
    if (this->introCamera > CAM_ID_MAIN) {
        Math_ApproachF(&this->camData.eye.x, this->camData.nextEye.x, this->camData.eyeMaxVel.x,
                       this->camData.eyeVel.x * this->camData.speedMod);
        Math_ApproachF(&this->camData.eye.y, this->camData.nextEye.y, this->camData.eyeMaxVel.y,
                       this->camData.eyeVel.y * this->camData.speedMod);
        Math_ApproachF(&this->camData.eye.z, this->camData.nextEye.z, this->camData.eyeMaxVel.z,
                       this->camData.eyeVel.z * this->camData.speedMod);
        Math_ApproachF(&this->camData.at.x, this->camData.nextAt.x, this->camData.atMaxVel.x,
                       this->camData.atVel.x * this->camData.speedMod);
        Math_ApproachF(&this->camData.at.y, this->camData.nextAt.y, this->camData.atMaxVel.y,
                       this->camData.atVel.y * this->camData.speedMod);
        Math_ApproachF(&this->camData.at.z, this->camData.nextAt.z, this->camData.atMaxVel.z,
                       this->camData.atVel.z * this->camData.speedMod);
        Math_ApproachF(&this->camData.speedMod, 1.0f, 1.0f, this->camData.accel);
        this->camData.at.y += this->camData.yMod;
        Play_CameraSetAtEye(play, this->introCamera, &this->camData.at, &this->camData.eye);
        Math_ApproachZeroF(&this->camData.yMod, 1.0f, 0.1f);
    }
}

void BossFd_Init(Actor* thisx, PlayState* play) {
    s32 pad;
    BossFd* this = (BossFd*)thisx;
    s16 i;

    Flags_SetSwitch(play, 0x14);
    Actor_SpawnAsChild(&play->actorCtx, &this->actor, play, ACTOR_BG_VB_SIMA, 680.0f, -100.0f, 0.0f, 0, 0, 0, 100);
    Actor_ProcessInitChain(&this->actor, sInitChain);
    ActorShape_Init(&this->actor.shape, 0.0f, NULL, 0.0f);
    Actor_SetScale(&this->actor, 0.05f);
    SkelAnime_Init(play, &this->skelAnimeHead, &gVolvagiaHeadSkel, &gVolvagiaHeadEmergeAnim, NULL, NULL, 0);
    SkelAnime_Init(play, &this->skelAnimeRightArm, &gVolvagiaRightArmSkel, &gVolvagiaRightArmEmergeAnim, NULL, NULL, 0);
    SkelAnime_Init(play, &this->skelAnimeLeftArm, &gVolvagiaLeftArmSkel, &gVolvagiaLeftArmEmergeAnim, NULL, NULL, 0);
    this->introState = BFD_CS_WAIT;
    this->introCamera = SUBCAM_FREE;
    if (this->introState == BFD_CS_NONE) {
        Audio_QueueSeqCmd(SEQ_PLAYER_BGM_MAIN << 24 | NA_BGM_FIRE_BOSS);
    }

    this->actor.world.pos.x = this->actor.world.pos.z = 0.0f;
    this->actor.world.pos.y = -200.0f;
    Collider_InitJntSph(play, &this->collider);
    Collider_SetJntSph(play, &this->collider, &this->actor, &sJntSphInit, this->elements);

    for (i = 0; i < 100; i++) {
        this->bodySegsPos[i].x = this->actor.world.pos.x;
        this->bodySegsPos[i].y = this->actor.world.pos.y;
        this->bodySegsPos[i].z = this->actor.world.pos.z;
        if (i < 30) {
            this->centerMane.pos[i].x = this->actor.world.pos.x;
            this->centerMane.pos[i].y = this->actor.world.pos.y;
            this->centerMane.pos[i].z = this->actor.world.pos.z;
        }
    }

    this->actor.colChkInfo.health = BOSSFD_MAX_HEALTH;
    this->skinSegments = 18;
    if (this->introState == BFD_CS_NONE) {
        this->actionFunc = BossFd_Wait;
    } else {
        BossFd_SetupFly(this, play);
    }

    this->grabbingPlayer = false;
    this->grabPlayerFloorWasDisabled = false;
    this->grabTransitionedThisFrame = false;
    this->grabTimer = 0;
    this->attackPhase = 0;
    this->attackStep = 0;
    this->attackTarget = this->actor.world.pos;
    this->groundSpawnRetryTimer = 0;
    this->hazardsCleared = false;
    this->work[BFD_ATTACK_TRACKER] = 0;
    this->rightHandPos = this->actor.world.pos;
    this->rightHandForward.x = 0.0f;
    this->rightHandForward.y = 0.0f;
    this->rightHandForward.z = 0.0f;
    this->mouthForward.x = 0.0f;
    this->mouthForward.y = 0.0f;
    this->mouthForward.z = 0.0f;
    this->grabbedWaistOffset.x = 0.0f;
    this->grabbedWaistOffset.y = 0.0f;
    this->grabbedWaistOffset.z = 0.0f;

    if (Flags_GetClear(play, play->roomCtx.curRoom.num)) {
        Actor_Kill(&this->actor);
        Actor_SpawnAsChild(&play->actorCtx, &this->actor, play, ACTOR_DOOR_WARP1, 0.0f, 100.0f, 0.0f, 0, 0, 0,
                           WARP_DUNGEON_ADULT);
        if (GameInteractor_Should(VB_SPAWN_HEART_CONTAINER, true)) {
            Actor_Spawn(&play->actorCtx, play, ACTOR_ITEM_B_HEART, 0.0f, 100.0f, 200.0f, 0, 0, 0, 0);
        }
    } else {
        if (Actor_SpawnAsChild(&play->actorCtx, &this->actor, play, ACTOR_BOSS_FD2, this->actor.world.pos.x,
                               this->actor.world.pos.y, this->actor.world.pos.z, 0, 0, 0,
                               this->introState) == NULL) {
            this->groundSpawnRetryTimer = 1;
        }
    }
}

static Vec3f BossFd_GetGrabAnchor(BossFd* this) {
    return this->headPos;
}

static f32 BossFd_GetHeadDrawOffset(BossFd* this, f32 speed) {
    f32 offset;

    if (this->work[BFD_ACTION_STATE] >= BOSSFD_SKULL_FALL) {
        return -20.0f;
    }

    offset = -10.0f - ((speed - 5.0f) * 10.0f);
    if (this->work[BFD_ACTION_STATE] == BOSSFD_FLY_GRAB) {
        offset = CLAMP(offset, -70.0f, -20.0f);
    }
    return offset;
}

static Vec3f BossFd_GrabOffsetToLocal(Vec3f* offset, f32 pitch, f32 yaw) {
    Vec3f yawSpace;
    Vec3f local;
    f32 sinPitch = sinf(pitch);
    f32 cosPitch = cosf(pitch);
    f32 sinYaw = sinf(yaw);
    f32 cosYaw = cosf(yaw);

    yawSpace.x = (offset->x * cosYaw) - (offset->z * sinYaw);
    yawSpace.y = offset->y;
    yawSpace.z = (offset->x * sinYaw) + (offset->z * cosYaw);
    local.x = yawSpace.x;
    local.y = (yawSpace.y * cosPitch) - (yawSpace.z * sinPitch);
    local.z = (yawSpace.y * sinPitch) + (yawSpace.z * cosPitch);
    return local;
}

static Vec3f BossFd_GrabOffsetToWorld(Vec3f* offset, f32 pitch, f32 yaw) {
    Vec3f pitchSpace;
    Vec3f world;
    f32 sinPitch = sinf(pitch);
    f32 cosPitch = cosf(pitch);
    f32 sinYaw = sinf(yaw);
    f32 cosYaw = cosf(yaw);

    pitchSpace.x = offset->x;
    pitchSpace.y = (offset->y * cosPitch) + (offset->z * sinPitch);
    pitchSpace.z = (-offset->y * sinPitch) + (offset->z * cosPitch);
    world.x = (pitchSpace.x * cosYaw) + (pitchSpace.z * sinYaw);
    world.y = pitchSpace.y;
    world.z = (-pitchSpace.x * sinYaw) + (pitchSpace.z * cosYaw);
    return world;
}

static Vec3f BossFd_GetPostMoveGrabAnchor(BossFd* this) {
    // Reproject the last rendered mouth through this frame's body-root pose so bite contact matches Draw.
    s16 currentIndex = this->work[BFD_LEAD_BODY_SEG];
    s16 previousIndex = (currentIndex + ARRAY_COUNT(this->bodySegsPos) - 1) % ARRAY_COUNT(this->bodySegsPos);
    s16 beforePreviousIndex =
        (previousIndex + ARRAY_COUNT(this->bodySegsPos) - 1) % ARRAY_COUNT(this->bodySegsPos);
    Vec3f previousOffset = { this->headPos.x - this->bodySegsPos[previousIndex].x,
                             this->headPos.y - this->bodySegsPos[previousIndex].y,
                             this->headPos.z - this->bodySegsPos[previousIndex].z };
    Vec3f localOffset = BossFd_GrabOffsetToLocal(&previousOffset, this->bodySegsRot[previousIndex].x,
                                                 this->bodySegsRot[previousIndex].y);
    Vec3f previousMovement = { this->bodySegsPos[previousIndex].x - this->bodySegsPos[beforePreviousIndex].x,
                               this->bodySegsPos[previousIndex].y - this->bodySegsPos[beforePreviousIndex].y,
                               this->bodySegsPos[previousIndex].z - this->bodySegsPos[beforePreviousIndex].z };
    f32 previousSpeed = sqrtf(SQ(previousMovement.x) + SQ(previousMovement.y) + SQ(previousMovement.z));
    f32 updateSpeedScale = R_UPDATE_RATE * 0.5f;
    Vec3f currentOffset;
    Vec3f anchor;

    if (updateSpeedScale > 0.001f) {
        // Body history stores scaled displacement, while the head draw offset consumes raw flight speed.
        previousSpeed /= updateSpeedScale;
    }
    localOffset.z += BossFd_GetHeadDrawOffset(this, this->actor.speedXZ) -
                     BossFd_GetHeadDrawOffset(this, previousSpeed);
    currentOffset = BossFd_GrabOffsetToWorld(&localOffset, this->bodySegsRot[currentIndex].x,
                                             this->bodySegsRot[currentIndex].y);
    anchor.x = this->bodySegsPos[currentIndex].x + currentOffset.x;
    anchor.y = this->bodySegsPos[currentIndex].y + currentOffset.y;
    anchor.z = this->bodySegsPos[currentIndex].z + currentOffset.z;
    return anchor;
}

static Vec3f BossFd_GetPlayerBitePoint(Player* player) {
    Vec3f bitePoint = player->actor.world.pos;

    bitePoint.y += BOSSFD_GRAB_BITE_POINT_HEIGHT;
    return bitePoint;
}

static f32 BossFd_PointToSegmentDistSq(Vec3f* point, Vec3f* start, Vec3f* end) {
    Vec3f segment = { end->x - start->x, end->y - start->y, end->z - start->z };
    Vec3f toPoint = { point->x - start->x, point->y - start->y, point->z - start->z };
    f32 segmentLengthSq = SQ(segment.x) + SQ(segment.y) + SQ(segment.z);
    f32 t = 0.0f;
    Vec3f closest;

    if (segmentLengthSq > 0.001f) {
        t = ((toPoint.x * segment.x) + (toPoint.y * segment.y) + (toPoint.z * segment.z)) / segmentLengthSq;
        if (t < 0.0f) {
            t = 0.0f;
        } else if (t > 1.0f) {
            t = 1.0f;
        }
    }
    closest.x = start->x + (segment.x * t);
    closest.y = start->y + (segment.y * t);
    closest.z = start->z + (segment.z * t);
    return SQ(point->x - closest.x) + SQ(point->y - closest.y) + SQ(point->z - closest.z);
}

static Vec3f BossFd_GetGrabStageTarget(BossFd* this, Player* player) {
    Vec3f target = player->actor.world.pos;
    f32 dx = this->actor.world.pos.x - player->actor.world.pos.x;
    f32 dz = this->actor.world.pos.z - player->actor.world.pos.z;
    f32 distance = sqrtf(SQ(dx) + SQ(dz));

    if (distance < 1.0f) {
        dx = -Math_SinS(this->actor.world.rot.y);
        dz = -Math_CosS(this->actor.world.rot.y);
        distance = 1.0f;
    }
    target.x += (dx / distance) * BOSSFD_GRAB_STAGE_DISTANCE;
    target.y += BOSSFD_GRAB_STAGE_HEIGHT;
    target.z += (dz / distance) * BOSSFD_GRAB_STAGE_DISTANCE;
    return target;
}

static void BossFd_SetupGrabRecovery(BossFd* this) {
    this->attackPhase = BOSSFD_GRAB_RECOVER;
    this->attackStep = 0;
    this->timers[0] = BOSSFD_GRAB_RECOVERY_TIME;
    this->timers[1] = 0;
    this->grabTimer = 0;
    this->work[BFD_STOP_FLAG] = false;
    this->attackTarget.x =
        this->actor.world.pos.x + (Math_SinS(this->actor.world.rot.y) * BOSSFD_GRAB_RECOVERY_DISTANCE);
    this->attackTarget.y = this->actor.world.pos.y + BOSSFD_GRAB_RECOVERY_RISE;
    if (this->attackTarget.y > BOSSFD_GRAB_RECOVERY_MAX_HEIGHT) {
        this->attackTarget.y = BOSSFD_GRAB_RECOVERY_MAX_HEIGHT;
    }
    this->attackTarget.z =
        this->actor.world.pos.z + (Math_CosS(this->actor.world.rot.y) * BOSSFD_GRAB_RECOVERY_DISTANCE);
    this->targetPosition = this->attackTarget;
    this->fwork[BFD_FLY_SPEED] = BOSSFD_GRAB_RECOVERY_SPEED;
    this->fwork[BFD_TURN_RATE_MAX] = 800.0f;
    this->fwork[BFD_FLY_WOBBLE_AMP] = 25.0f;
}

static void BossFd_ReleasePlayer(BossFd* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    bool ownsPlayer = (player != NULL) && (player->actor.parent == &this->actor);
    bool ownsOrphanedGrab =
        (player != NULL) && this->grabbingPlayer && (player->actor.parent == NULL);

    if (ownsPlayer || ownsOrphanedGrab) {
        player->stateFlags2 &= ~PLAYER_STATE2_GRABBED_BY_ENEMY;
        if (!this->grabPlayerFloorWasDisabled) {
            player->stateFlags1 &= ~PLAYER_STATE1_FLOOR_DISABLED;
        }
        player->av2.actionVar2 = 0xC8;
        if (ownsPlayer) {
            player->actor.parent = NULL;
        }
        player->actor.shape.rot.x = 0;
        player->actor.shape.rot.z = 0;
        player->actor.world.rot.x = 0;
        player->actor.world.rot.z = 0;
    }
    this->grabbingPlayer = false;
    this->grabPlayerFloorWasDisabled = false;
    this->grabTimer = 0;
}

static void BossFd_PositionGrabbedPlayer(BossFd* this, Player* player, Vec3f* grabAnchor) {
    player->actor.world.pos = *grabAnchor;
    player->actor.world.pos.y += BOSSFD_GRAB_WAIST_FOOT_OFFSET;
    player->actor.prevPos = player->actor.world.pos;
    player->actor.shape.rot.x = 0;
    player->actor.shape.rot.y = this->actor.world.rot.y + 0x8000;
    player->actor.shape.rot.z = 0;
    player->actor.world.rot = player->actor.shape.rot;
    player->linearVelocity = 0.0f;
    player->actor.speedXZ = 0.0f;
    player->actor.velocity.x = 0.0f;
    player->actor.velocity.y = 0.0f;
    player->actor.velocity.z = 0.0f;
}

static void BossFd_BeginGrabAlign(BossFd* this, Player* player) {
    this->attackPhase = BOSSFD_GRAB_ALIGN;
    this->timers[0] = BOSSFD_GRAB_ALIGN_TIMEOUT;
    this->timers[1] = BOSSFD_GRAB_ALIGN_MIN_TIME;
    this->targetPosition = BossFd_GetPlayerBitePoint(player);
    this->actor.speedXZ = BOSSFD_GRAB_ALIGN_SPEED;
    this->fwork[BFD_FLY_SPEED] = BOSSFD_GRAB_ALIGN_SPEED;
    this->fwork[BFD_TURN_RATE] = 4000.0f;
    this->fwork[BFD_TURN_RATE_MAX] = 4000.0f;
    this->fwork[BFD_FLY_WOBBLE_AMP] = 0.0f;
}

static void BossFd_UpdateGrabLungeTarget(BossFd* this, Player* player) {
    Vec3f grabAnchor = BossFd_GetGrabAnchor(this);
    Vec3f bitePoint = BossFd_GetPlayerBitePoint(player);
    Vec3f mouthOffset = { grabAnchor.x - this->actor.world.pos.x, grabAnchor.y - this->actor.world.pos.y,
                         grabAnchor.z - this->actor.world.pos.z };
    f32 directionX = bitePoint.x - grabAnchor.x;
    f32 directionZ = bitePoint.z - grabAnchor.z;
    f32 directionLength = sqrtf(SQ(directionX) + SQ(directionZ));

    if (directionLength < 1.0f) {
        directionX = Math_SinS(this->actor.world.rot.y);
        directionZ = Math_CosS(this->actor.world.rot.y);
    } else {
        directionX /= directionLength;
        directionZ /= directionLength;
    }

    this->attackTarget.x = bitePoint.x - mouthOffset.x + (directionX * BOSSFD_GRAB_LUNGE_OVERSHOOT);
    this->attackTarget.y = bitePoint.y - mouthOffset.y;
    if (this->attackTarget.y < BOSSFD_GRAB_LUNGE_HEIGHT_MIN) {
        this->attackTarget.y = BOSSFD_GRAB_LUNGE_HEIGHT_MIN;
    }
    this->attackTarget.z = bitePoint.z - mouthOffset.z + (directionZ * BOSSFD_GRAB_LUNGE_OVERSHOOT);
    this->targetPosition = this->attackTarget;
}

static void BossFd_BeginGrabLunge(BossFd* this, Player* player) {
    BossFd_UpdateGrabLungeTarget(this, player);
    this->attackPhase = BOSSFD_GRAB_LUNGE;
    this->timers[0] = BOSSFD_GRAB_LUNGE_TIME;
    this->timers[1] = 0;
    this->grabTimer = BOSSFD_GRAB_LUNGE_TIME;
    this->actor.speedXZ = BOSSFD_GRAB_LUNGE_SPEED;
    this->fwork[BFD_FLY_SPEED] = BOSSFD_GRAB_LUNGE_SPEED;
    this->fwork[BFD_TURN_RATE] = 1800.0f;
    this->fwork[BFD_TURN_RATE_MAX] = 1800.0f;
    this->fwork[BFD_FLY_WOBBLE_AMP] = 0.0f;
    this->work[BFD_MANE_EMBERS_TIMER] = 20;
    Audio_PlayActorSound2(&this->actor, NA_SE_EN_VALVAISA_SW_NAIL);
}

static bool BossFd_CapturePlayer(BossFd* this, PlayState* play, Player* player, Vec3f* grabAnchor) {
    u8 floorWasDisabled;
    f32 ascentTargetRadius;

    if ((player->actor.parent != NULL) || (play->grabPlayer == NULL)) {
        return false;
    }
    floorWasDisabled = (player->stateFlags1 & PLAYER_STATE1_FLOOR_DISABLED) != 0;
    if (!play->grabPlayer(play, player)) {
        return false;
    }

    player->actor.parent = &this->actor;
    player->av2.actionVar2 = 0xA;
    player->stateFlags1 |= PLAYER_STATE1_FLOOR_DISABLED;
    this->grabPlayerFloorWasDisabled = floorWasDisabled;
    this->grabbingPlayer = true;
    this->grabTimer = BOSSFD_GRAB_ASCENT_TIMEOUT;
    this->attackPhase = BOSSFD_GRAB_ASCEND;
    this->attackStep = 0;
    this->timers[0] = BOSSFD_GRAB_ASCENT_TIMEOUT;
    this->timers[1] = BOSSFD_GRAB_FIRST_DAMAGE_DELAY;
    this->attackTarget.x = (this->actor.world.pos.x * BOSSFD_GRAB_ASCENT_CENTER_SCALE) +
                           (Math_SinS(this->actor.world.rot.y) * BOSSFD_GRAB_ASCENT_FORWARD_DISTANCE);
    this->attackTarget.y = BOSSFD_GRAB_ASCENT_HEIGHT;
    this->attackTarget.z = (this->actor.world.pos.z * BOSSFD_GRAB_ASCENT_CENTER_SCALE) +
                           (Math_CosS(this->actor.world.rot.y) * BOSSFD_GRAB_ASCENT_FORWARD_DISTANCE);
    ascentTargetRadius = sqrtf(SQ(this->attackTarget.x) + SQ(this->attackTarget.z));
    if (ascentTargetRadius > BOSSFD_GRAB_ASCENT_MAX_RADIUS) {
        this->attackTarget.x *= BOSSFD_GRAB_ASCENT_MAX_RADIUS / ascentTargetRadius;
        this->attackTarget.z *= BOSSFD_GRAB_ASCENT_MAX_RADIUS / ascentTargetRadius;
    }
    this->targetPosition = this->attackTarget;
    this->fwork[BFD_FLY_SPEED] = BOSSFD_GRAB_ASCENT_SPEED;
    this->fwork[BFD_TURN_RATE] = 1000.0f;
    this->fwork[BFD_TURN_RATE_MAX] = 1000.0f;
    this->fwork[BFD_FLY_WOBBLE_AMP] = 10.0f;
    this->work[BFD_STOP_FLAG] = false;
    this->work[BFD_MANE_EMBERS_TIMER] = 30;
    this->fogMode = 2;
    this->grabbedWaistOffset.x = grabAnchor->x - this->actor.world.pos.x;
    this->grabbedWaistOffset.y = grabAnchor->y - this->actor.world.pos.y;
    this->grabbedWaistOffset.z = grabAnchor->z - this->actor.world.pos.z;
    BossFd_PositionGrabbedPlayer(this, player, grabAnchor);
    Actor_RequestQuakeWithSpeed(play, 1, 0x10, 0x2000);
    Audio_PlayActorSound2(&player->actor, NA_SE_PL_BODY_HIT);
    return true;
}

static void BossFd_ThrowGrabbedPlayer(BossFd* this, PlayState* play, Player* player) {
    Vec3f center = { 0.0f, player->actor.world.pos.y, 0.0f };
    s16 throwYaw;

    if ((SQ(player->actor.world.pos.x) + SQ(player->actor.world.pos.z)) < SQ(40.0f)) {
        throwYaw = this->actor.world.rot.y;
    } else {
        throwYaw = Math_Vec3f_Yaw(&player->actor.world.pos, &center);
    }
    BossFd_ReleasePlayer(this, play);
    Actor_SetPlayerKnockbackLargeNoDamage(play, &this->actor, BOSSFD_GRAB_THROW_SPEED, throwYaw,
                                           BOSSFD_GRAB_THROW_LIFT);
    Audio_PlayActorSound2(&player->actor, NA_SE_VO_LI_FALL_L);
    Actor_RequestQuakeWithSpeed(play, 1, 0x18, 0x2800);
    BossFd_SetupGrabRecovery(this);
}

static void BossFd_UpdateGrabAfterMovement(BossFd* this, PlayState* play, Player* player) {
    Vec3f previousGrabAnchor;
    Vec3f grabAnchor;

    if (this->work[BFD_ACTION_STATE] != BOSSFD_FLY_GRAB) {
        return;
    }
    previousGrabAnchor = BossFd_GetGrabAnchor(this);
    grabAnchor = BossFd_GetPostMoveGrabAnchor(this);

    if ((this->attackPhase == BOSSFD_GRAB_LUNGE) && !this->grabbingPlayer) {
        Vec3f bitePoint = BossFd_GetPlayerBitePoint(player);

        if (!GameInteractor_SecondCollisionUpdate() &&
            (BossFd_PointToSegmentDistSq(&bitePoint, &previousGrabAnchor, &grabAnchor) <
             SQ(BOSSFD_GRAB_BITE_RADIUS))) {
            BossFd_CapturePlayer(this, play, player, &grabAnchor);
        }
        if (!this->grabbingPlayer) {
            if (this->timers[0] == 0) {
                BossFd_SetupGrabRecovery(this);
            }
        }
    } else if ((this->attackPhase == BOSSFD_GRAB_ASCEND) && this->grabbingPlayer) {
        this->grabbedWaistOffset.x = grabAnchor.x - this->actor.world.pos.x;
        this->grabbedWaistOffset.y = grabAnchor.y - this->actor.world.pos.y;
        this->grabbedWaistOffset.z = grabAnchor.z - this->actor.world.pos.z;
        BossFd_PositionGrabbedPlayer(this, player, &grabAnchor);
        this->grabTimer = this->timers[0];

        if ((this->attackStep < BOSSFD_GRAB_DAMAGE_BEATS) && (this->timers[1] == 0)) {
            bool playerDied = false;

            this->attackStep++;
            if (play->damagePlayer != NULL) {
                playerDied = play->damagePlayer(play, -BOSSFD_GRAB_DAMAGE_PER_BEAT);
                Audio_PlayActorSound2(&player->actor, NA_SE_VO_LI_DAMAGE_S);
                Actor_RequestQuakeWithSpeed(play, 1, 0x14, 0x2800);
                this->work[BFD_MANE_EMBERS_TIMER] = 24;
                this->fogMode = 2;
            }
            if (playerDied || !(player->stateFlags2 & PLAYER_STATE2_GRABBED_BY_ENEMY) ||
                (player->actor.parent != &this->actor)) {
                BossFd_ReleasePlayer(this, play);
                BossFd_SetupGrabRecovery(this);
                return;
            }
            if (this->attackStep < BOSSFD_GRAB_DAMAGE_BEATS) {
                this->timers[1] = BOSSFD_GRAB_DAMAGE_INTERVAL;
            }
        }

        if (((this->actor.world.pos.y >= BOSSFD_GRAB_ASCENT_HEIGHT) &&
             (this->attackStep >= BOSSFD_GRAB_DAMAGE_BEATS)) ||
            (this->timers[0] == 0)) {
            BossFd_ThrowGrabbedPlayer(this, play, player);
        }
    }
}

void BossFd_Destroy(Actor* thisx, PlayState* play) {
    s32 pad;
    BossFd* this = (BossFd*)thisx;
    Player* player = GET_PLAYER(play);

    SkelAnime_Free(&this->skelAnimeHead, play);
    SkelAnime_Free(&this->skelAnimeRightArm, play);
    SkelAnime_Free(&this->skelAnimeLeftArm, play);
    Collider_DestroyJntSph(play, &this->collider);

    if (this->grabbingPlayer || ((player != NULL) && (player->actor.parent == &this->actor))) {
        BossFd_ReleasePlayer(this, play);
    }
}

s32 BossFd_IsFacingLink(BossFd* this) {
    return ABS((s16)(this->actor.yawTowardsPlayer - this->actor.world.rot.y)) < 0x2000;
}

void BossFd_SetupFly(BossFd* this, PlayState* play) {
    Animation_PlayOnce(&this->skelAnimeHead, &gVolvagiaHeadEmergeAnim);
    Animation_PlayOnce(&this->skelAnimeRightArm, &gVolvagiaRightArmEmergeAnim);
    Animation_PlayOnce(&this->skelAnimeLeftArm, &gVolvagiaLeftArmEmergeAnim);
    this->actionFunc = BossFd_Fly;
    this->fwork[BFD_TURN_RATE_MAX] = 1000.0f;
}

static void BossFd_RelocateFlightHistory(BossFd* this, const Vec3f* position) {
    Vec3f translation;
    s16 i;

    // BossFd_Wait teleports the flying form between holes; never let the trail bridge that discontinuity.
    translation.x = position->x - this->actor.world.pos.x;
    translation.y = position->y - this->actor.world.pos.y;
    translation.z = position->z - this->actor.world.pos.z;
    for (i = 0; i < ARRAY_COUNT(this->bodySegsPos); i++) {
        this->bodySegsPos[i].x += translation.x;
        this->bodySegsPos[i].y += translation.y;
        this->bodySegsPos[i].z += translation.z;
    }

    for (i = 0; i < ARRAY_COUNT(this->centerMane.pos); i++) {
        this->centerMane.pos[i].x += translation.x;
        this->centerMane.pos[i].y += translation.y;
        this->centerMane.pos[i].z += translation.z;
        this->rightMane.pos[i].x += translation.x;
        this->rightMane.pos[i].y += translation.y;
        this->rightMane.pos[i].z += translation.z;
        this->leftMane.pos[i].x += translation.x;
        this->leftMane.pos[i].y += translation.y;
        this->leftMane.pos[i].z += translation.z;
    }
    this->centerMane.head.x += translation.x;
    this->centerMane.head.y += translation.y;
    this->centerMane.head.z += translation.z;
    this->rightMane.head.x += translation.x;
    this->rightMane.head.y += translation.y;
    this->rightMane.head.z += translation.z;
    this->leftMane.head.x += translation.x;
    this->leftMane.head.y += translation.y;
    this->leftMane.head.z += translation.z;
    this->headPos.x += translation.x;
    this->headPos.y += translation.y;
    this->headPos.z += translation.z;

    this->actor.world.pos = *position;
    this->actor.prevPos = *position;
}

static Vec3f sHoleLocations[] = {
    { 0.0f, 90.0f, -243.0f },    { 0.0f, 90.0f, 0.0f },    { 0.0f, 90.0f, 243.0f },
    { -243.0f, 90.0f, -243.0f }, { -243.0f, 90.0f, 0.0f }, { -243.0f, 90.0f, 243.0f },
    { 243.0f, 90.0f, -243.0f },  { 243.0f, 90.0f, 0.0f },  { 243.0f, 90.0f, 243.0f },
};

static Vec3f sCeilingTargets[] = {
    { 0.0f, 900.0f, -243.0f }, { 243.0, 900.0f, -100.0f },  { 243.0f, 900.0f, 100.0f },
    { 0.0f, 900.0f, 243.0f },  { -243.0f, 900.0f, 100.0f }, { -243.0, 900.0f, -100.0f },
};

static const Vec3f sRockfallSafePoints[] = {
    { -122.0f, 0.0f, -122.0f },
    { -122.0f, 0.0f, 122.0f },
    { 122.0f, 0.0f, 122.0f },
    { 122.0f, 0.0f, -122.0f },
};

static s16 BossFd_GetRockfallWaveCount(BossFd* this) {
    if (this->actor.colChkInfo.health <= BOSSFD_ENRAGED_HEALTH) {
        return BOSSFD_ROCKFALL_ENRAGED_WAVE_COUNT;
    }
    if (this->actor.colChkInfo.health <= BOSSFD_ROCKFALL_EMPOWERED_HEALTH_THRESHOLD) {
        return BOSSFD_ROCKFALL_EMPOWERED_WAVE_COUNT;
    }
    return BOSSFD_ROCKFALL_BASE_WAVE_COUNT;
}

static void BossFd_SpawnRockfallDebris(BossFd* this, Vec3f* origin, s32 count, f32 spread, f32 downwardSpeed) {
    s32 i;

    for (i = 0; i < count; i++) {
        Vec3f position = { origin->x + Rand_CenteredFloat(spread), origin->y + Rand_CenteredFloat(spread * 0.25f),
                           origin->z + Rand_CenteredFloat(spread) };
        Vec3f velocity = { Rand_CenteredFloat(spread * 0.12f), -Rand_ZeroFloat(downwardSpeed) - 1.0f,
                           Rand_CenteredFloat(spread * 0.12f) };
        Vec3f acceleration = { 0.0f, -0.4f, 0.0f };

        BossFd_SpawnDebris(this->effects, &position, &velocity, &acceleration, Rand_ZeroFloat(15.0f) + 15.0f);
    }
}

static void BossFd_BeginRockfallBarrage(BossFd* this, PlayState* play) {
    Vec3f shatterPosition = this->actor.world.pos;

    this->attackPhase = BOSSFD_ROCKFALL_BARRAGE;
    this->attackStep = 0;
    this->timers[0] = BOSSFD_ROCKFALL_BARRAGE_TIMEOUT;
    this->timers[1] = BOSSFD_ROCKFALL_FIRST_WAVE_DELAY;
    this->work[BFD_CEILING_TARGET] = 0;
    this->fwork[BFD_CEILING_BOUNCE] = -18384.0f;
    this->work[BFD_MANE_EMBERS_TIMER] = 30;
    this->fogMode = 2;

    Audio_PlaySoundGeneral(NA_SE_EV_EXPLOSION, &this->actor.projectedPos, 4, &gSfxDefaultFreqAndVolScale,
                           &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
    Actor_RequestQuakeWithSpeed(play, 3, 0x18, 0x7530);
    BossFd_SpawnRockfallDebris(this, &shatterPosition, BOSSFD_ROCKFALL_SHATTER_DEBRIS_COUNT, 80.0f, 10.0f);
}

static Vec3f BossFd_GetRockfallSafePointNear(Vec3f* position, f32 desiredDistance) {
    Vec3f safePoint = sRockfallSafePoints[0];
    f32 bestDistanceError = 100000.0f;
    s32 i;

    for (i = 0; i < ARRAY_COUNT(sRockfallSafePoints); i++) {
        f32 distance = sqrtf(SQ(sRockfallSafePoints[i].x - position->x) +
                             SQ(sRockfallSafePoints[i].z - position->z));
        f32 distanceError = fabsf(distance - desiredDistance);

        if (distanceError < bestDistanceError) {
            bestDistanceError = distanceError;
            safePoint = sRockfallSafePoints[i];
        }
    }
    return safePoint;
}

static bool BossFd_IsRockfallLandingSafe(BossFd* this, PlayState* play, Vec3f* position) {
    CollisionPoly* floorPoly;
    s32 bgId;
    Vec3f probePosition = *position;
    f32 floorHeight;

    probePosition.y = BOSSFD_ROCKFALL_BARRAGE_HEIGHT;
    floorHeight = BgCheck_EntityRaycastFloor4(&play->colCtx, &floorPoly, &bgId, &this->actor, &probePosition);
    return (floorHeight >= BOSSFD_ROCKFALL_ARENA_FLOOR_MIN) &&
           (floorHeight <= BOSSFD_ROCKFALL_ARENA_FLOOR_MAX);
}

static bool BossFd_TrySpawnRockfallWave(BossFd* this, PlayState* play, Player* player) {
    Vec3f spawnPosition;
    EnVbBall* rock;
    s32 wave = this->attackStep;
    s16 scale = (s16)Rand_ZeroFloat(20.0f) + 155 + (wave * 5);

    if (wave == 0) {
        spawnPosition =
            BossFd_GetRockfallSafePointNear(&player->actor.world.pos, BOSSFD_ROCKFALL_TEACHING_DISTANCE);
    } else {
        spawnPosition = player->actor.world.pos;
        if (!BossFd_IsRockfallLandingSafe(this, play, &spawnPosition)) {
            spawnPosition = BossFd_GetRockfallSafePointNear(&player->actor.world.pos, 0.0f);
        }
    }
    spawnPosition.y = BOSSFD_ROCKFALL_ROCK_HEIGHT;

    rock = BossFd_TrySpawnRock(this, play, spawnPosition.x, spawnPosition.y, spawnPosition.z, scale, 100);
    if (rock == NULL) {
        return false;
    }

    BossFd_SpawnRockfallDebris(this, &spawnPosition, BOSSFD_ROCKFALL_SPAWN_DEBRIS_COUNT, 40.0f, 5.0f);
    if (this->actor.colChkInfo.health <= BOSSFD_ROCKFALL_EMPOWERED_HEALTH_THRESHOLD) {
        this->work[BFD_ATTACK_TRACKER] |= BOSSFD_ATTACK_SEEN_EMPOWERED_ROCKFALL;
    }
    this->attackStep++;
    this->work[BFD_CEILING_TARGET] = (this->work[BFD_CEILING_TARGET] + 1) % ARRAY_COUNT(sCeilingTargets);
    this->work[BFD_MANE_EMBERS_TIMER] = 18;

    if (this->attackStep >= BossFd_GetRockfallWaveCount(this)) {
        this->fogMode = 2;
        BossFd_SetupRockfallRecovery(this);
    } else {
        this->timers[1] = BOSSFD_ROCKFALL_WAVE_INTERVAL;
    }
    return true;
}

void BossFd_Fly(BossFd* this, PlayState* play) {
    u8 sp1CF = false;
    u8 temp_rand;
    s16 i1;
    s16 i2;
    s16 i3;
    f32 dx;
    f32 dy;
    f32 dz;
    Player* player = GET_PLAYER(play);
    f32 angleToTarget;
    f32 pitchToTarget;
    Vec3f* holePosition1;
    f32 temp_x;
    f32 temp_z;
    f32 temp;
    bool aggressiveTuning =
        (this->introState == BFD_CS_NONE) && (this->work[BFD_ACTION_STATE] < BOSSFD_DEATH_START);

    if ((this->actor.colChkInfo.health == 0) && (this->work[BFD_ACTION_STATE] == BOSSFD_FLY_GRAB)) {
        if (this->grabbingPlayer || (player->actor.parent == &this->actor)) {
            BossFd_ReleasePlayer(this, play);
        }
        this->attackPhase = 0;
        this->attackStep = 0;
        this->timers[0] = 0;
        this->work[BFD_ACTION_STATE] = BOSSFD_FLY_MAIN;
        this->work[BFD_START_ATTACK] = false;
        this->grabTransitionedThisFrame = true;
    }
    if (this->grabbingPlayer &&
        ((player->actor.parent != &this->actor) || !(player->stateFlags2 & PLAYER_STATE2_GRABBED_BY_ENEMY))) {
        BossFd_ReleasePlayer(this, play);
        if (this->work[BFD_ACTION_STATE] == BOSSFD_FLY_GRAB) {
            BossFd_SetupGrabRecovery(this);
        }
    }
    if (!this->grabbingPlayer && (player->actor.parent == &this->actor)) {
        BossFd_ReleasePlayer(this, play);
        if (this->work[BFD_ACTION_STATE] == BOSSFD_FLY_GRAB) {
            BossFd_SetupGrabRecovery(this);
        }
    }
    if ((this->work[BFD_ACTION_STATE] != BOSSFD_FLY_GRAB) &&
        (this->grabbingPlayer || (player->actor.parent == &this->actor))) {
        BossFd_ReleasePlayer(this, play);
    }

    SkelAnime_Update(&this->skelAnimeHead);
    SkelAnime_Update(&this->skelAnimeRightArm);
    SkelAnime_Update(&this->skelAnimeLeftArm);
    dx = this->targetPosition.x - this->actor.world.pos.x;
    dy = this->targetPosition.y - this->actor.world.pos.y;
    dz = this->targetPosition.z - this->actor.world.pos.z;
    dx += Math_SinS((2096.0f + this->fwork[BFD_FLY_WOBBLE_RATE]) * this->work[BFD_MOVE_TIMER]) *
          this->fwork[BFD_FLY_WOBBLE_AMP];
    dy += Math_SinS((1096.0f + this->fwork[BFD_FLY_WOBBLE_RATE]) * this->work[BFD_MOVE_TIMER]) *
          this->fwork[BFD_FLY_WOBBLE_AMP];
    dz += Math_SinS((1796.0f + this->fwork[BFD_FLY_WOBBLE_RATE]) * this->work[BFD_MOVE_TIMER]) *
          this->fwork[BFD_FLY_WOBBLE_AMP];
    angleToTarget = (s16)(Math_FAtan2F(dx, dz) * (0x8000 / M_PI));
    pitchToTarget = (s16)(Math_FAtan2F(dy, sqrtf(SQ(dx) + SQ(dz))) * (0x8000 / M_PI));

    osSyncPrintf("MODE %d\n", this->work[BFD_ACTION_STATE]);

    Math_ApproachF(&this->fwork[BFD_BODY_PULSE], 0.1f, 1.0f, 0.02);

    //                                        Boss Intro Cutscene

    if (this->introState != BFD_CS_NONE) {
        Player* player2 = GET_PLAYER(play);
        Camera* mainCam = Play_GetCamera(play, CAM_ID_MAIN);

        switch (this->introState) {
            case BFD_CS_WAIT:
                this->fogMode = 3;
                this->targetPosition.x = 0.0f;
                this->targetPosition.y = -110.0f;
                this->targetPosition.z = 0.0;
                this->fwork[BFD_TURN_RATE_MAX] = 10000.0f;
                this->work[BFD_ACTION_STATE] = BOSSFD_WAIT_INTRO;
                if ((fabsf(player2->actor.world.pos.z) < 80.0f) &&
                    (fabsf(player2->actor.world.pos.x - 340.0f) < 60.0f)) {

                    this->introState = BFD_CS_START;
                    func_80064520(play, &play->csCtx);
                    Player_SetCsActionWithHaltedActors(play, &this->actor, 8);
                    this->introCamera = Play_CreateSubCamera(play);
                    if (this->introCamera > CAM_ID_MAIN) {
                        Play_ChangeCameraStatus(play, CAM_ID_MAIN, CAM_STAT_WAIT);
                        Play_ChangeCameraStatus(play, this->introCamera, CAM_STAT_ACTIVE);
                    } else {
                        this->introCamera = SUBCAM_FREE;
                    }
                    player2->actor.world.pos.x = 380.0f;
                    player2->actor.world.pos.y = 100.0f;
                    player2->actor.world.pos.z = 0.0f;
                    player2->actor.shape.rot.y = player2->actor.world.rot.y = -0x4000;
                    player2->actor.speedXZ = 0.0f;
                    this->camData.eye.x = player2->actor.world.pos.x - 70.0f;
                    this->camData.eye.y = player2->actor.world.pos.y + 40.0f;
                    this->camData.eye.z = player2->actor.world.pos.z + 70.0f;
                    this->camData.at.x = player2->actor.world.pos.x;
                    this->camData.at.y = player2->actor.world.pos.y + 30.0f;
                    this->camData.at.z = player2->actor.world.pos.z;
                    this->camData.nextEye.x = player2->actor.world.pos.x - 50.0f + 18.0f;
                    this->camData.nextEye.y = player2->actor.world.pos.y + 40;
                    this->camData.nextEye.z = player2->actor.world.pos.z + 50.0f - 18.0f;
                    this->camData.nextAt.x = player2->actor.world.pos.x;
                    this->camData.nextAt.y = player2->actor.world.pos.y + 50.0f;
                    this->camData.nextAt.z = player2->actor.world.pos.z;
                    BossFd_SetCameraSpeed(this, 1.0f);
                    this->camData.atMaxVel.x = this->camData.atMaxVel.y = this->camData.atMaxVel.z = 0.05f;
                    this->camData.eyeMaxVel.x = this->camData.eyeMaxVel.y = this->camData.eyeMaxVel.z = 0.05f;
                    this->timers[0] = 0;
                    this->camData.speedMod = 0.0f;
                    this->camData.accel = 0.0f;
                    if (Flags_GetEventChkInf(EVENTCHKINF_BEGAN_VOLVAGIA_BATTLE)) {
                        this->introState = BFD_CS_EMERGE;
                        this->camData.nextEye.x = player2->actor.world.pos.x + 100.0f + 300.0f - 600.0f;
                        this->camData.nextEye.y = player2->actor.world.pos.y + 100.0f - 50.0f;
                        this->camData.nextEye.z = player2->actor.world.pos.z + 200.0f - 150.0f;
                        this->camData.nextAt.x = 0.0f;
                        this->camData.nextAt.y = 120.0f;
                        this->camData.nextAt.z = 0.0f;
                        BossFd_SetCameraSpeed(this, 0.5f);
                        this->camData.eyeMaxVel.x = this->camData.eyeMaxVel.y = this->camData.eyeMaxVel.z = 0.1f;
                        this->camData.atMaxVel.x = this->camData.atMaxVel.y = this->camData.atMaxVel.z = 0.1f;
                        this->camData.accel = 0.005f;
                        this->timers[0] = 0;
                        this->holeIndex = 1;
                        this->targetPosition.x = sHoleLocations[this->holeIndex].x;
                        this->targetPosition.y = sHoleLocations[this->holeIndex].y - 200.0f;
                        this->targetPosition.z = sHoleLocations[this->holeIndex].z;
                        this->timers[0] = 50;
                        this->work[BFD_ACTION_STATE] = BOSSFD_EMERGE;
                        this->actor.world.rot.x = 0x4000;
                        this->work[BFD_MOVE_TIMER] = 0;
                        this->timers[3] = 250;
                        this->timers[2] = 470;
                        this->fwork[BFD_FLY_SPEED] = 5.0f;
                    }
                }
                break;
            case BFD_CS_START:
                if (this->timers[0] == 0) {
                    this->camData.accel = 0.0010000002f;
                    this->timers[0] = 100;
                    this->introState = BFD_CS_LOOK_LINK;
                }
            case BFD_CS_LOOK_LINK:
                player2->actor.world.pos.x = 380.0f;
                player2->actor.world.pos.y = 100.0f;
                player2->actor.world.pos.z = 0.0f;
                player2->actor.speedXZ = 0.0f;
                player2->actor.shape.rot.y = player2->actor.world.rot.y = -0x4000;
                if (this->timers[0] == 50) {
                    this->fogMode = 1;
                }
                if (this->timers[0] < 50) {
                    Audio_PlaySoundGeneral(NA_SE_EN_DODO_K_ROLL - SFX_FLAG, &this->actor.projectedPos, 4,
                                           &gSfxDefaultFreqAndVolScale, &gSfxDefaultFreqAndVolScale,
                                           &gSfxDefaultReverb);
                    this->camData.yMod = Math_CosS(this->work[BFD_MOVE_TIMER] * 0x8000) * this->camData.shake;
                    Math_ApproachF(&this->camData.shake, 2.0f, 1.0f, 0.8 * 0.01f);
                }
                if (this->timers[0] == 40) {
                    Player_SetCsActionWithHaltedActors(play, &this->actor, 0x13);
                }
                if (this->timers[0] == 0) {
                    this->introState = BFD_CS_LOOK_GROUND;
                    this->camData.nextAt.y = player2->actor.world.pos.y + 10.0f;
                    this->camData.atMaxVel.y = 0.2f;
                    this->camData.speedMod = 0.0f;
                    this->camData.accel = 0.02f;
                    this->timers[0] = 70;
                    this->work[BFD_MOVE_TIMER] = 0;
                }
                break;
            case BFD_CS_LOOK_GROUND:
                this->camData.yMod = Math_CosS(this->work[BFD_MOVE_TIMER] * 0x8000) * this->camData.shake;
                Math_ApproachF(&this->camData.shake, 2.0f, 1.0f, 0.8 * 0.01f);
                Audio_PlaySoundGeneral(NA_SE_EN_DODO_K_ROLL - SFX_FLAG, &this->actor.projectedPos, 4,
                                       &gSfxDefaultFreqAndVolScale, &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
                if (this->timers[0] == 0) {
                    this->introState = BFD_CS_COLLAPSE;
                    this->camData.nextEye.x = player2->actor.world.pos.x + 100.0f + 300.0f;
                    this->camData.nextEye.y = player2->actor.world.pos.y + 100.0f;
                    this->camData.nextEye.z = player2->actor.world.pos.z + 200.0f;
                    this->camData.nextAt.x = player2->actor.world.pos.x;
                    this->camData.nextAt.y = player2->actor.world.pos.y - 150.0f;
                    this->camData.nextAt.z = player2->actor.world.pos.z - 50.0f;
                    BossFd_SetCameraSpeed(this, 0.1f);
                    this->timers[0] = 170;
                    this->camData.speedMod = 0.0f;
                    this->camData.accel = 0.0f;
                    Player_SetCsActionWithHaltedActors(play, &this->actor, 0x14);
                }
                break;
            case BFD_CS_COLLAPSE:
                this->camData.accel = 0.005f;
                this->camData.yMod = Math_CosS(this->work[BFD_MOVE_TIMER] * 0x8000) * this->camData.shake;
                Math_ApproachF(&this->camData.shake, 2.0f, 1.0f, 0.8 * 0.01f);
                Audio_PlaySoundGeneral(NA_SE_EN_DODO_K_ROLL - SFX_FLAG, &this->actor.projectedPos, 4,
                                       &gSfxDefaultFreqAndVolScale, &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
                if (this->timers[0] == 100) {
                    this->platformSignal = VBSIMA_COLLAPSE;
                }
                if (this->timers[0] == 0) {
                    this->introState = BFD_CS_EMERGE;
                    this->camData.speedMod = 0.0f;
                    this->camData.nextEye.x = player2->actor.world.pos.x + 100.0f + 300.0f - 600.0f;
                    this->camData.nextEye.y = player2->actor.world.pos.y + 100.0f - 50.0f;
                    this->camData.nextEye.z = player2->actor.world.pos.z + 200.0f - 150.0f;
                    this->camData.nextAt.x = 0.0f;
                    this->camData.nextAt.y = 120.0f;
                    this->camData.nextAt.z = 0.0f;
                    BossFd_SetCameraSpeed(this, 0.5f);
                    this->camData.atMaxVel.x = this->camData.atMaxVel.y = this->camData.atMaxVel.z = 0.1f;
                    this->camData.eyeMaxVel.x = this->camData.eyeMaxVel.y = this->camData.eyeMaxVel.z = 0.1f;
                    this->camData.accel = 0.005f;
                    this->timers[0] = 0;
                    this->holeIndex = 1;
                    this->targetPosition.x = sHoleLocations[this->holeIndex].x;
                    this->targetPosition.y = sHoleLocations[this->holeIndex].y - 200.0f;
                    this->targetPosition.z = sHoleLocations[this->holeIndex].z;
                    this->timers[0] = 50;
                    this->work[BFD_ACTION_STATE] = BOSSFD_EMERGE;
                    this->actor.world.rot.x = 0x4000;
                    this->work[BFD_MOVE_TIMER] = 0;
                    this->timers[3] = 250;
                    this->timers[2] = 470;
                    this->fwork[BFD_FLY_SPEED] = 5.0f;
                }
                break;
            case BFD_CS_EMERGE:
                osSyncPrintf("WAY_SPD X = %f\n", this->camData.atVel.x);
                osSyncPrintf("WAY_SPD Y = %f\n", this->camData.atVel.y);
                osSyncPrintf("WAY_SPD Z = %f\n", this->camData.atVel.z);
                if ((this->timers[3] > 190) && !Flags_GetEventChkInf(EVENTCHKINF_BEGAN_VOLVAGIA_BATTLE)) {
                    Audio_PlaySoundGeneral(NA_SE_EN_DODO_K_ROLL - SFX_FLAG, &this->actor.projectedPos, 4,
                                           &gSfxDefaultFreqAndVolScale, &gSfxDefaultFreqAndVolScale,
                                           &gSfxDefaultReverb);
                }
                if (this->timers[3] == 190) {
                    this->camData.atMaxVel.x = this->camData.atMaxVel.y = this->camData.atMaxVel.z = 0.05f;
                    this->platformSignal = VBSIMA_KILL;
                    Player_SetCsActionWithHaltedActors(play, &this->actor, 1);
                }
                if (this->actor.world.pos.y > 120.0f) {
                    this->camData.nextAt = this->actor.world.pos;
                    this->camData.atVel.x = 190.0f;
                    this->camData.atVel.y = 85.56f;
                    this->camData.atVel.z = 25.0f;
                } else {
                    // the following `temp` stuff is probably fake but is required to match
                    // it's optimized to 1.0f because sp1CF is false at this point, but the 0.1f ends up in rodata
                    temp = 0.1f;
                    if (!sp1CF) {
                        temp = 1.0f;
                    }
                    Math_ApproachF(&this->camData.shake, 2.0f, temp, 0.1 * 0.08f);
                    this->camData.yMod = Math_CosS(this->work[BFD_MOVE_TIMER] * 0x8000) * this->camData.shake;
                }
                if (this->timers[3] == 160) {
                    Audio_QueueSeqCmd(SEQ_PLAYER_BGM_MAIN << 24 | NA_BGM_FIRE_BOSS);
                }
                if ((this->timers[3] == 130) && !Flags_GetEventChkInf(EVENTCHKINF_BEGAN_VOLVAGIA_BATTLE)) {
                    TitleCard_InitBossName(play, &play->actorCtx.titleCtx,
                                           SEGMENTED_TO_VIRTUAL(gVolvagiaBossTitleCardENGTex), 160, 180, 128, 40, true);
                }
                if (this->timers[3] <= 100) {
                    this->camData.eyeVel.x = this->camData.eyeVel.y = this->camData.eyeVel.z = 2.0f;
                    this->camData.nextEye.x = player2->actor.world.pos.x + 50.0f;
                    this->camData.nextEye.y = player2->actor.world.pos.y + 50.0f;
                    this->camData.nextEye.z = player2->actor.world.pos.z + 50.0f;
                }
                if (this->work[BFD_ACTION_STATE] == BOSSFD_FLY_HOLE) {
                    switch (this->introFlyState) {
                        case INTRO_FLY_EMERGE:
                            this->timers[5] = 100;
                            this->introFlyState = INTRO_FLY_HOLE;
                        case INTRO_FLY_HOLE:
                            if (this->timers[5] == 0) {
                                this->introFlyState = INTRO_FLY_CAMERA;
                                this->timers[5] = 75;
                            }
                            break;
                        case INTRO_FLY_CAMERA:
                            this->targetPosition = this->camData.eye;
                            if (this->timers[5] == 0) {
                                this->timers[0] = 0;
                                this->holeIndex = 7;
                                this->targetPosition.x = sHoleLocations[this->holeIndex].x;
                                this->targetPosition.y = sHoleLocations[this->holeIndex].y + 200.0f + 50.0f;
                                this->targetPosition.z = sHoleLocations[this->holeIndex].z;
                                this->introFlyState = INTRO_FLY_RETRAT;
                            }
                            if (this->timers[5] == 30) {
                                this->work[BFD_ROAR_TIMER] = 40;
                                this->fireBreathTimer = 20;
                            }
                        case INTRO_FLY_RETRAT:
                            break;
                    }
                }
                osSyncPrintf("this->timer[2] = %d\n", this->timers[2]);
                osSyncPrintf("this->timer[5] = %d\n", this->timers[5]);
                if (this->timers[2] == 0) {
                    if (this->introCamera > CAM_ID_MAIN) {
                        mainCam->eye = this->camData.eye;
                        mainCam->eyeNext = this->camData.eye;
                        mainCam->at = this->camData.at;
                        func_800C08AC(play, this->introCamera, 0);
                    }
                    this->introState = this->introFlyState = BFD_CS_NONE;
                    this->introCamera = SUBCAM_FREE;
                    func_80064534(play, &play->csCtx);
                    Player_SetCsActionWithHaltedActors(play, &this->actor, 7);
                    this->actionFunc = BossFd_Wait;
                    this->handoffSignal = FD2_SIGNAL_GROUND;
                    Flags_SetEventChkInf(EVENTCHKINF_BEGAN_VOLVAGIA_BATTLE);
                }
                break;
        }
        BossFd_UpdateCamera(this, play);
    } else {
        this->fwork[BFD_FLY_SPEED] = 5.0f;
    }

    //                             Attacks and Death Cutscene

    switch (this->work[BFD_ACTION_STATE]) {
        case BOSSFD_FLY_MAIN:
            sp1CF = true;
            if (this->timers[0] == 0) {
                if (this->actor.colChkInfo.health == 0) {
                    this->work[BFD_ACTION_STATE] = BOSSFD_DEATH_START;
                    this->timers[0] = 0;
                    this->timers[1] = 100;
                } else {
                    if (this->introState != BFD_CS_NONE) {
                        this->holeIndex = 6;
                    } else {
                        do {
                            temp_rand = Rand_ZeroFloat(8.9f);
                        } while (temp_rand == this->holeIndex);
                        this->holeIndex = temp_rand;
                    }
                    this->targetPosition.x = sHoleLocations[this->holeIndex].x;
                    this->targetPosition.y = sHoleLocations[this->holeIndex].y + 200.0f + 50.0f;
                    this->targetPosition.z = sHoleLocations[this->holeIndex].z;
                    this->fwork[BFD_TURN_RATE] = 0.0f;
                    this->fwork[BFD_TURN_RATE_MAX] = aggressiveTuning ? 1400.0f : 1000.0f;
                    if (this->introState != BFD_CS_NONE) {
                        this->timers[0] = 10050;
                    } else {
                        this->timers[0] = aggressiveTuning ? 12 : 20;
                    }
                    this->fwork[BFD_FLY_WOBBLE_AMP] = aggressiveTuning ? 140.0f : 100.0f;
                    this->work[BFD_ACTION_STATE] = BOSSFD_FLY_HOLE;

                    if (this->work[BFD_START_ATTACK]) {
                        this->work[BFD_START_ATTACK] = false;
                        switch (BossFd_PickNextFlyAttack(this)) {
                            case BOSSFD_FLY_CHASE:
                                this->work[BFD_ACTION_STATE] = BOSSFD_FLY_CHASE;
                                this->timers[0] = aggressiveTuning ? 240 : 300;
                                this->fwork[BFD_TURN_RATE_MAX] = aggressiveTuning ? 1200.0f : 900.0f;
                                this->targetPosition = player->actor.world.pos;
                                this->targetPosition.y += 30.0f;
                                break;
                            case BOSSFD_FLY_LOW_CIRCLE:
                                this->work[BFD_ACTION_STATE] = BOSSFD_FLY_LOW_CIRCLE;
                                this->attackPhase = BOSSFD_LOW_CIRCLE_RISE;
                                this->attackStep = 0;
                                this->attackTarget = this->actor.world.pos;
                                this->attackTarget.y = BOSSFD_LOW_CIRCLE_SAFE_HEIGHT;
                                this->targetPosition = this->attackTarget;
                                this->timers[0] = 0;
                                this->timers[1] = BOSSFD_LOW_CIRCLE_ENTRY_TIME;
                                this->fwork[BFD_FLY_SPEED] = BOSSFD_LOW_CIRCLE_SPEED_DEFAULT;
                                this->fwork[BFD_TURN_RATE_MAX] = 1600.0f;
                                this->work[BOSSFD_LOW_CIRCLE_ANGLE_IDX] =
                                    (s16)(Math_FAtan2F(this->actor.world.pos.x - sHoleLocations[1].x,
                                                      this->actor.world.pos.z - sHoleLocations[1].z) *
                                          (0x8000 / M_PI));
                                this->work[BOSSFD_LOW_CIRCLE_DIR_IDX] = (Rand_ZeroOne() < 0.5f) ? 1 : -1;
                                this->fwork[BFD_FLY_WOBBLE_AMP] = 0.0f;
                                this->work[BFD_ROCK_TIMER] = 0;
                                this->fogMode = BOSSFD_FOG_MODE_ERUPTION_IN;
                                play->envCtx.blendIndoorLights = true;
                                play->envCtx.unk_BE = play->envCtx.unk_BD;
                                play->envCtx.unk_BD = 3;
                                play->envCtx.unk_D8 = 0.0f;
                                play->envCtx.unk_DC = 0;
                                break;
                            case BOSSFD_FLY_ROCKFALL:
                                BossFd_SetupRockfall(this);
                                break;
                            case BOSSFD_FLY_GRAB:
                            default:
                                this->work[BFD_ACTION_STATE] = BOSSFD_FLY_GRAB;
                                this->attackPhase = BOSSFD_GRAB_STAGE;
                                this->attackStep = 0;
                                this->timers[0] = BOSSFD_GRAB_STAGE_TIMEOUT;
                                this->timers[1] = BOSSFD_GRAB_STAGE_MIN_TIME;
                                this->fwork[BFD_FLY_SPEED] = 7.0f;
                                this->fwork[BFD_TURN_RATE_MAX] = 2600.0f;
                                this->fwork[BFD_FLY_WOBBLE_AMP] = 30.0f;
                                this->grabTimer = 0;
                                this->grabbingPlayer = false;
                                this->grabPlayerFloorWasDisabled = false;
                                this->work[BFD_STOP_FLAG] = false;
                                this->work[BFD_ROAR_TIMER] = BOSSFD_GRAB_ROAR_TIME;
                                this->work[BFD_MANE_EMBERS_TIMER] = BOSSFD_GRAB_ROAR_TIME;
                                break;
                        }
                    }
                }
            }
            break;
        case BOSSFD_FLY_HOLE:
            if ((this->timers[0] == 0) && (sqrtf(SQ(dx) + SQ(dy) + SQ(dz)) < 100.0f)) {
                this->work[BFD_ACTION_STATE] = BOSSFD_BURROW;
                this->targetPosition.y = sHoleLocations[this->holeIndex].y - 70.0f;
                this->fwork[BFD_TURN_RATE_MAX] = 10000.0f;
                this->fwork[BFD_FLY_WOBBLE_AMP] = 0.0f;
                this->timers[0] = 150;
                this->work[BFD_ROAR_TIMER] = 40;
                this->holePosition.x = this->targetPosition.x;
                this->holePosition.z = this->targetPosition.z;
            }
            break;
        case BOSSFD_BURROW:
            sp1CF = true;
            if (this->timers[0] == 0) {
                this->actionFunc = BossFd_Wait;
                this->handoffSignal = FD2_SIGNAL_GROUND;
            }
            break;
        case BOSSFD_EMERGE:
            if ((this->timers[0] == 0) && (sqrtf(SQ(dx) + SQ(dy) + SQ(dz)) < 100.0f)) {
                this->actor.world.pos = this->targetPosition;
                this->work[BFD_ACTION_STATE] = BOSSFD_FLY_MAIN;
                this->actor.world.rot.x = 0x4000;
                this->targetPosition.y = sHoleLocations[this->holeIndex].y + 200.0f;
                this->timers[4] = 80;
                this->fwork[BFD_TURN_RATE_MAX] = 1000.0f;
                this->fwork[BFD_FLY_WOBBLE_AMP] = 0.0f;
                this->holePosition.x = this->targetPosition.x;
                this->holePosition.z = this->targetPosition.z;

                Actor_RequestQuakeWithSpeed(play, 1, 0x50, 0x5000);
                if (this->introState != BFD_CS_NONE) {
                    this->timers[0] = 50;
                } else {
                    this->timers[0] = 50;
                }
            }
            break;
        case BOSSFD_FLY_ROCKFALL:
            sp1CF = true;
            if (((this->work[BFD_DAMAGE_FLASH_TIMER] != 0) || (player->actor.world.pos.y < 70.0f)) &&
                (this->attackPhase != BOSSFD_ROCKFALL_RECOVER)) {
                BossFd_SetupRockfallRecovery(this);
            }

            switch (this->attackPhase) {
                case BOSSFD_ROCKFALL_CLIMB:
                    this->targetPosition.x = 0.0f;
                    this->targetPosition.y = BOSSFD_ROCKFALL_STAGING_HEIGHT;
                    this->targetPosition.z = -220.0f;
                    this->fwork[BFD_FLY_SPEED] = 10.0f;
                    this->fwork[BFD_TURN_RATE_MAX] = 3200.0f;
                    this->fwork[BFD_FLY_WOBBLE_AMP] = 60.0f;
                    if ((this->actor.world.pos.y >= (BOSSFD_ROCKFALL_STAGING_HEIGHT - 55.0f)) ||
                        (this->timers[0] == 0)) {
                        this->attackPhase = BOSSFD_ROCKFALL_WINDUP;
                        this->timers[0] = BOSSFD_ROCKFALL_WINDUP_TIME;
                        this->work[BFD_ROAR_TIMER] = BOSSFD_ROCKFALL_WINDUP_TIME;
                        this->work[BFD_MANE_EMBERS_TIMER] = BOSSFD_ROCKFALL_WINDUP_TIME;
                    }
                    break;
                case BOSSFD_ROCKFALL_WINDUP:
                    this->targetPosition.x = 0.0f;
                    this->targetPosition.y = BOSSFD_ROCKFALL_STAGING_HEIGHT;
                    this->targetPosition.z = 0.0f;
                    this->fwork[BFD_FLY_SPEED] = 3.0f;
                    this->fwork[BFD_TURN_RATE_MAX] = 3600.0f;
                    this->fwork[BFD_FLY_WOBBLE_AMP] = 20.0f;
                    if (this->timers[0] == 0) {
                        this->attackPhase = BOSSFD_ROCKFALL_CHARGE;
                        this->timers[0] = BOSSFD_ROCKFALL_CHARGE_TIME;
                    }
                    break;
                case BOSSFD_ROCKFALL_CHARGE:
                    this->targetPosition.x = 0.0f;
                    this->targetPosition.y = 1400.0f;
                    this->targetPosition.z = 0.0f;
                    this->fwork[BFD_FLY_SPEED] = 14.0f;
                    this->fwork[BFD_TURN_RATE_MAX] = 4200.0f;
                    this->fwork[BFD_FLY_WOBBLE_AMP] = 0.0f;
                    Actor_UpdateBgCheckInfo(play, &this->actor, 50.0f, 50.0f, 100.0f, 2);
                    if ((this->actor.bgCheckFlags & 0x10) || (this->timers[0] == 0)) {
                        BossFd_BeginRockfallBarrage(this, play);
                    }
                    break;
                case BOSSFD_ROCKFALL_BARRAGE:
                    this->targetPosition.x = sCeilingTargets[this->work[BFD_CEILING_TARGET]].x;
                    this->targetPosition.y = BOSSFD_ROCKFALL_BARRAGE_HEIGHT;
                    this->targetPosition.z = sCeilingTargets[this->work[BFD_CEILING_TARGET]].z;
                    this->fwork[BFD_FLY_SPEED] = 9.0f;
                    this->fwork[BFD_TURN_RATE_MAX] = 3600.0f;
                    this->fwork[BFD_FLY_WOBBLE_AMP] = 70.0f;
                    if (this->fwork[BFD_CEILING_BOUNCE] != 0.0f) {
                        pitchToTarget = this->fwork[BFD_CEILING_BOUNCE];
                        Math_ApproachZeroF(&this->fwork[BFD_CEILING_BOUNCE], 1.0f, 1000.0f);
                    }
                    if (this->timers[0] == 0) {
                        BossFd_SetupRockfallRecovery(this);
                    } else if ((this->timers[1] == 0) &&
                               !BossFd_TrySpawnRockfallWave(this, play, player)) {
                        this->timers[1] = BOSSFD_ROCKFALL_RETRY_TIME;
                    }
                    break;
                case BOSSFD_ROCKFALL_RECOVER:
                default:
                    this->targetPosition.x = 0.0f;
                    this->targetPosition.y = 360.0f;
                    this->targetPosition.z = 0.0f;
                    this->fwork[BFD_FLY_SPEED] = 10.0f;
                    this->fwork[BFD_TURN_RATE_MAX] = 2600.0f;
                    this->fwork[BFD_FLY_WOBBLE_AMP] = 50.0f;
                    if (this->timers[0] == 0) {
                        this->attackPhase = 0;
                        this->attackStep = 0;
                        this->work[BFD_ACTION_STATE] = BOSSFD_FLY_MAIN;
                        this->work[BFD_START_ATTACK] = false;
                        this->work[BFD_ROCK_TIMER] = 0;
                    }
                    break;
            }
            break;
        case BOSSFD_FLY_LEGACY_DROP_ROCKS:
        case BOSSFD_FLY_LEGACY_METEOR:
            BossFd_SetupRockfall(this);
            break;
        case BOSSFD_FLY_CHASE:
            this->actor.flags |= ACTOR_FLAG_SFX_FOR_PLAYER_BODY_HIT;
            this->fwork[BFD_FLY_SPEED] = aggressiveTuning ? 9.0f : 7.0f;
            this->targetPosition.x = player->actor.world.pos.x;
            this->targetPosition.y = player->actor.world.pos.y + 30.0f;
            this->targetPosition.z = player->actor.world.pos.z;
            this->fwork[BFD_FLY_WOBBLE_AMP] = 0.0f;
            if ((this->timers[0] > 0) && ((this->timers[0] % 64) == 0) && (this->timers[0] < 450)) {
                this->work[BFD_ROAR_TIMER] = 40;
                if (BossFd_IsFacingLink(this)) {
                    this->fireBreathTimer = 30;
                }
            }
            if ((this->work[BFD_DAMAGE_FLASH_TIMER] != 0) || (this->timers[0] == 0) ||
                (player->actor.world.pos.y < 70.0f)) {
                this->work[BFD_ACTION_STATE] = BOSSFD_FLY_MAIN;
                this->timers[0] = 0;
                this->fireBreathTimer = 0;
                this->work[BFD_START_ATTACK] = false;
            }
            break;
        case BOSSFD_FLY_GRAB: {
            Vec3f stageTarget;
            Vec3f bitePoint;
            f32 distanceToStage;
            f32 alignDx;
            f32 alignDy;
            f32 alignDz;
            s32 alignYawError;

            sp1CF = true;
            this->work[BFD_STOP_FLAG] = false;
            if ((this->work[BFD_DAMAGE_FLASH_TIMER] != 0) && (this->attackPhase != BOSSFD_GRAB_RECOVER)) {
                if (this->grabbingPlayer) {
                    BossFd_ReleasePlayer(this, play);
                }
                BossFd_SetupGrabRecovery(this);
            } else if (!this->grabbingPlayer && (player->actor.world.pos.y < 70.0f)) {
                BossFd_SetupGrabRecovery(this);
            }
            switch (this->attackPhase) {
                case BOSSFD_GRAB_STAGE:
                    stageTarget = BossFd_GetGrabStageTarget(this, player);
                    this->targetPosition = stageTarget;
                    this->fwork[BFD_FLY_SPEED] = 7.0f;
                    this->fwork[BFD_TURN_RATE_MAX] = 2600.0f;
                    this->fwork[BFD_FLY_WOBBLE_AMP] = 30.0f;
                    distanceToStage = sqrtf(SQ(stageTarget.x - this->actor.world.pos.x) +
                                            SQ(stageTarget.z - this->actor.world.pos.z));
                    if (((this->timers[1] == 0) && (distanceToStage < BOSSFD_GRAB_STAGE_RANGE) &&
                         (fabsf(stageTarget.y - this->actor.world.pos.y) < BOSSFD_GRAB_STAGE_VERTICAL_RANGE)) ||
                        (this->timers[0] == 0)) {
                        BossFd_BeginGrabAlign(this, player);
                    }
                    break;
                case BOSSFD_GRAB_ALIGN:
                    bitePoint = BossFd_GetPlayerBitePoint(player);
                    alignDx = bitePoint.x - this->actor.world.pos.x;
                    alignDy = bitePoint.y - this->actor.world.pos.y;
                    alignDz = bitePoint.z - this->actor.world.pos.z;
                    this->targetPosition = bitePoint;
                    this->fwork[BFD_FLY_SPEED] = BOSSFD_GRAB_ALIGN_SPEED;
                    this->fwork[BFD_TURN_RATE_MAX] = 4000.0f;
                    this->fwork[BFD_FLY_WOBBLE_AMP] = 0.0f;
                    angleToTarget = (s16)(Math_FAtan2F(alignDx, alignDz) * (0x8000 / M_PI));
                    pitchToTarget =
                        (s16)(Math_FAtan2F(alignDy, sqrtf(SQ(alignDx) + SQ(alignDz))) * (0x8000 / M_PI));
                    alignYawError = (s16)(angleToTarget - this->actor.world.rot.y);
                    if (alignYawError < 0) {
                        alignYawError = -alignYawError;
                    }
                    if ((this->timers[1] == 0) && (alignYawError < BOSSFD_GRAB_ALIGN_YAW_TOLERANCE)) {
                        BossFd_BeginGrabLunge(this, player);
                    } else if (this->timers[0] == 0) {
                        BossFd_SetupGrabRecovery(this);
                    }
                    break;
                case BOSSFD_GRAB_LUNGE:
                    if (this->timers[0] > (BOSSFD_GRAB_LUNGE_TIME - BOSSFD_GRAB_LUNGE_TRACK_TIME)) {
                        BossFd_UpdateGrabLungeTarget(this, player);
                    }
                    this->targetPosition = this->attackTarget;
                    this->fwork[BFD_FLY_SPEED] = BOSSFD_GRAB_LUNGE_SPEED;
                    this->fwork[BFD_TURN_RATE_MAX] = 1800.0f;
                    this->fwork[BFD_FLY_WOBBLE_AMP] = 0.0f;
                    this->grabTimer = this->timers[0];
                    break;
                case BOSSFD_GRAB_ASCEND:
                    if (!this->grabbingPlayer) {
                        BossFd_SetupGrabRecovery(this);
                        break;
                    }
                    this->targetPosition = this->attackTarget;
                    Math_ApproachF(&this->actor.speedXZ, BOSSFD_GRAB_ASCENT_SPEED, 1.0f, 1.0f);
                    this->fwork[BFD_FLY_SPEED] = BOSSFD_GRAB_ASCENT_SPEED;
                    this->fwork[BFD_TURN_RATE_MAX] = 1000.0f;
                    this->fwork[BFD_FLY_WOBBLE_AMP] = 10.0f;
                    this->grabTimer = this->timers[0];
                    break;
                case BOSSFD_GRAB_RECOVER:
                default:
                    this->targetPosition = this->attackTarget;
                    this->fwork[BFD_FLY_SPEED] = BOSSFD_GRAB_RECOVERY_SPEED;
                    this->fwork[BFD_TURN_RATE_MAX] = 800.0f;
                    this->fwork[BFD_FLY_WOBBLE_AMP] = 25.0f;
                    if (this->timers[0] == 0) {
                        this->attackPhase = 0;
                        this->attackStep = 0;
                        this->work[BFD_ACTION_STATE] = BOSSFD_FLY_MAIN;
                        this->work[BFD_START_ATTACK] = false;
                        this->grabTransitionedThisFrame = true;
                    }
                    break;
            }
            break;
        }
        case BOSSFD_FLY_UNUSED_103:
            this->fireBreathTimer = 0;
            this->attackPhase = 0;
            this->attackStep = 0;
            this->work[BFD_ACTION_STATE] = BOSSFD_FLY_MAIN;
            this->work[BFD_START_ATTACK] = false;
            this->timers[0] = 0;
            break;
        case BOSSFD_FLY_LOW_CIRCLE: {
            Vec3f circleCenter = sHoleLocations[1];
            f32 radius = BOSSFD_LOW_CIRCLE_RADIUS_DEFAULT;
            f32 angleRadians = (this->work[BOSSFD_LOW_CIRCLE_ANGLE_IDX] / (f32)0x8000) * M_PI;
            f32 distanceToTarget;
            f32 angularStep;
            s16 angleStep;

            sp1CF = true;
            if ((this->fogMode != BOSSFD_FOG_MODE_ERUPTION_IN) && (this->fogMode != BOSSFD_FOG_MODE_ERUPTION)) {
                this->fogMode = BOSSFD_FOG_MODE_ERUPTION_IN;
                play->envCtx.blendIndoorLights = true;
                play->envCtx.unk_BE = play->envCtx.unk_BD;
                play->envCtx.unk_BD = 3;
                play->envCtx.unk_D8 = 0.0f;
                play->envCtx.unk_DC = 0;
            }
            radius = CLAMP_MIN(radius, 120.0f);
            this->fwork[BFD_FLY_SPEED] = BOSSFD_LOW_CIRCLE_SPEED_DEFAULT;
            this->fwork[BFD_FLY_WOBBLE_AMP] =
                (this->attackPhase == BOSSFD_LOW_CIRCLE_RISE) ? 0.0f : 60.0f;
            this->fwork[BFD_TURN_RATE_MAX] = 1600.0f;

            if ((this->attackPhase == BOSSFD_LOW_CIRCLE_RISE) &&
                (this->actor.world.pos.y >= BOSSFD_LOW_CIRCLE_SAFE_HEIGHT)) {
                this->attackPhase = BOSSFD_LOW_CIRCLE_ENTER;
                this->timers[1] = BOSSFD_LOW_CIRCLE_ENTRY_TIME;
            }

            if (this->attackPhase == BOSSFD_LOW_CIRCLE_RISE) {
                this->targetPosition = this->attackTarget;
            } else {
                this->targetPosition.x = circleCenter.x + sinf(angleRadians) * radius;
                this->targetPosition.z = circleCenter.z + cosf(angleRadians) * radius;
                this->targetPosition.y = BOSSFD_LOW_CIRCLE_SAFE_HEIGHT;
            }

            if (this->attackPhase == BOSSFD_LOW_CIRCLE_ENTER) {
                distanceToTarget = sqrtf(SQ(this->targetPosition.x - this->actor.world.pos.x) +
                                         SQ(this->targetPosition.z - this->actor.world.pos.z));
                if (((distanceToTarget < 55.0f) &&
                     (fabsf(this->actor.world.pos.y - BOSSFD_LOW_CIRCLE_SAFE_HEIGHT) < 25.0f)) ||
                    (this->timers[1] == 0)) {
                    this->attackPhase = BOSSFD_LOW_CIRCLE_ORBIT;
                    this->work[BOSSFD_LOW_CIRCLE_ANGLE_IDX] =
                        (s16)(Math_FAtan2F(this->actor.world.pos.x - circleCenter.x,
                                          this->actor.world.pos.z - circleCenter.z) *
                              (0x8000 / M_PI));
                    this->timers[0] = BOSSFD_LOW_CIRCLE_TIMER;
                    this->work[BFD_ROCK_TIMER] = BOSSFD_LOW_CIRCLE_ROCK_TIMER;
                }
            } else if (this->attackPhase == BOSSFD_LOW_CIRCLE_ORBIT) {
                angularStep = (BOSSFD_LOW_CIRCLE_SPEED_DEFAULT / radius) * (0x8000 / M_PI);
                angleStep = (s16)CLAMP(angularStep, 0x20, 0x600);
                this->work[BOSSFD_LOW_CIRCLE_ANGLE_IDX] +=
                    this->work[BOSSFD_LOW_CIRCLE_DIR_IDX] * angleStep;

                if ((this->fireBreathTimer == 0) && (this->timers[0] > 0) &&
                    ((this->timers[0] % BOSSFD_LOW_CIRCLE_FIRE_INTERVAL) == 0) && BossFd_IsFacingLink(this)) {
                    this->fireBreathTimer = 26;
                }
            }
            if (((this->attackPhase == BOSSFD_LOW_CIRCLE_ORBIT) && (this->timers[0] == 0)) ||
                (this->work[BFD_DAMAGE_FLASH_TIMER] != 0)) {
                this->work[BFD_ACTION_STATE] = BOSSFD_FLY_MAIN;
                this->timers[0] = 0;
                this->timers[1] = 0;
                this->fireBreathTimer = 0;
                this->attackStep = 0;
            }
            break;
        }
        case BOSSFD_DEATH_START:
            if (sqrtf(SQ(dx) + SQ(dz)) < 50.0f) {
                this->timers[0] = 0;
            }
            if (this->timers[0] == 0) {
                this->timers[0] = (s16)Rand_ZeroFloat(10.0f) + 10;
                do {
                    this->targetPosition.x = Rand_CenteredFloat(200.0f);
                    this->targetPosition.y = 390.0f;
                    this->targetPosition.z = Rand_CenteredFloat(200.0f);
                    temp_x = this->targetPosition.x - this->actor.world.pos.x;
                    temp_z = this->targetPosition.z - this->actor.world.pos.z;
                } while (!(sqrtf(SQ(temp_x) + SQ(temp_z)) > 100.0f));
            }
            this->fwork[BFD_FLY_WOBBLE_AMP] = 200.0f;
            this->fwork[BFD_FLY_WOBBLE_RATE] = 1000.0f;
            this->fwork[BFD_TURN_RATE_MAX] = 10000.0f;
            Math_ApproachF(&this->fwork[BFD_BODY_PULSE], 0.3f, 1.0f, 0.05f);
            if (this->timers[1] == 0) {
                this->work[BFD_ACTION_STATE] = BOSSFD_SKIN_BURN;
                this->timers[0] = 30;
            }
            break;
        case BOSSFD_SKIN_BURN:
            this->targetPosition.x = 0.0f;
            this->targetPosition.y = 390.0f;
            this->targetPosition.z = 0.0f;
            this->fwork[BFD_FLY_WOBBLE_AMP] = 200.0f;
            this->fwork[BFD_FLY_WOBBLE_RATE] = 1000.0f;
            this->fwork[BFD_TURN_RATE_MAX] = 2000.0f;
            Math_ApproachF(&this->fwork[BFD_BODY_PULSE], 0.3f, 1.0f, 0.05f);
            if ((this->timers[0] == 0) && ((this->work[BFD_MOVE_TIMER] % 4) == 0)) {
                if (this->skinSegments != 0) {
                    this->skinSegments--;
                    if (this->skinSegments == 0) {
                        Audio_QueueSeqCmd(SEQ_PLAYER_BGM_MAIN << 24 | NA_BGM_BOSS_CLEAR);
                    }
                } else {
                    this->work[BFD_ACTION_STATE] = BOSSFD_BONES_FALL;
                    this->timers[0] = 30;
                }
            }
            if ((this->work[BFD_MOVE_TIMER] % 32) == 0) {
                this->work[BFD_ROAR_TIMER] = 40;
            }

            if (this->skinSegments != 0) {
                Vec3f sp188;
                Vec3f sp17C = { 0.0f, 0.0f, 0.0f };
                Vec3f sp170;
                Vec3f sp164 = { 0.0f, 0.03f, 0.0f };
                Vec3f sp158;
                f32 pad154;
                s16 temp_rand2;
                s16 sp150;

                if (this->fogMode == 0) {
                    play->envCtx.unk_D8 = 0;
                }
                this->fogMode = 0xA;

                sp150 = 1;
                if (this->work[BFD_MOVE_TIMER] & 0x1C) {
                    Audio_PlaySoundGeneral(NA_SE_EN_VALVAISA_BURN - SFX_FLAG, &this->actor.projectedPos, 4,
                                           &gSfxDefaultFreqAndVolScale, &gSfxDefaultFreqAndVolScale,
                                           &gSfxDefaultReverb);
                }
                for (i1 = 0; i1 < sp150; i1++) {
                    if (sp150) { // Needed for matching
                        temp_rand2 = Rand_ZeroFloat(99.9f);

                        sp188.x = this->bodySegsPos[temp_rand2].x;
                        sp188.y = this->bodySegsPos[temp_rand2].y - 10.0f;
                        sp188.z = this->bodySegsPos[temp_rand2].z;

                        sp164.y = 0.03f;

                        EffectSsKFire_Spawn(play, &sp188, &sp17C, &sp164, (s16)Rand_ZeroFloat(20.0f) + 40, 0x64);

                        for (i2 = 0; i2 < 15; i2++) {
                            sp170.x = Rand_CenteredFloat(20.0f);
                            sp170.y = Rand_CenteredFloat(20.0f);
                            sp170.z = Rand_CenteredFloat(20.0f);

                            sp158.y = 0.4f;
                            sp158.x = Rand_CenteredFloat(0.5f);
                            sp158.z = Rand_CenteredFloat(0.5f);

                            BossFd_SpawnEmber(this->effects, &sp188, &sp170, &sp158, (s16)Rand_ZeroFloat(3.0f) + 8);
                        }
                    }
                }
            }
            break;
        case BOSSFD_BONES_FALL:
            this->work[BFD_STOP_FLAG] = true;
            this->fogMode = 3;
            if (this->timers[0] < 18) {
                this->bodyFallApart[this->timers[0]] = 1;
            }
            if (this->timers[0] == 0) {
                this->work[BFD_ACTION_STATE] = BOSSFD_SKULL_PAUSE;
                this->timers[0] = 15;
                this->work[BFD_CEILING_TARGET] = 0;
                player->actor.world.pos.y = 90.0f;
                player->actor.world.pos.x = 40.0f;
                player->actor.world.pos.z = 150.0f;
            }
            break;
        case BOSSFD_SKULL_PAUSE:
            if (this->timers[0] == 0) {
                this->work[BFD_ACTION_STATE] = BOSSFD_SKULL_FALL;
                this->timers[0] = 20;
                this->work[BFD_STOP_FLAG] = false;
            }
            break;
        case BOSSFD_SKULL_FALL:
            this->fwork[BFD_TURN_RATE] = this->fwork[BFD_TURN_RATE_MAX] = this->actor.speedXZ =
                this->fwork[BFD_FLY_SPEED] = 0;

            if (this->timers[0] == 1) {
                this->actor.world.pos.x = 0;
                this->actor.world.pos.y = 900.0f;
                this->actor.world.pos.z = 150.0f;
                this->actor.world.rot.x = this->actor.world.rot.y = 0;
                this->actor.shape.rot.z = 0x1200;
                this->actor.velocity.x = 0;
                this->actor.velocity.z = 0;
            }
            if (this->timers[0] == 0) {
                if (this->actor.world.pos.y <= 110.0f) {
                    this->actor.world.pos.y = 110.0f;
                    this->actor.velocity.y = 0;
                    if (this->work[BFD_CEILING_TARGET] == 0) {
                        this->work[BFD_CEILING_TARGET]++;
                        this->timers[1] = 60;
                        this->work[BFD_CAM_SHAKE_TIMER] = 20;
                        Audio_PlaySoundGeneral(NA_SE_EN_VALVAISA_LAND2, &this->actor.projectedPos, 4,
                                               &gSfxDefaultFreqAndVolScale, &gSfxDefaultFreqAndVolScale,
                                               &gSfxDefaultReverb);
                        Player_SetCsActionWithHaltedActors(play, &this->actor, 5);
                        for (i1 = 0; i1 < 15; i1++) {
                            Vec3f sp144 = { 0.0f, 0.0f, 0.0f };
                            Vec3f sp138 = { 0.0f, 0.0f, 0.0f };
                            Vec3f sp12C;

                            sp144.x = Rand_CenteredFloat(8.0f);
                            sp144.y = Rand_ZeroFloat(1.0f);
                            sp144.z = Rand_CenteredFloat(8.0f);

                            sp138.y = 0.3f;

                            sp12C.x = Rand_CenteredFloat(10.0f) + this->actor.world.pos.x;
                            sp12C.y = Rand_CenteredFloat(10.0f) + this->actor.world.pos.y;
                            sp12C.z = Rand_CenteredFloat(10.0f) + this->actor.world.pos.z;
                            BossFd_SpawnDust(this->effects, &sp12C, &sp144, &sp138, Rand_ZeroFloat(100.0f) + 300);
                        }
                    }
                } else {
                    this->actor.velocity.y -= 1.0f;
                }
            } else {
                this->actor.velocity.y = 0;
            }
            if (this->timers[1] == 1) {
                this->work[BFD_ACTION_STATE] = BOSSFD_SKULL_BURN;
                this->timers[0] = 70;
            }
            break;
        case BOSSFD_SKULL_BURN:
            this->actor.velocity.y = 0.0f;
            this->actor.world.pos.y = 110.0f;
            this->fwork[BFD_TURN_RATE] = this->fwork[BFD_TURN_RATE_MAX] = this->actor.speedXZ =
                this->fwork[BFD_FLY_SPEED] = 0.0f;

            if ((50 > this->timers[0]) && (this->timers[0] > 0)) {
                Vec3f sp120;
                Vec3f sp114 = { 0.0f, 0.0f, 0.0f };
                Vec3f sp108 = { 0.0f, 0.03f, 0.0f };

                Audio_PlaySoundGeneral(NA_SE_EN_GOMA_LAST - SFX_FLAG, &this->actor.projectedPos, 4,
                                       &gSfxDefaultFreqAndVolScale, &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);

                sp120.x = Rand_CenteredFloat(40.0f) + this->actor.world.pos.x;
                sp120.y = (Rand_CenteredFloat(10.0f) + this->actor.world.pos.y) - 10.0f;
                sp120.z = (Rand_CenteredFloat(40.0f) + this->actor.world.pos.z) + 5.0f;

                sp108.y = 0.03f;

                EffectSsKFire_Spawn(play, &sp120, &sp114, &sp108, (s16)Rand_ZeroFloat(15.0f) + 30, 0);
            }
            if (this->timers[0] < 20) {
                Math_ApproachZeroF(&this->actor.scale.x, 1.0f, 0.0025f);
                Actor_SetScale(&this->actor, this->actor.scale.x);
            }
            if (this->timers[0] == 0) {
                this->actionFunc = BossFd_Wait;
                this->actor.world.pos.y -= 1000.0f;
            }
            if (GameInteractor_Should(VB_SPAWN_HEART_CONTAINER, this->timers[0] == 7)) {
                Actor_Spawn(&play->actorCtx, play, ACTOR_ITEM_B_HEART, this->actor.world.pos.x, this->actor.world.pos.y,
                            this->actor.world.pos.z, 0, 0, 0, 0);
            }
            break;
        case BOSSFD_WAIT_INTRO:
            break;
    }

    //                                 Update body segments and mane

    if (!this->work[BFD_STOP_FLAG]) {
        s16 i4;
        Vec3f spE0[3];
        Vec3f spBC[3];
        f32 phi_f20;
        f32 padB4;
        f32 padB0;
        f32 padAC;
        f32 minFlyHeight = 110.0f;
        bool enforceLowCircleHeight = (this->work[BFD_ACTION_STATE] == BOSSFD_FLY_LOW_CIRCLE) &&
                                      (this->attackPhase != BOSSFD_LOW_CIRCLE_RISE);

        Math_ApproachS(&this->actor.world.rot.y, angleToTarget, 0xA, this->fwork[BFD_TURN_RATE]);

        if (this->work[BFD_ACTION_STATE] == BOSSFD_FLY_LOW_CIRCLE) {
            minFlyHeight = BOSSFD_LOW_CIRCLE_SAFE_HEIGHT;
        }

        if (((this->work[BFD_ACTION_STATE] == BOSSFD_FLY_CHASE) ||
             (this->work[BFD_ACTION_STATE] == BOSSFD_FLY_LOW_CIRCLE)) &&
            (this->actor.world.pos.y < minFlyHeight) && (pitchToTarget < 0)) {
            pitchToTarget = 0;
            Math_ApproachF(&this->actor.world.pos.y, minFlyHeight, 1.0f, 5.0f);
        }

        Math_ApproachS(&this->actor.world.rot.x, pitchToTarget, 0xA, this->fwork[BFD_TURN_RATE]);
        Math_ApproachF(&this->fwork[BFD_TURN_RATE], this->fwork[BFD_TURN_RATE_MAX], 1.0f, 20000.0f);
        Math_ApproachF(&this->actor.speedXZ, this->fwork[BFD_FLY_SPEED], 1.0f, 0.1f);
        if (this->work[BFD_ACTION_STATE] < BOSSFD_SKULL_FALL) {
            Actor_UpdateVelocityXYZ(&this->actor);
        }
        Actor_UpdatePos(&this->actor);
        if (enforceLowCircleHeight && (this->actor.world.pos.y < BOSSFD_LOW_CIRCLE_SAFE_HEIGHT)) {
            this->actor.world.pos.y = BOSSFD_LOW_CIRCLE_SAFE_HEIGHT;
            this->actor.velocity.y = CLAMP_MIN(this->actor.velocity.y, 0.0f);
        }

        this->work[BFD_LEAD_BODY_SEG]++;
        if (this->work[BFD_LEAD_BODY_SEG] >= 100) {
            this->work[BFD_LEAD_BODY_SEG] = 0;
        }
        i4 = this->work[BFD_LEAD_BODY_SEG];
        this->bodySegsPos[i4].x = this->actor.world.pos.x;
        this->bodySegsPos[i4].y = this->actor.world.pos.y;
        this->bodySegsPos[i4].z = this->actor.world.pos.z;
        this->bodySegsRot[i4].x = (this->actor.world.rot.x / (f32)0x8000) * M_PI;
        this->bodySegsRot[i4].y = (this->actor.world.rot.y / (f32)0x8000) * M_PI;
        this->bodySegsRot[i4].z = (this->actor.world.rot.z / (f32)0x8000) * M_PI;

        this->work[BFD_LEAD_MANE_SEG]++;
        if (this->work[BFD_LEAD_MANE_SEG] >= 30) {
            this->work[BFD_LEAD_MANE_SEG] = 0;
        }
        i4 = this->work[BFD_LEAD_MANE_SEG];
        this->centerMane.scale[i4] = (Math_SinS(this->work[BFD_MOVE_TIMER] * 5596.0f) * 0.3f) + 1.0f;
        this->rightMane.scale[i4] = (Math_SinS(this->work[BFD_MOVE_TIMER] * 5496.0f) * 0.3f) + 1.0f;
        this->leftMane.scale[i4] = (Math_CosS(this->work[BFD_MOVE_TIMER] * 5696.0f) * 0.3f) + 1.0f;
        this->centerMane.pos[i4] = this->centerMane.head;
        this->fireManeRot[i4].x = (this->actor.world.rot.x / (f32)0x8000) * M_PI;
        this->fireManeRot[i4].y = (this->actor.world.rot.y / (f32)0x8000) * M_PI;
        this->fireManeRot[i4].z = (this->actor.world.rot.z / (f32)0x8000) * M_PI;
        this->rightMane.pos[i4] = this->rightMane.head;
        this->leftMane.pos[i4] = this->leftMane.head;

        if ((0x3000 > this->actor.world.rot.x) && (this->actor.world.rot.x > -0x3000)) {
            Math_ApproachF(&this->flattenMane, 1.0f, 1.0f, 0.05f);
        } else {
            Math_ApproachF(&this->flattenMane, 0.5f, 1.0f, 0.05f);
        }

        if (this->work[BFD_ACTION_STATE] < BOSSFD_SKULL_FALL) {
            if ((this->actor.prevPos.y < 90.0f) && (90.0f <= this->actor.world.pos.y)) {
                this->timers[4] = 80;
                Actor_RequestQuakeWithSpeed(play, 1, 80, 0x5000);
                this->work[BFD_ROAR_TIMER] = 40;
                this->work[BFD_MANE_EMBERS_TIMER] = 30;
                this->work[BFD_SPLASH_TIMER] = 10;
            }
            if ((this->actor.prevPos.y > 90.0f) && (90.0f >= this->actor.world.pos.y)) {
                this->timers[4] = 80;
                Actor_RequestQuakeWithSpeed(play, 1, 80, 0x5000);
                this->work[BFD_MANE_EMBERS_TIMER] = 30;
                this->work[BFD_SPLASH_TIMER] = 10;
            }
        }

        if (!sp1CF) {
            spE0[0].x = spE0[0].y = Math_SinS(this->work[BFD_MOVE_TIMER] * 1500.0f) * 3000.0f;
            spE0[1].x = Math_SinS(this->work[BFD_MOVE_TIMER] * 2000.0f) * 4000.0f;
            spE0[1].y = Math_SinS(this->work[BFD_MOVE_TIMER] * 2200.0f) * 4000.0f;
            spE0[2].x = Math_SinS(this->work[BFD_MOVE_TIMER] * 1700.0f) * 2000.0f;
            spE0[2].y = Math_SinS(this->work[BFD_MOVE_TIMER] * 1900.0f) * 2000.0f;
            spBC[0].x = spBC[0].y = Math_SinS(this->work[BFD_MOVE_TIMER] * 1500.0f) * -3000.0f;
            spBC[1].x = Math_SinS(this->work[BFD_MOVE_TIMER] * 2200.0f) * -4000.0f;
            spBC[1].y = Math_SinS(this->work[BFD_MOVE_TIMER] * 2000.0f) * -4000.0f;
            spBC[2].x = Math_SinS(this->work[BFD_MOVE_TIMER] * 1900.0f) * -2000.0f;
            spBC[2].y = Math_SinS(this->work[BFD_MOVE_TIMER] * 1700.0f) * -2000.0f;

            for (i3 = 0; i3 < 3; i3++) {
                Math_ApproachF(&this->rightArmRot[i3].x, spE0[i3].x, 1.0f, 1000.0f);
                Math_ApproachF(&this->rightArmRot[i3].y, spE0[i3].y, 1.0f, 1000.0f);
                Math_ApproachF(&this->leftArmRot[i3].x, spBC[i3].x, 1.0f, 1000.0f);
                Math_ApproachF(&this->leftArmRot[i3].y, spBC[i3].y, 1.0f, 1000.0f);
            }
        } else {
            for (i2 = 0; i2 < 3; i2++) {
                phi_f20 = 0.0f;
                Math_ApproachZeroF(&this->rightArmRot[i2].y, 0.1f, 100.0f);
                Math_ApproachZeroF(&this->leftArmRot[i2].y, 0.1f, 100.0f);
                if (i2 == 0) {
                    phi_f20 = -3000.0f;
                }
                Math_ApproachF(&this->rightArmRot[i2].x, phi_f20, 0.1f, 100.0f);
                Math_ApproachF(&this->leftArmRot[i2].x, -phi_f20, 0.1f, 100.0f);
            }
        }

        if (this->work[BFD_ACTION_STATE] == BOSSFD_FLY_GRAB) {
            f32 reachRoot = -2600.0f;
            f32 reachMid = -1600.0f;
            f32 reachRoll = 1000.0f;

            if (this->attackPhase == BOSSFD_GRAB_LUNGE) {
                reachRoot = -3600.0f;
                reachMid = -2400.0f;
                reachRoll = 1400.0f;
            } else if ((this->attackPhase == BOSSFD_GRAB_ASCEND) && this->grabbingPlayer) {
                reachRoot = -1800.0f;
                reachMid = -1100.0f;
                reachRoll = 700.0f;
            }

            Math_ApproachF(&this->rightArmRot[0].x, reachRoot, 0.2f, 300.0f);
            Math_ApproachF(&this->rightArmRot[1].x, reachMid, 0.2f, 260.0f);
            Math_ApproachF(&this->rightArmRot[2].x, reachMid * 0.5f, 0.2f, 220.0f);
            Math_ApproachF(&this->rightArmRot[1].y, reachRoll, 0.2f, 220.0f);
            Math_ApproachF(&this->rightArmRot[2].y, reachRoll * 0.5f, 0.2f, 200.0f);

            Math_ApproachF(&this->leftArmRot[0].x, -reachRoot, 0.2f, 300.0f);
            Math_ApproachF(&this->leftArmRot[1].x, -reachMid, 0.2f, 260.0f);
            Math_ApproachF(&this->leftArmRot[2].x, -reachMid * 0.5f, 0.2f, 220.0f);
            Math_ApproachF(&this->leftArmRot[1].y, -reachRoll, 0.2f, 220.0f);
            Math_ApproachF(&this->leftArmRot[2].y, -reachRoll * 0.5f, 0.2f, 200.0f);
        }
    }
    BossFd_UpdateGrabAfterMovement(this, play, player);
}

void BossFd_Wait(BossFd* this, PlayState* play) {
    if (this->groundSpawnRetryTimer > 0) {
        this->groundSpawnRetryTimer--;
    }
    if ((this->actor.colChkInfo.health > 0) && (this->introState == BFD_CS_NONE) &&
        (this->handoffSignal != FD2_SIGNAL_FLY) && (this->handoffSignal != FD2_SIGNAL_FLY_FROM_HOLE) &&
        (this->handoffSignal != FD2_SIGNAL_DEATH) &&
        (this->groundSpawnRetryTimer == 0) && !BossFd_HasGroundForm(play, this)) {
        Actor* groundForm = Actor_SpawnAsChild(&play->actorCtx, &this->actor, play, ACTOR_BOSS_FD2,
                                               this->actor.world.pos.x, this->actor.world.pos.y,
                                               this->actor.world.pos.z, 0, 0, 0, BFD_CS_NONE);

        this->groundSpawnRetryTimer = 30;
        if (groundForm != NULL) {
            this->handoffSignal = FD2_SIGNAL_NONE;
        }
    }

    if ((this->handoffSignal == FD2_SIGNAL_FLY) ||
        (this->handoffSignal == FD2_SIGNAL_FLY_FROM_HOLE)) { // Set by BossFd2
        u8 temp_rand;
        bool useForcedHole = this->handoffSignal == FD2_SIGNAL_FLY_FROM_HOLE;

        this->handoffSignal = FD2_SIGNAL_NONE;
        BossFd_SetupFly(this, play);
        if (!useForcedHole) {
            do {
                temp_rand = Rand_ZeroFloat(8.9f);
            } while (temp_rand == this->holeIndex);
            this->holeIndex = temp_rand;
        }
        this->targetPosition.x = sHoleLocations[this->holeIndex].x;
        this->targetPosition.y = sHoleLocations[this->holeIndex].y - 200.0f;
        this->targetPosition.z = sHoleLocations[this->holeIndex].z;
        BossFd_RelocateFlightHistory(this, &this->targetPosition);

        this->timers[0] = 10;
        this->work[BFD_ACTION_STATE] = BOSSFD_EMERGE;
        this->work[BFD_START_ATTACK] = true;
    }
    if (this->handoffSignal == FD2_SIGNAL_DEATH) {
        this->handoffSignal = FD2_SIGNAL_NONE;
        BossFd_SetupFly(this, play);
        this->holeIndex = 1;
        this->targetPosition.x = sHoleLocations[1].x;
        this->targetPosition.y = sHoleLocations[1].y - 200.0f;
        this->targetPosition.z = sHoleLocations[1].z;
        BossFd_RelocateFlightHistory(this, &this->targetPosition);
        this->timers[0] = 10;
        this->work[BFD_ACTION_STATE] = BOSSFD_EMERGE;
    }
}

static Vec3f sFireAudioVec = { 0.0f, 0.0f, 50.0f };

void BossFd_Effects(BossFd* this, PlayState* play) {
    static Color_RGBA8 colorYellow = { 255, 255, 0, 255 };
    static Color_RGBA8 colorRed = { 255, 10, 0, 255 };
    s16 breathOpacity = 0;
    f32 jawAngle;
    f32 jawSpeed;
    f32 emberRate;
    f32 emberSpeed;
    s16 eyeStates[] = { EYE_OPEN, EYE_HALF, EYE_CLOSED, EYE_CLOSED, EYE_HALF };
    f32 temp_x;
    f32 temp_z;
    s16 i;

    if (this->fogMode == 0) {
        play->envCtx.unk_BF = 0;
        play->envCtx.unk_D8 = 0.5f + 0.5f * Math_SinS(this->work[BFD_VAR_TIMER] * 0x500);
        play->envCtx.unk_DC = 2;
        play->envCtx.unk_BD = 1;
        play->envCtx.unk_BE = 0;
    } else if (this->fogMode == BOSSFD_FOG_MODE_ERUPTION_IN) {
        play->envCtx.unk_BF = 0;
        play->envCtx.unk_DC = 0;
        play->envCtx.unk_BD = 3;
        Math_ApproachF(&play->envCtx.unk_D8, 1.0f, 1.0f, 0.05f);
        if (play->envCtx.unk_D8 >= 0.99f) {
            this->fogMode = BOSSFD_FOG_MODE_ERUPTION;
        }
    } else if (this->fogMode == BOSSFD_FOG_MODE_ERUPTION) {
        play->envCtx.unk_BF = 0;
        play->envCtx.unk_DC = 0;
        play->envCtx.unk_BD = 3;
        Math_ApproachF(&play->envCtx.unk_D8,
                       0.9f + 0.05f * Math_SinS(this->work[BFD_VAR_TIMER] * BOSSFD_LOW_CIRCLE_FOG_PULSE_RATE), 1.0f,
                       0.02f);
    } else if (this->fogMode == BOSSFD_FOG_MODE_ERUPTION_OUT) {
        play->envCtx.unk_BF = 0;
        play->envCtx.unk_DC = 0;
        play->envCtx.unk_BE = 3;
        play->envCtx.unk_BD = 1;
        Math_ApproachF(&play->envCtx.unk_D8, 1.0f, 1.0f, 0.05f);
        if (play->envCtx.unk_D8 >= 0.99f) {
            this->fogMode = 1;
        }
    } else if (this->fogMode == 3) {
        play->envCtx.unk_BF = 0;
        play->envCtx.unk_DC = 2;
        play->envCtx.unk_BD = 2;
        play->envCtx.unk_BE = 0;
        Math_ApproachF(&play->envCtx.unk_D8, 1.0f, 1.0f, 0.05f);
    } else if (this->fogMode == 2) {
        this->fogMode--;
        play->envCtx.unk_BF = 0;
        Math_ApproachF(&play->envCtx.unk_D8, 0.55f + 0.05f * Math_SinS(this->work[BFD_VAR_TIMER] * 0x3E00), 1.0f,
                       0.15f);
        play->envCtx.unk_DC = 2;
        play->envCtx.unk_BD = 3;
        play->envCtx.unk_BE = 0;
    } else if (this->fogMode == 10) {
        this->fogMode = 1;
        play->envCtx.unk_BF = 0;
        Math_ApproachF(&play->envCtx.unk_D8, 0.21f + 0.07f * Math_SinS(this->work[BFD_VAR_TIMER] * 0xC00), 1.0f, 0.05f);
        play->envCtx.unk_DC = 2;
        play->envCtx.unk_BD = 3;
        play->envCtx.unk_BE = 0;
    } else if (this->fogMode == 1) {
        Math_ApproachF(&play->envCtx.unk_D8, 0.0f, 1.0f, 0.03f);
        if (play->envCtx.unk_D8 <= 0.01f) {
            this->fogMode = 0;
        }
    }

    if (this->work[BFD_MANE_EMBERS_TIMER] != 0) {
        this->work[BFD_MANE_EMBERS_TIMER]--;
        emberSpeed = emberRate = 20.0f;
    } else {
        emberRate = 3.0f;
        emberSpeed = 5.0f;
    }
    Math_ApproachF(&this->fwork[BFD_MANE_EMBER_RATE], emberRate, 1.0f, 0.1f);
    Math_ApproachF(&this->fwork[BFD_MANE_EMBER_SPEED], emberSpeed, 1.0f, 0.5f);

    if (((this->work[BFD_VAR_TIMER] % 8) == 0) && (Rand_ZeroOne() < 0.3f)) {
        this->work[BFD_BLINK_TIMER] = 4;
    }
    this->eyeState = eyeStates[this->work[BFD_BLINK_TIMER]];

    if (this->work[BFD_BLINK_TIMER] != 0) {
        this->work[BFD_BLINK_TIMER]--;
    }

    if ((this->work[BFD_ACTION_STATE] == BOSSFD_FLY_GRAB) && this->grabbingPlayer) {
        jawAngle = 2000.0f;
        jawSpeed = 2400.0f;
    } else if (this->work[BFD_ROAR_TIMER] != 0) {
        if (this->work[BFD_ROAR_TIMER] == 37) {
            Audio_PlaySoundGeneral(NA_SE_EN_VALVAISA_ROAR, &this->actor.projectedPos, 4, &gSfxDefaultFreqAndVolScale,
                                   &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
        }
        jawAngle = 6000.0f;
        jawSpeed = 1300.0f;
    } else if ((this->work[BFD_ACTION_STATE] == BOSSFD_FLY_GRAB) &&
               ((this->attackPhase == BOSSFD_GRAB_STAGE) || (this->attackPhase == BOSSFD_GRAB_ALIGN) ||
                (this->attackPhase == BOSSFD_GRAB_LUNGE))) {
        jawAngle = 7000.0f;
        jawSpeed = 1800.0f;
    } else {
        jawAngle = (this->work[BFD_VAR_TIMER] & 0x10) ? 0.0f : 1000.0f;
        jawSpeed = 500.0f;
    }
    Math_ApproachF(&this->jawOpening, jawAngle, 0.3f, jawSpeed);

    if (this->work[BFD_ROAR_TIMER] != 0) {
        this->work[BFD_ROAR_TIMER]--;
    }

    if (this->timers[4] != 0) {
        Vec3f spawnVel1;
        Vec3f spawnAccel1;
        Vec3f spawnPos1;
        s32 pad;

        Audio_PlaySoundGeneral(NA_SE_EN_VALVAISA_APPEAR - SFX_FLAG, &this->actor.projectedPos, 4,
                               &gSfxDefaultFreqAndVolScale, &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
        if (this->work[BFD_SPLASH_TIMER] != 0) {
            this->work[BFD_SPLASH_TIMER]--;
            if ((this->actor.colChkInfo.health == 0) ||
                ((this->introState == BFD_CS_EMERGE) && (this->actor.world.rot.x > 0x3000))) {
                if ((u8)this->fogMode == 0) {
                    play->envCtx.unk_D8 = 0.0f;
                }
                this->fogMode = 2;
            }
            for (i = 0; i < 5; i++) {
                spawnVel1.x = Rand_CenteredFloat(20.0f);
                spawnVel1.y = Rand_ZeroFloat(5.0f) + 4.0f;
                spawnVel1.z = Rand_CenteredFloat(20.0f);

                spawnAccel1.x = spawnAccel1.z = 0.0f;
                spawnAccel1.y = -0.3f;

                temp_x = (spawnVel1.x * 20) / 10.0f;
                temp_z = (spawnVel1.z * 20) / 10.0f;
                spawnPos1.x = temp_x + this->holePosition.x;
                spawnPos1.y = 100.0f;
                spawnPos1.z = temp_z + this->holePosition.z;

                func_8002836C(play, &spawnPos1, &spawnVel1, &spawnAccel1, &colorYellow, &colorRed,
                              (s16)Rand_ZeroFloat(150.0f) + 800, 10, (s16)Rand_ZeroFloat(5.0f) + 17);
            }
        } else {
            for (i = 0; i < 2; i++) {
                spawnVel1.x = Rand_CenteredFloat(10.0f);
                spawnVel1.y = Rand_ZeroFloat(3.0f) + 3.0f;
                spawnVel1.z = Rand_CenteredFloat(10.0f);

                spawnAccel1.x = spawnAccel1.z = 0.0f;
                spawnAccel1.y = -0.3f;
                temp_x = (spawnVel1.x * 50) / 10.0f;
                temp_z = (spawnVel1.z * 50) / 10.0f;

                spawnPos1.x = temp_x + this->holePosition.x;
                spawnPos1.y = 100.0f;
                spawnPos1.z = temp_z + this->holePosition.z;

                func_8002836C(play, &spawnPos1, &spawnVel1, &spawnAccel1, &colorYellow, &colorRed, 500, 10, 20);
            }
        }

        for (i = 0; i < 8; i++) {
            spawnVel1.x = Rand_CenteredFloat(20.0f);
            spawnVel1.y = Rand_ZeroFloat(10.0f);
            spawnVel1.z = Rand_CenteredFloat(20.0f);

            spawnAccel1.y = 0.4f;
            spawnAccel1.x = Rand_CenteredFloat(0.5f);
            spawnAccel1.z = Rand_CenteredFloat(0.5f);

            spawnPos1.x = Rand_CenteredFloat(60.0) + this->holePosition.x;
            spawnPos1.y = Rand_ZeroFloat(40.0f) + 100.0f;
            spawnPos1.z = Rand_CenteredFloat(60.0) + this->holePosition.z;

            BossFd_SpawnEmber(this->effects, &spawnPos1, &spawnVel1, &spawnAccel1, (s16)Rand_ZeroFloat(1.5f) + 6);
        }
    }

    if ((this->fireBreathTimer != 0) && (this->fireBreathTimer < 17)) {
        breathOpacity = (this->fireBreathTimer >= 6) ? 255 : this->fireBreathTimer * 50;
    }
    if (breathOpacity != 0) {
        f32 spawnAngleX;
        f32 spawnAngleY;
        Vec3f spawnSpeed2 = { 0.0f, 0.0f, 0.0f };
        Vec3f spawnVel2;
        Vec3f spawnAccel2 = { 0.0f, 0.0f, 0.0f };
        Vec3f spawnPos2;

        this->fogMode = 2;
        spawnSpeed2.z = 30.0f;

        Audio_PlaySoundGeneral(NA_SE_EN_VALVAISA_FIRE - SFX_FLAG, &sFireAudioVec, 4, &gSfxDefaultFreqAndVolScale,
                               &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
        spawnPos2 = this->headPos;

        spawnAngleY = (this->actor.world.rot.y / (f32)0x8000) * M_PI;
        spawnAngleX = (((-this->actor.world.rot.x) / (f32)0x8000) * M_PI) + 0.3f;
        Matrix_RotateY(spawnAngleY, MTXMODE_NEW);
        Matrix_RotateX(spawnAngleX, MTXMODE_APPLY);
        Matrix_MultVec3f(&spawnSpeed2, &spawnVel2);

        BossFd_SpawnFireBreath(this->effects, &spawnPos2, &spawnVel2, &spawnAccel2,
                               50.0f * Math_SinS(this->work[BFD_VAR_TIMER] * 0x2000) + 300.0f, breathOpacity,
                               this->actor.world.rot.y);

        spawnPos2.x += spawnVel2.x * 0.5f;
        spawnPos2.y += spawnVel2.y * 0.5f;
        spawnPos2.z += spawnVel2.z * 0.5f;

        BossFd_SpawnFireBreath(this->effects, &spawnPos2, &spawnVel2, &spawnAccel2,
                               50.0f * Math_SinS(this->work[BFD_VAR_TIMER] * 0x2000) + 300.0f, breathOpacity,
                               this->actor.world.rot.y);
        spawnSpeed2.x = 0.0f;
        spawnSpeed2.y = 17.0f;
        spawnSpeed2.z = 0.0f;

        for (i = 0; i < 6; i++) {
            spawnAngleY = Rand_ZeroFloat(2.0f * M_PI);
            spawnAngleX = Rand_ZeroFloat(2.0f * M_PI);
            Matrix_RotateY(spawnAngleY, MTXMODE_NEW);
            Matrix_RotateX(spawnAngleX, MTXMODE_APPLY);
            Matrix_MultVec3f(&spawnSpeed2, &spawnVel2);

            spawnAccel2.x = (spawnVel2.x * -10) / 100;
            spawnAccel2.y = (spawnVel2.y * -10) / 100;
            spawnAccel2.z = (spawnVel2.z * -10) / 100;

            BossFd_SpawnEmber(this->effects, &this->headPos, &spawnVel2, &spawnAccel2, (s16)Rand_ZeroFloat(2.0f) + 8);
        }
    }

    if (!BossFd_IsCombatActive(this)) {
        this->actor.flags &= ~ACTOR_FLAG_ATTENTION_ENABLED;
    } else {
        this->actor.flags |= ACTOR_FLAG_ATTENTION_ENABLED;
    }
}

void BossFd_CollisionCheck(BossFd* this, PlayState* play) {
    ColliderJntSphElement* headCollider = &this->collider.elements[0];
    ColliderInfo* hurtbox;

    if (headCollider->info.bumperFlags & BUMP_HIT) {
        headCollider->info.bumperFlags &= ~BUMP_HIT;
        hurtbox = headCollider->info.acHitInfo;
        this->actor.colChkInfo.health -= 2;
        if (hurtbox->toucher.dmgFlags & 0x1000) {
            this->actor.colChkInfo.health -= 2;
        }
        if ((s8)this->actor.colChkInfo.health <= 2) {
            this->actor.colChkInfo.health = 2;
        }
        this->work[BFD_DAMAGE_FLASH_TIMER] = 10;
        this->work[BFD_INVINC_TIMER] = 20;
        Audio_PlaySoundGeneral(NA_SE_EN_VALVAISA_DAMAGE1, &this->actor.projectedPos, 4, &gSfxDefaultFreqAndVolScale,
                               &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
    }
}

static bool BossFd_IsCombatActive(BossFd* this) {
    return (this->actor.colChkInfo.health > 0) && (this->actionFunc == BossFd_Fly) &&
           (this->introState == BFD_CS_NONE) && (this->work[BFD_ACTION_STATE] < BOSSFD_DEATH_START) &&
           (this->actor.world.pos.y >= 90.0f) && (this->actor.world.pos.y <= 700.0f);
}

static void BossFd_UpdateCollisionState(BossFd* this, PlayState* play) {
    bool combatActive = BossFd_IsCombatActive(this);
    s32 i;

    if (combatActive && (this->work[BFD_ACTION_STATE] != BOSSFD_FLY_GRAB)) {
        this->collider.base.atFlags |= AT_ON;
    } else {
        this->collider.base.atFlags &= ~(AT_ON | AT_HIT | AT_BOUNCED);
        for (i = 0; i < ARRAY_COUNT(this->elements); i++) {
            this->collider.elements[i].info.toucherFlags &= ~TOUCH_HIT;
        }
    }
    if (combatActive) {
        this->collider.base.acFlags |= AC_ON;
    } else {
        this->collider.base.acFlags &= ~(AC_ON | AC_HIT | AC_BOUNCED);
        for (i = 0; i < ARRAY_COUNT(this->elements); i++) {
            this->collider.elements[i].info.bumperFlags &= ~BUMP_HIT;
        }
    }
    this->collider.base.ocFlags1 &= ~(OC1_ON | OC1_HIT);

    if (this->work[BFD_ACTION_STATE] < BOSSFD_DEATH_START) {
        CollisionCheck_SetAC(play, &play->colChkCtx, &this->collider.base);
        CollisionCheck_SetAT(play, &play->colChkCtx, &this->collider.base);
    }
}

void BossFd_Update(Actor* thisx, PlayState* play) {
    s32 pad;
    BossFd* this = (BossFd*)thisx;
    f32 headGlow;
    f32 rManeGlow;
    f32 lManeGlow;
    s16 i;
    bool secondCollisionUpdate = GameInteractor_SecondCollisionUpdate();

    if (secondCollisionUpdate &&
        ((this->work[BFD_ACTION_STATE] == BOSSFD_FLY_GRAB) || this->grabTransitionedThisFrame)) {
        return;
    }
    if (!secondCollisionUpdate) {
        this->grabTransitionedThisFrame = false;
    }

    osSyncPrintf("FD MOVE START \n");
    this->work[BFD_VAR_TIMER]++;
    this->work[BFD_MOVE_TIMER]++;
    this->actionFunc(this, play);

    if ((this->actor.colChkInfo.health == 0) && !this->hazardsCleared) {
        BossFd_ClearCombatHazards(this, play);
        this->hazardsCleared = true;
    }

    for (i = 0; i < ARRAY_COUNT(this->timers); i++) {
        if (this->timers[i] != 0) {
            this->timers[i]--;
        }
    }
    if (this->fireBreathTimer != 0) {
        this->fireBreathTimer--;
    }
    if (this->work[BFD_DAMAGE_FLASH_TIMER] != 0) {
        this->work[BFD_DAMAGE_FLASH_TIMER]--;
    }
    if (this->work[BFD_INVINC_TIMER] != 0) {
        this->work[BFD_INVINC_TIMER]--;
    }
    if ((this->work[BFD_ACTION_STATE] != BOSSFD_FLY_LOW_CIRCLE) &&
        ((this->fogMode == BOSSFD_FOG_MODE_ERUPTION) || (this->fogMode == BOSSFD_FOG_MODE_ERUPTION_IN))) {
        this->fogMode = BOSSFD_FOG_MODE_ERUPTION_OUT;
        play->envCtx.blendIndoorLights = true;
        play->envCtx.unk_D8 = 0.0f;
        play->envCtx.unk_DC = 0;
    }
    if (this->work[BFD_ACTION_STATE] < BOSSFD_DEATH_START) {
        if (BossFd_IsCombatActive(this) && (this->work[BFD_INVINC_TIMER] == 0)) {
            BossFd_CollisionCheck(this, play);
        }
    }
    BossFd_UpdateCollisionState(this, play);

    BossFd_Effects(this, play);
    this->fwork[BFD_TEX1_SCROLL_X] += 4.0f;
    this->fwork[BFD_TEX1_SCROLL_Y] = 120.0f;
    this->fwork[BFD_TEX2_SCROLL_X] += 3.0f;
    this->fwork[BFD_TEX2_SCROLL_Y] -= 2.0f;

    Math_ApproachF(&this->fwork[BFD_BODY_TEX2_ALPHA], (this->work[BFD_VAR_TIMER] & 0x10) ? 30.0f : 158.0f, 1.0f, 8.0f);
    if (this->skinSegments == 0) {
        this->fwork[BFD_HEAD_TEX2_ALPHA] = this->fwork[BFD_BODY_TEX2_ALPHA];
    } else {
        headGlow = (this->work[BFD_VAR_TIMER] & 4) ? 0.0f : 255.0f;
        Math_ApproachF(&this->fwork[BFD_HEAD_TEX2_ALPHA], headGlow, 1.0f, 64.0f);
    }

    headGlow = (this->work[BFD_VAR_TIMER] & 8) ? 128.0f : 255.0f;
    rManeGlow = ((this->work[BFD_VAR_TIMER] + 3) & 8) ? 128.0f : 255.0f;
    lManeGlow = ((this->work[BFD_VAR_TIMER] + 6) & 8) ? 128.0f : 255.0f;

    Math_ApproachF(&this->fwork[BFD_MANE_COLOR_CENTER], headGlow, 1.0f, 16.0f);
    Math_ApproachF(&this->fwork[BFD_MANE_COLOR_RIGHT], rManeGlow, 1.0f, 16.0f);
    Math_ApproachF(&this->fwork[BFD_MANE_COLOR_LEFT], lManeGlow, 1.0f, 16.0f);

    if (this->work[BFD_ROCK_TIMER] != 0) {
        if (this->work[BFD_ACTION_STATE] != BOSSFD_FLY_LOW_CIRCLE) {
            this->work[BFD_ROCK_TIMER] = 0;
        } else {
            this->work[BFD_ROCK_TIMER]--;
            if ((this->work[BFD_ROCK_TIMER] % BOSSFD_LOW_CIRCLE_ROCK_INTERVAL) == 0) {
                u8 holeIndex = (u8)Rand_ZeroFloat(8.9f);
                f32 rockPosX = sHoleLocations[holeIndex].x + Rand_CenteredFloat(120.0f);
                f32 rockPosZ = sHoleLocations[holeIndex].z + Rand_CenteredFloat(120.0f);

                EnVbBall* bossFdRock =
                    BossFd_TrySpawnRock(this, play, rockPosX, BOSSFD_LOW_CIRCLE_ROCK_HEIGHT, rockPosZ,
                                        (s16)Rand_ZeroFloat(50.0f) + 130, ENVBALL_ROCK_IMPACT);

                if (bossFdRock != NULL) {
                    for (i = 0; i < BOSSFD_LOW_CIRCLE_ROCK_DEBRIS_COUNT; i++) {
                        Vec3f debrisVel = { 0.0f, 0.0f, 0.0f };
                        Vec3f debrisAccel = { 0.0f, -0.5f, 0.0f };
                        Vec3f debrisPos;

                        debrisPos.x = Rand_CenteredFloat(300.0f) + bossFdRock->actor.world.pos.x;
                        debrisPos.y = Rand_CenteredFloat(300.0f) + bossFdRock->actor.world.pos.y;
                        debrisPos.z = Rand_CenteredFloat(300.0f) + bossFdRock->actor.world.pos.z;

                        BossFd_SpawnDebris(this->effects, &debrisPos, &debrisVel, &debrisAccel,
                                           (s16)Rand_ZeroFloat(15.0f) + 10);
                    }
                }
            }
        }
    }

    if (1) { // Needed for matching, and also to define new variables
        Vec3f emberVel = { 0.0f, 0.0f, 0.0f };
        Vec3f emberAccel = { 0.0f, 0.0f, 0.0f };
        Vec3f emberPos;
        s16 temp_rand;

        for (i = 0; i < 6; i++) {
            emberAccel.y = 0.4f;
            emberAccel.x = Rand_CenteredFloat(0.5f);
            emberAccel.z = Rand_CenteredFloat(0.5f);

            temp_rand = Rand_ZeroFloat(8.9f);

            emberPos.x = sHoleLocations[temp_rand].x + Rand_CenteredFloat(60.0f);
            emberPos.y = (sHoleLocations[temp_rand].y + 10.0f) + Rand_ZeroFloat(40.0f);
            emberPos.z = sHoleLocations[temp_rand].z + Rand_CenteredFloat(60.0f);

            BossFd_SpawnEmber(this->effects, &emberPos, &emberVel, &emberAccel, (s16)Rand_ZeroFloat(2.0f) + 6);
        }

        if (this->skinSegments != 0) {
            for (i = 0; i < (s16)this->fwork[BFD_MANE_EMBER_RATE]; i++) {
                temp_rand = Rand_ZeroFloat(29.9f);
                emberPos.y = this->centerMane.pos[temp_rand].y + Rand_CenteredFloat(20.0f);

                if (emberPos.y >= 90.0f) {
                    emberPos.x = this->centerMane.pos[temp_rand].x + Rand_CenteredFloat(20.0f);
                    emberPos.z = this->centerMane.pos[temp_rand].z + Rand_CenteredFloat(20.0f);

                    emberVel.x = Rand_CenteredFloat(this->fwork[BFD_MANE_EMBER_SPEED]);
                    emberVel.y = Rand_CenteredFloat(this->fwork[BFD_MANE_EMBER_SPEED]);
                    emberVel.z = Rand_CenteredFloat(this->fwork[BFD_MANE_EMBER_SPEED]);

                    emberAccel.y = 0.4f;
                    emberAccel.x = Rand_CenteredFloat(0.5f);
                    emberAccel.z = Rand_CenteredFloat(0.5f);

                    BossFd_SpawnEmber(this->effects, &emberPos, &emberVel, &emberAccel, (s16)Rand_ZeroFloat(2.0f) + 8);
                }
            }
        }
    }
    osSyncPrintf("FD MOVE END 1\n");
    BossFd_UpdateEffects(this, play);
    osSyncPrintf("FD MOVE END 2\n");
}

void BossFd_UpdateEffects(BossFd* this, PlayState* play) {
    BossFdEffect* effect = this->effects;
    Player* player = GET_PLAYER(play);
    Color_RGB8 colors[4] = { { 255, 128, 0 }, { 255, 0, 0 }, { 255, 255, 0 }, { 255, 0, 0 } };
    Vec3f diff;
    s16 i1;
    s16 i2;

    for (i1 = 0; i1 < 180; i1++, effect++) {
        if (effect->type != BFD_FX_NONE) {
            effect->timer1++;

            effect->pos.x += effect->velocity.x;
            effect->pos.y += effect->velocity.y;
            effect->pos.z += effect->velocity.z;

            effect->velocity.x += effect->accel.x;
            effect->velocity.y += effect->accel.y;
            effect->velocity.z += effect->accel.z;
            if (effect->type == BFD_FX_EMBER) {
                s16 cInd = effect->timer1 % 4;

                effect->color.r = colors[cInd].r;
                effect->color.g = colors[cInd].g;
                effect->color.b = colors[cInd].b;
                effect->alpha -= 20;
                if (effect->alpha <= 0) {
                    effect->alpha = 0;
                    effect->type = 0;
                }
            } else if ((effect->type == BFD_FX_DEBRIS) || (effect->type == BFD_FX_SKULL_PIECE)) {
                effect->vFdFxRotX += 0.55f;
                effect->vFdFxRotY += 0.1f;
                if (effect->pos.y <= 100.0f) {
                    effect->type = 0;
                }
            } else if (effect->type == BFD_FX_DUST) {
                if (effect->timer2 >= 8) {
                    effect->timer2 = 8;
                    effect->type = 0;
                } else if (((effect->timer1 % 2) != 0) || (Rand_ZeroOne() < 0.3f)) {
                    effect->timer2++;
                }
            } else if (effect->type == BFD_FX_FIRE_BREATH) {
                diff.x = player->actor.world.pos.x - effect->pos.x;
                diff.y = player->actor.world.pos.y + 30.0f - effect->pos.y;
                diff.z = player->actor.world.pos.z - effect->pos.z;
                if ((this->timers[3] == 0) && (sqrtf(SQ(diff.x) + SQ(diff.y) + SQ(diff.z)) < 20.0f)) {
                    this->timers[3] = 50;
                    Actor_SetPlayerKnockbackLarge(play, NULL, 5.0f, effect->kbAngle, 0.0f, 0x30);
                    if (player->bodyIsBurning == false) {
                        for (i2 = 0; i2 < ARRAY_COUNT(player->bodyFlameTimers); i2++) {
                            player->bodyFlameTimers[i2] = Rand_S16Offset(0, 200);
                        }
                        player->bodyIsBurning = true;
                    }
                }
                if (effect->timer2 == 0) {
                    if (effect->scale < 2.5f) {
                        effect->scale += effect->vFdFxScaleMod;
                        effect->vFdFxScaleMod += 0.08f;
                    }
                    if ((effect->pos.y <= (effect->vFdFxYStop + 130.0f)) || (effect->timer1 >= 10)) {
                        effect->accel.y = 5.0f;
                        effect->timer2++;
                        effect->velocity.y = 0.0f;
                        effect->accel.x = (effect->velocity.x * -25.0f) / 100.0f;
                        effect->accel.z = (effect->velocity.z * -25.0f) / 100.0f;
                    }
                } else {
                    if (effect->scale < 2.5f) {
                        Math_ApproachF(&effect->scale, 2.5f, 0.5f, 0.5f);
                    }
                    effect->timer2++;
                    if (effect->timer2 >= 9) {
                        effect->type = 0;
                    }
                }
            }
        }
    }
}

void BossFd_DrawEffects(BossFdEffect* effect, PlayState* play) {
    static void* dustTex[] = {
        gDust1Tex, gDust1Tex, gDust2Tex, gDust3Tex, gDust4Tex, gDust5Tex, gDust6Tex, gDust7Tex, gDust8Tex,
    };
    u8 flag = false;
    GraphicsContext* gfxCtx = play->state.gfxCtx;
    s16 i;
    BossFdEffect* firstEffect = effect;

    OPEN_DISPS(gfxCtx);

    for (i = 0; i < 180; i++, effect++) {
        if (effect->type == BFD_FX_EMBER) {
            FrameInterpolation_RecordOpenChild(effect, effect->epoch);
            if (!flag) {
                Gfx_SetupDL_25Xlu(play->state.gfxCtx);
                gSPDisplayList(POLY_XLU_DISP++, gVolvagiaEmberMaterialDL);
                flag++;
            }

            gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, effect->color.r, effect->color.g, effect->color.b, effect->alpha);
            Matrix_Translate(effect->pos.x, effect->pos.y, effect->pos.z, MTXMODE_NEW);
            Matrix_ReplaceRotation(&play->billboardMtxF);
            Matrix_Scale(effect->scale, effect->scale, 1.0f, MTXMODE_APPLY);

            gSPMatrix(POLY_XLU_DISP++, MATRIX_NEWMTX(gfxCtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
            gSPDisplayList(POLY_XLU_DISP++, gVolvagiaEmberModelDL);
            FrameInterpolation_RecordCloseChild();
        }
    }

    effect = firstEffect;
    flag = false;
    for (i = 0; i < 180; i++, effect++) {
        if (effect->type == BFD_FX_DEBRIS) {
            FrameInterpolation_RecordOpenChild(effect, effect->epoch);
            if (!flag) {
                Gfx_SetupDL_25Opa(play->state.gfxCtx);
                gSPDisplayList(POLY_OPA_DISP++, gVolvagiaDebrisMaterialDL);
                flag++;
            }

            Matrix_Translate(effect->pos.x, effect->pos.y, effect->pos.z, MTXMODE_NEW);
            Matrix_RotateY(effect->vFdFxRotY, MTXMODE_APPLY);
            Matrix_RotateX(effect->vFdFxRotX, MTXMODE_APPLY);
            Matrix_Scale(effect->scale, effect->scale, 1.0f, MTXMODE_APPLY);

            gSPMatrix(POLY_OPA_DISP++, MATRIX_NEWMTX(gfxCtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
            gSPDisplayList(POLY_OPA_DISP++, gVolvagiaDebrisModelDL);
            FrameInterpolation_RecordCloseChild();
        }
    }

    effect = firstEffect;
    flag = false;
    for (i = 0; i < 180; i++, effect++) {
        if (effect->type == BFD_FX_DUST) {
            FrameInterpolation_RecordOpenChild(effect, effect->epoch);
            if (!flag) {
                POLY_XLU_DISP = Gfx_SetupDL(POLY_XLU_DISP, 0);
                gSPDisplayList(POLY_XLU_DISP++, gVolvagiaDustMaterialDL);
                gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 90, 30, 0, 255);
                gDPSetEnvColor(POLY_XLU_DISP++, 90, 30, 0, 0);
                flag++;
            }

            Matrix_Translate(effect->pos.x, effect->pos.y, effect->pos.z, MTXMODE_NEW);
            Matrix_Scale(effect->scale, effect->scale, effect->scale, MTXMODE_APPLY);
            Matrix_ReplaceRotation(&play->billboardMtxF);

            gSPMatrix(POLY_XLU_DISP++, MATRIX_NEWMTX(gfxCtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
            gSPSegment(POLY_XLU_DISP++, 0x08, SEGMENTED_TO_VIRTUAL(dustTex[effect->timer2]));
            gSPDisplayList(POLY_XLU_DISP++, gVolvagiaDustModelDL);
            FrameInterpolation_RecordCloseChild();
        }
    }

    effect = firstEffect;
    flag = false;
    for (i = 0; i < 180; i++, effect++) {
        if (effect->type == BFD_FX_FIRE_BREATH) {
            FrameInterpolation_RecordOpenChild(effect, effect->epoch);
            if (!flag) {
                POLY_XLU_DISP = Gfx_SetupDL(POLY_XLU_DISP, 0);
                gSPDisplayList(POLY_XLU_DISP++, gVolvagiaDustMaterialDL);
                gDPSetEnvColor(POLY_XLU_DISP++, 255, 10, 0, 255);
                flag++;
            }

            gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 255, 255, 0, effect->alpha);
            Matrix_Translate(effect->pos.x, effect->pos.y, effect->pos.z, MTXMODE_NEW);
            Matrix_Scale(effect->scale, effect->scale, effect->scale, MTXMODE_APPLY);
            Matrix_ReplaceRotation(&play->billboardMtxF);

            gSPMatrix(POLY_XLU_DISP++, MATRIX_NEWMTX(gfxCtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
            gSPSegment(POLY_XLU_DISP++, 0x08, SEGMENTED_TO_VIRTUAL(dustTex[effect->timer2]));
            gSPDisplayList(POLY_XLU_DISP++, gVolvagiaDustModelDL);
            FrameInterpolation_RecordCloseChild();
        }
    }

    effect = firstEffect;
    flag = false;
    for (i = 0; i < 180; i++, effect++) {
        if (effect->type == BFD_FX_SKULL_PIECE) {
            FrameInterpolation_RecordOpenChild(effect, effect->epoch);
            if (!flag) {
                Gfx_SetupDL_25Xlu(play->state.gfxCtx);
                gSPDisplayList(POLY_XLU_DISP++, gVolvagiaSkullPieceMaterialDL);
                flag++;
            }

            Matrix_Translate(effect->pos.x, effect->pos.y, effect->pos.z, MTXMODE_NEW);
            Matrix_RotateY(effect->vFdFxRotY, MTXMODE_APPLY);
            Matrix_RotateX(effect->vFdFxRotX, MTXMODE_APPLY);
            Matrix_Scale(effect->scale, effect->scale, 1.0f, MTXMODE_APPLY);

            gSPMatrix(POLY_XLU_DISP++, MATRIX_NEWMTX(gfxCtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
            gSPDisplayList(POLY_XLU_DISP++, gVolvagiaSkullPieceModelDL);
            FrameInterpolation_RecordCloseChild();
        }
    }

    CLOSE_DISPS(gfxCtx);
}

void BossFd_Draw(Actor* thisx, PlayState* play) {
    s32 pad;
    BossFd* this = (BossFd*)thisx;

    osSyncPrintf("FD DRAW START\n");
    if (this->actionFunc != BossFd_Wait) {
        OPEN_DISPS(play->state.gfxCtx);
        Gfx_SetupDL_25Opa(play->state.gfxCtx);
        if (this->work[BFD_DAMAGE_FLASH_TIMER] & 2) {
            POLY_OPA_DISP = Gfx_SetFog(POLY_OPA_DISP, 255, 255, 255, 0, 900, 1099);
        }

        BossFd_DrawBody(play, this);
        POLY_OPA_DISP = Play_SetFog(play, POLY_OPA_DISP);
        CLOSE_DISPS(play->state.gfxCtx);
    }

    osSyncPrintf("FD DRAW END\n");
    BossFd_DrawEffects(this->effects, play);
    osSyncPrintf("FD DRAW END2\n");
}

s32 BossFd_OverrideRightArmDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3f* pos, Vec3s* rot, void* thisx) {
    BossFd* this = (BossFd*)thisx;

    switch (limbIndex) {
        case 1:
            rot->y += 4000.0f + this->rightArmRot[0].x;
            break;
        case 2:
            rot->y += this->rightArmRot[1].x;
            rot->z += this->rightArmRot[1].y;
            break;
        case 3:
            rot->y += this->rightArmRot[2].x;
            rot->z += this->rightArmRot[2].y;
            break;
    }
    if (this->skinSegments < limbIndex) {
        *dList = NULL;
    }
    return false;
}

s32 BossFd_PostRightArmDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3s* rot, void* thisx) {
    static Vec3f handRef = { 0.0f, 0.0f, 0.0f };
    static Vec3f forwardRef = { 0.0f, 0.0f, 2000.0f };
    BossFd* this = (BossFd*)thisx;

    if (limbIndex == 3) {
        Matrix_MultVec3f(&handRef, &this->rightHandPos);
        Matrix_MultVec3f(&forwardRef, &this->rightHandForward);
    }

    return false;
}

s32 BossFd_OverrideLeftArmDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3f* pos, Vec3s* rot, void* thisx) {
    BossFd* this = (BossFd*)thisx;

    switch (limbIndex) {
        case 1:
            rot->y += -4000.0f + this->leftArmRot[0].x;
            break;
        case 2:
            rot->y += this->leftArmRot[1].x;
            rot->z += this->leftArmRot[1].y;
            break;
        case 3:
            rot->y += this->leftArmRot[2].x;
            rot->z += this->leftArmRot[2].y;
            break;
    }
    if (this->skinSegments < limbIndex) {
        *dList = NULL;
    }
    return false;
}

static s16 sBodyIndex[] = { 0, 95, 90, 85, 80, 75, 70, 65, 60, 55, 50, 45, 40, 35, 30, 25, 20, 15, 10, 5 };
static s16 sManeIndex[] = { 0, 28, 26, 24, 22, 20, 18, 16, 14, 12, 10 }; // Unused

void BossFd_DrawMane(PlayState* play, BossFd* this, Vec3f* manePos, Vec3f* maneRot, f32* maneScale, u8 mode) {
    f32 sp140[] = { 0.0f, 10.0f, 17.0f, 20.0f, 19.5f, 18.0f, 17.0f, 15.0f, 15.0f, 15.0f };
    f32 sp118[] = { 0.0f, 10.0f, 17.0f, 20.0f, 21.0f, 21.0f, 21.0f, 21.0f, 21.0f, 21.0f };
    f32 spF0[] = { 0.4636457f, 0.3366129f, 0.14879614f, 0.04995025f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
    // arctan of {0.5, 0.35, 0.15, 0.05, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}
    f32 spC8[] = { -0.4636457f, -0.3366129f, -0.14879614f, 0.024927188f, 0.07478157f,
                   0.04995025f, 0.09961288f, 0.0f,         0.0f,         0.0f };
    // arctan of {-0.5, -0.35, -0.15, 0.025, 0.075, 0.05, 0.1, 0.0, 0.0}
    s16 maneIndex;
    s16 i;
    s16 maneLength;
    Vec3f spB4;
    Vec3f spA8;
    f32 phi_f20;
    f32 phi_f22;

    OPEN_DISPS(play->state.gfxCtx);

    maneLength = this->skinSegments;
    maneLength = CLAMP_MAX(maneLength, 10);

    for (i = 0; i < maneLength; i++) {
        maneIndex = (this->work[BFD_LEAD_MANE_SEG] - (i * 2) + 30) % 30;

        if (mode == 0) {
            spB4.x = spB4.z = 0.0f;
            spB4.y = ((sp140[i] * 0.1f) * 10.0f) * this->flattenMane;
            phi_f20 = 0.0f;
            phi_f22 = spC8[i] * this->flattenMane;
        } else if (mode == 1) {
            phi_f22 = (spC8[i] * this->flattenMane) * 0.7f;
            phi_f20 = spF0[i] * this->flattenMane;

            spB4.y = (sp140[i] * this->flattenMane) * 0.7f;
            spB4.x = -sp118[i] * this->flattenMane;
            spB4.z = 0.0f;
        } else {
            phi_f22 = (spC8[i] * this->flattenMane) * 0.7f;
            phi_f20 = -spF0[i] * this->flattenMane;

            spB4.y = (sp140[i] * this->flattenMane) * 0.7f;
            spB4.x = sp118[i] * this->flattenMane;
            spB4.z = 0.0f;
        }

        Matrix_RotateY((maneRot + maneIndex)->y, MTXMODE_NEW);
        Matrix_RotateX(-(maneRot + maneIndex)->x, MTXMODE_APPLY);

        Matrix_MultVec3f(&spB4, &spA8);

        Matrix_Translate((manePos + maneIndex)->x + spA8.x, (manePos + maneIndex)->y + spA8.y,
                         (manePos + maneIndex)->z + spA8.z, MTXMODE_NEW);
        Matrix_RotateY((maneRot + maneIndex)->y + phi_f20, MTXMODE_APPLY);
        Matrix_RotateX(-((maneRot + maneIndex)->x + phi_f22), MTXMODE_APPLY);
        Matrix_Scale(maneScale[maneIndex] * (0.01f - (i * 0.0008f)), maneScale[maneIndex] * (0.01f - (i * 0.0008f)),
                     0.01f, MTXMODE_APPLY);
        Matrix_RotateX(-M_PI / 2.0f, MTXMODE_APPLY);
        gSPMatrix(POLY_XLU_DISP++, MATRIX_NEWMTX(play->state.gfxCtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
        gSPDisplayList(POLY_XLU_DISP++, gVolvagiaManeModelDL);
    }

    CLOSE_DISPS(play->state.gfxCtx);
}

s32 BossFd_OverrideHeadDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3f* pos, Vec3s* rot, void* thisx) {
    BossFd* this = (BossFd*)thisx;

    switch (limbIndex) {
        case 5:
        case 6:
            rot->z -= this->jawOpening * 0.1f;
            break;
        case 2:
            rot->z += this->jawOpening;
            break;
    }
    if ((this->faceExposed == true) && (limbIndex == 5)) {
        *dList = gVolvagiaBrokenFaceDL;
    }
    if (this->skinSegments == 0) {
        if (limbIndex == 6) {
            *dList = gVolvagiaSkullDL;
        } else if (limbIndex == 2) {
            *dList = gVolvagiaJawboneDL;
        } else {
            *dList = NULL;
        }
    }
    return false;
}

void BossFd_PostHeadDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3s* rot, void* thisx) {
    static Vec3f targetMod = { 4500.0f, 0.0f, 0.0f };
    static Vec3f headMod = { 4000.0f, 0.0f, 0.0f };
    static Vec3f forwardRef = { 0.0f, 0.0f, 2000.0f };
    BossFd* this = (BossFd*)thisx;

    if (limbIndex == 5) {
        MtxF headMtx;

        Matrix_MultVec3f(&targetMod, &this->actor.focus.pos);
        Matrix_MultVec3f(&headMod, &this->headPos);
        Matrix_MultVec3f(&forwardRef, &this->mouthForward);
        Matrix_Get(&headMtx);
        Matrix_MtxFToYXZRotS(&headMtx, &this->headRot, 0);
    }
}

static void* sEyeTextures[] = {
    gVolvagiaEyeOpenTex,
    gVolvagiaEyeHalfTex,
    gVolvagiaEyeClosedTex,
};

static Gfx* sBodyDLists[] = {
    gVolvagiaBodySeg1DL,  gVolvagiaBodySeg2DL,  gVolvagiaBodySeg3DL,  gVolvagiaBodySeg4DL,  gVolvagiaBodySeg5DL,
    gVolvagiaBodySeg6DL,  gVolvagiaBodySeg7DL,  gVolvagiaBodySeg8DL,  gVolvagiaBodySeg9DL,  gVolvagiaBodySeg10DL,
    gVolvagiaBodySeg11DL, gVolvagiaBodySeg12DL, gVolvagiaBodySeg13DL, gVolvagiaBodySeg14DL, gVolvagiaBodySeg15DL,
    gVolvagiaBodySeg16DL, gVolvagiaBodySeg17DL, gVolvagiaBodySeg18DL,
};

void BossFd_DrawBody(PlayState* play, BossFd* this) {
    s16 segIndex;
    s16 i;
    f32 temp_float;
    Mtx* tempMat = Graph_Alloc(play->state.gfxCtx, 18 * sizeof(Mtx));

    OPEN_DISPS(play->state.gfxCtx);
    if (this->skinSegments != 0) {
        gSPSegment(POLY_OPA_DISP++, 0x09, SEGMENTED_TO_VIRTUAL(sEyeTextures[this->eyeState]));
    }
    gSPSegment(POLY_OPA_DISP++, 0x08,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, (s16)this->fwork[BFD_TEX1_SCROLL_X],
                                  (s16)this->fwork[BFD_TEX1_SCROLL_Y], 0x20, 0x20, 1,
                                  (s16)this->fwork[BFD_TEX2_SCROLL_X], (s16)this->fwork[BFD_TEX2_SCROLL_Y], 0x20, 0x20,
                                  4, 1, 3, -2));
    gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 255, 255);
    gDPSetEnvColor(POLY_OPA_DISP++, 255, 255, 255, (s8)this->fwork[BFD_BODY_TEX2_ALPHA]);

    osSyncPrintf("LH\n");
    Matrix_Push();
    segIndex = (this->work[BFD_LEAD_BODY_SEG] + sBodyIndex[2]) % 100;
    Matrix_Translate(this->bodySegsPos[segIndex].x, this->bodySegsPos[segIndex].y, this->bodySegsPos[segIndex].z,
                     MTXMODE_NEW);
    Matrix_RotateY(this->bodySegsRot[segIndex].y, MTXMODE_APPLY);
    Matrix_RotateX(-this->bodySegsRot[segIndex].x, MTXMODE_APPLY);
    Matrix_Translate(-13.0f, -5.0f, 13.0f, MTXMODE_APPLY);
    Matrix_Scale(this->actor.scale.x * 0.1f, this->actor.scale.y * 0.1f, this->actor.scale.z * 0.1f, MTXMODE_APPLY);
    SkelAnime_DrawSkeletonOpa(play, &this->skelAnimeRightArm, BossFd_OverrideRightArmDraw, BossFd_PostRightArmDraw, this);
    Matrix_Pop();
    osSyncPrintf("RH\n");
    Matrix_Push();
    segIndex = (this->work[BFD_LEAD_BODY_SEG] + sBodyIndex[2]) % 100;
    Matrix_Translate(this->bodySegsPos[segIndex].x, this->bodySegsPos[segIndex].y, this->bodySegsPos[segIndex].z,
                     MTXMODE_NEW);
    Matrix_RotateY(this->bodySegsRot[segIndex].y, MTXMODE_APPLY);
    Matrix_RotateX(-this->bodySegsRot[segIndex].x, MTXMODE_APPLY);
    Matrix_Translate(13.0f, -5.0f, 13.0f, MTXMODE_APPLY);
    Matrix_Scale(this->actor.scale.x * 0.1f, this->actor.scale.y * 0.1f, this->actor.scale.z * 0.1f, MTXMODE_APPLY);
    SkelAnime_DrawSkeletonOpa(play, &this->skelAnimeLeftArm, BossFd_OverrideLeftArmDraw, NULL, this);
    Matrix_Pop();
    osSyncPrintf("BD\n");
    gSPSegment(POLY_OPA_DISP++, 0x0D, tempMat);

    Matrix_Push();
    for (i = 0; i < 18; i++, tempMat++) {
        segIndex = (this->work[BFD_LEAD_BODY_SEG] + sBodyIndex[i + 1]) % 100;
        Matrix_Translate(this->bodySegsPos[segIndex].x, this->bodySegsPos[segIndex].y, this->bodySegsPos[segIndex].z,
                         MTXMODE_NEW);
        Matrix_RotateY(this->bodySegsRot[segIndex].y, MTXMODE_APPLY);
        Matrix_RotateX(-this->bodySegsRot[segIndex].x, MTXMODE_APPLY);
        Matrix_Translate(0.0f, 0.0f, 35.0f, MTXMODE_APPLY);
        Matrix_Scale(this->actor.scale.x, this->actor.scale.y, this->actor.scale.z, MTXMODE_APPLY);
        if (i < this->skinSegments) {
            Matrix_Scale(1.0f + (Math_SinS((this->work[BFD_LEAD_BODY_SEG] * 5000.0f) + (i * 7000.0f)) *
                                 this->fwork[BFD_BODY_PULSE]),
                         1.0f + (Math_SinS((this->work[BFD_LEAD_BODY_SEG] * 5000.0f) + (i * 7000.0f)) *
                                 this->fwork[BFD_BODY_PULSE]),
                         1.0f, MTXMODE_APPLY);
            Matrix_RotateY(M_PI / 2.0f, MTXMODE_APPLY);
            MATRIX_TOMTX(tempMat);
            gSPMatrix(POLY_OPA_DISP++, tempMat, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
            gSPDisplayList(POLY_OPA_DISP++, sBodyDLists[i]);
        } else {
            MtxF spFC;
            Vec3f spF0 = { 0.0f, 0.0f, 0.0f };
            Vec3f spE4;
            Vec3s spDC;
            f32 padD8;

            if (this->bodyFallApart[i] < 2) {
                FrameInterpolation_RecordOpenChild(tempMat, i);

                f32 spD4 = 0.1f;

                temp_float = 0.1f;
                Matrix_Translate(0.0f, 0.0f, -1100.0f, MTXMODE_APPLY);
                Matrix_RotateY(-M_PI, MTXMODE_APPLY);
                if (i >= 14) {
                    f32 sp84 = 1.0f - ((i - 14) * 0.2f);

                    Matrix_Scale(sp84, sp84, 1.0f, MTXMODE_APPLY);
                    spD4 = 0.1f * sp84;
                    temp_float = 0.1f * sp84;
                }
                Matrix_Scale(0.1f, 0.1f, 0.1f, MTXMODE_APPLY);
                gSPMatrix(POLY_OPA_DISP++, MATRIX_NEWMTX(play->state.gfxCtx),
                          G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
                gSPDisplayList(POLY_OPA_DISP++, gVolvagiaRibsDL);

                if (this->bodyFallApart[i] == 1) {
                    EnVbBall* bones;

                    this->bodyFallApart[i] = 2;
                    Matrix_MultVec3f(&spF0, &spE4);
                    Matrix_Get(&spFC);
                    Matrix_MtxFToYXZRotS(&spFC, &spDC, 0);
                    bones = (EnVbBall*)Actor_SpawnAsChild(&play->actorCtx, &this->actor, play, ACTOR_EN_VB_BALL, spE4.x,
                                                          spE4.y, spE4.z, spDC.x, spDC.y, spDC.z, i + 200);

                    if (bones != NULL) {
                        bones->actor.scale.x = this->actor.scale.x * temp_float;
                        bones->actor.scale.y = this->actor.scale.y * spD4;
                        bones->actor.scale.z = this->actor.scale.z * 0.1f;
                    }
                }

                FrameInterpolation_RecordCloseChild();
            }
        }
        if (i > 0) {
            Collider_UpdateSpheres(i + 1, &this->collider);
        }
    }
    Matrix_Pop();
    osSyncPrintf("BH\n");

    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetEnvColor(POLY_OPA_DISP++, 255, 255, 255, (s8)this->fwork[BFD_HEAD_TEX2_ALPHA]);
    Matrix_Push();
    temp_float = BossFd_GetHeadDrawOffset(this, this->actor.speedXZ);
    segIndex = (this->work[BFD_LEAD_BODY_SEG] + sBodyIndex[0]) % 100;
    Matrix_Translate(this->bodySegsPos[segIndex].x, this->bodySegsPos[segIndex].y, this->bodySegsPos[segIndex].z,
                     MTXMODE_NEW);
    Matrix_RotateY(this->bodySegsRot[segIndex].y, MTXMODE_APPLY);
    Matrix_RotateX(-this->bodySegsRot[segIndex].x, MTXMODE_APPLY);
    Matrix_RotateZ((this->actor.shape.rot.z / (f32)0x8000) * M_PI, MTXMODE_APPLY);
    Matrix_Translate(0.0f, 0.0f, temp_float, MTXMODE_APPLY);
    Matrix_Push();
    Matrix_Translate(0.0f, 0.0f, 25.0f, MTXMODE_APPLY);
    osSyncPrintf("BHC\n");
    Collider_UpdateSpheres(0, &this->collider);
    Matrix_Pop();
    osSyncPrintf("BHCE\n");
    Matrix_Scale(this->actor.scale.x * 0.1f, this->actor.scale.y * 0.1f, this->actor.scale.z * 0.1f, MTXMODE_APPLY);
    SkelAnime_DrawSkeletonOpa(play, &this->skelAnimeHead, BossFd_OverrideHeadDraw, BossFd_PostHeadDraw, &this->actor);
    osSyncPrintf("SK\n");
    {
        Vec3f spB0 = { 0.0f, 1700.0f, 7000.0f };
        Vec3f spA4 = { -1000.0f, 700.0f, 7000.0f };

        Gfx_SetupDL_25Xlu(play->state.gfxCtx);
        gSPDisplayList(POLY_XLU_DISP++, gVolvagiaManeMaterialDL);
        gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 255, this->fwork[BFD_MANE_COLOR_CENTER], 0, 255);
        Matrix_Push();
        Matrix_MultVec3f(&spB0, &this->centerMane.head);
        BossFd_DrawMane(play, this, this->centerMane.pos, this->fireManeRot, this->centerMane.scale, MANE_CENTER);
        Matrix_Pop();
        gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 255, this->fwork[BFD_MANE_COLOR_RIGHT], 0, 255);
        Matrix_Push();
        Matrix_MultVec3f(&spA4, &this->rightMane.head);
        BossFd_DrawMane(play, this, this->rightMane.pos, this->fireManeRot, this->rightMane.scale, MANE_RIGHT);
        Matrix_Pop();
        gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 255, this->fwork[BFD_MANE_COLOR_LEFT], 0, 255);
        Matrix_Push();
        spA4.x *= -1.0f;
        Matrix_MultVec3f(&spA4, &this->leftMane.head);
        BossFd_DrawMane(play, this, this->leftMane.pos, this->fireManeRot, this->leftMane.scale, MANE_LEFT);
        Matrix_Pop();
    }

    Matrix_Pop();
    osSyncPrintf("END\n");
    CLOSE_DISPS(play->state.gfxCtx);
}
