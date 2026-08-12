#include "soh/ShipInit.hpp"
#include "functions.h"
#include "macros.h"
#include "variables.h"
#include "soh/Enhancements/game-interactor/GameInteractor_Hooks.h"
#include "soh/Enhancements/boss-rush/BossRush.h"
#include "soh/Enhancements/Difficulty/HyperSpeed.h"
#include "overlays/actors/ovl_Boss_Dodongo/z_boss_dodongo.h"
#include "overlays/actors/ovl_Boss_Fd/z_boss_fd.h"
#include "overlays/actors/ovl_Boss_Ganondrof/z_boss_ganondrof.h"
#include "overlays/actors/ovl_Boss_Tw/z_boss_tw.h"

extern "C" PlayState* gPlayState;
extern "C" s32 BossMo_GetHyperSpeedHealth(Actor* actor, s32* maximumHealth);
extern "C" s32 BossVa_GetHyperSpeedHealth(s32* maximumHealth);

#define CVAR_HYPER_BOSSES_DEFAULT 0
#define CVAR_HYPER_BOSSES_NAME CVAR_ENHANCEMENT("HyperBosses")
#define CVAR_HYPER_BOSSES_VALUE CVarGetInteger(CVAR_HYPER_BOSSES_NAME, CVAR_HYPER_BOSSES_DEFAULT)

namespace {

Actor* FindActiveBossActor(s16 actorId, bool matchParams = false, s16 params = 0) {
    Actor* actor = gPlayState->actorCtx.actorLists[ACTORCAT_BOSS].head;

    while (actor != nullptr) {
        if ((actor->id == actorId) && (!matchParams || (actor->params == params)) && (actor->update != nullptr)) {
            return actor;
        }
        actor = actor->next;
    }

    return nullptr;
}

Actor* FindActiveBossActorBelowParams(s16 actorId, s16 maximumParams) {
    Actor* actor = gPlayState->actorCtx.actorLists[ACTORCAT_BOSS].head;

    while (actor != nullptr) {
        if ((actor->id == actorId) && (actor->params < maximumParams) && (actor->update != nullptr)) {
            return actor;
        }
        actor = actor->next;
    }

    return nullptr;
}

int32_t ReadActorHealth(const Actor* actor) {
    const int32_t rawHealth = actor->colChkInfo.health;
    return rawHealth <= 0x7F ? rawHealth : 0;
}

HyperSpeed::Health GetActorHealth(const Actor* actor, int32_t maximumHealth) {
    return { ReadActorHealth(actor), maximumHealth };
}

HyperSpeed::Health GetOwnerHealth(s16 ownerId, int32_t maximumHealth, bool matchParams = false, s16 params = 0) {
    Actor* owner = FindActiveBossActor(ownerId, matchParams, params);
    return { owner != nullptr ? ReadActorHealth(owner) : 0, maximumHealth };
}

HyperSpeed::Health GetGanonOwnerHealth() {
    Actor* owner = FindActiveBossActorBelowParams(ACTOR_BOSS_GANON, 0x64);
    return { owner != nullptr ? ReadActorHealth(owner) : 0, 40 };
}

HyperSpeed::Health GetFhgFireOwnerHealth() {
    Actor* phantomGanon = FindActiveBossActor(ACTOR_BOSS_GANONDROF, true, GND_REAL_BOSS);
    if (phantomGanon != nullptr) {
        return { ReadActorHealth(phantomGanon), GND_MAX_HEALTH };
    }

    return GetGanonOwnerHealth();
}

HyperSpeed::Health GetBossHealth(Actor* actor) {
    switch (actor->id) {
        case ACTOR_BOSS_GOMA:
            return GetActorHealth(actor, 20);
        case ACTOR_BOSS_DODONGO:
            return { reinterpret_cast<BossDodongo*>(actor)->health, 20 };
        case ACTOR_EN_BDFIRE: {
            Actor* owner = FindActiveBossActor(ACTOR_BOSS_DODONGO);
            return { owner != nullptr ? reinterpret_cast<BossDodongo*>(owner)->health : 0, 20 };
        }
        case ACTOR_BOSS_VA: {
            s32 maximumHealth;
            s32 currentHealth = BossVa_GetHyperSpeedHealth(&maximumHealth);
            return { currentHealth, maximumHealth };
        }
        case ACTOR_BOSS_GANONDROF:
            return GetOwnerHealth(ACTOR_BOSS_GANONDROF, GND_MAX_HEALTH, true, GND_REAL_BOSS);
        case ACTOR_EN_FHG_FIRE:
            return GetFhgFireOwnerHealth();
        case ACTOR_EN_FHG:
            return GetOwnerHealth(ACTOR_BOSS_GANONDROF, GND_MAX_HEALTH, true, GND_REAL_BOSS);
        case ACTOR_BOSS_FD:
            return GetActorHealth(actor, BOSSFD_MAX_HEALTH);
        case ACTOR_BOSS_FD2:
        case ACTOR_EN_VB_BALL:
            return GetOwnerHealth(ACTOR_BOSS_FD, BOSSFD_MAX_HEALTH);
        case ACTOR_BOSS_MO: {
            s32 maximumHealth;
            s32 currentHealth = BossMo_GetHyperSpeedHealth(actor, &maximumHealth);
            return { currentHealth, maximumHealth };
        }
        case ACTOR_BOSS_SST:
            return GetOwnerHealth(ACTOR_BOSS_SST, 36, true, -1);
        case ACTOR_BOSS_TW:
            return GetOwnerHealth(ACTOR_BOSS_TW, 24, true, 2);
        case ACTOR_BOSS_GANON:
            return GetGanonOwnerHealth();
        case ACTOR_BOSS_GANON2:
            return GetActorHealth(actor, 30);
        default:
            return { ReadActorHealth(actor), 0 };
    }
}

} // namespace

bool IsHyperBossesActive() {
    return CVAR_HYPER_BOSSES_VALUE ||
           (IS_BOSS_RUSH &&
            gSaveContext.ship.quest.data.bossRush.options[BR_OPTIONS_HYPERBOSSES] == BR_CHOICE_HYPERBOSSES_YES);
}

void MakeHyperBosses(void* refActor) {
    Actor* actor = static_cast<Actor*>(refActor);
    if (actor == nullptr || actor->update == nullptr) {
        HyperSpeed::Erase(actor);
        return;
    }

    Player* player = GET_PLAYER(gPlayState);

    uint8_t isBossActor = actor->id == ACTOR_BOSS_GOMA ||      // Gohma
                          actor->id == ACTOR_BOSS_DODONGO ||   // King Dodongo
                          actor->id == ACTOR_EN_BDFIRE ||      // King Dodongo Fire Breath
                          actor->id == ACTOR_BOSS_VA ||        // Barinade
                          actor->id == ACTOR_BOSS_GANONDROF || // Phantom Ganon
                          actor->id == ACTOR_EN_FHG_FIRE ||    // Phantom Ganon/Ganondorf Energy Ball/Thunder
                          actor->id == ACTOR_EN_FHG ||         // Phantom Ganon's Horse
                          actor->id == ACTOR_BOSS_FD || actor->id == ACTOR_BOSS_FD2 || // Volvagia (grounded/flying)
                          actor->id == ACTOR_EN_VB_BALL ||                             // Volvagia Rocks
                          actor->id == ACTOR_BOSS_MO ||                                // Morpha
                          actor->id == ACTOR_BOSS_SST ||                               // Bongo Bongo
                          actor->id == ACTOR_BOSS_TW ||                                // Twinrova
                          actor->id == ACTOR_BOSS_GANON ||                             // Ganondorf
                          actor->id == ACTOR_BOSS_GANON2;                              // Ganon

    // Don't apply during cutscenes because it causes weird behaviour and/or crashes on some bosses.
    if (IsHyperBossesActive() && isBossActor && !Player_InBlockingCsMode(gPlayState, player)) {
        // Barinade needs to be updated in sequence to avoid unintended behaviour.
        if (actor->id == ACTOR_BOSS_VA) {
            // params -1 is BOSSVA_BODY
            if (actor->params == -1) {
                const int32_t additionalUpdates = HyperSpeed::CalculateAdditionalUpdates(
                    HyperSpeed::TrackerGroup::Bosses, actor, GetBossHealth(actor));
                for (int32_t i = 0; i < additionalUpdates; ++i) {
                    if (Player_InBlockingCsMode(gPlayState, player)) {
                        break;
                    }

                    Actor* actorList = gPlayState->actorCtx.actorLists[ACTORCAT_BOSS].head;
                    while (actorList != NULL) {
                        Actor* nextActor = actorList->next;
                        if (actorList->id == ACTOR_BOSS_VA && actorList->update != nullptr) {
                            GameInteractor::RawAction::UpdateActor(actorList);
                        }
                        actorList = nextActor;
                    }

                    if (actor->update == nullptr) {
                        HyperSpeed::Erase(actor);
                        break;
                    }
                }
            }
        } else {
            int32_t additionalUpdates = HyperSpeed::CalculateAdditionalUpdates(
                HyperSpeed::TrackerGroup::Bosses, actor, GetBossHealth(actor));
            if (actor->id == ACTOR_EN_FHG_FIRE) {
                // Reflectable projectiles and ground shocks need one collision step per movement step.
                // Their cadence still accelerates with the boss that spawns them.
                additionalUpdates = 0;
            } else if (actor->id == ACTOR_BOSS_TW &&
                       (actor->params >= TW_FIRE_BLAST || BossTw_ShouldUseNormalUpdateRate(actor))) {
                // Magic needs one collision step per movement step. Authored Twinrova attacks keep real-time tells
                // so their boss and independently updated warning actors cannot drift out of sync in Hyper mode.
                additionalUpdates = 0;
            } else if ((actor->id == ACTOR_BOSS_GANONDROF || actor->id == ACTOR_EN_FHG) &&
                       additionalUpdates > 1) {
                // Faster substeps collapse Phantom Ganon's tells and can tunnel his dash through the
                // one collision pass performed per rendered frame.
                additionalUpdates = 1;
            }
            for (int32_t i = 0; i < additionalUpdates; ++i) {
                if (Player_InBlockingCsMode(gPlayState, player)) {
                    break;
                }

                GameInteractor::RawAction::UpdateActor(actor);
                if (actor->update == nullptr) {
                    HyperSpeed::Erase(actor);
                    break;
                }
                if (actor->id == ACTOR_BOSS_TW && BossTw_ShouldUseNormalUpdateRate(actor)) {
                    // The update may have transitioned into an authored attack; do not spend the remaining Hyper
                    // substeps inside its telegraph on the same rendered frame.
                    break;
                }
            }
        }
    }
}

static void UpdateHyperBossesState() {
    HyperSpeed::Reset(HyperSpeed::TrackerGroup::Bosses);
    COND_HOOK(OnActorUpdate, IsHyperBossesActive() && HyperSpeed::HasConfiguredExtraUpdates(), MakeHyperBosses);
}

static void RegisterHyperBosses() {
    UpdateHyperBossesState();
    COND_HOOK(OnLoadGame, true, [](int32_t) { UpdateHyperBossesState(); });
}

static RegisterShipInitFunc initFunc(
    RegisterHyperBosses,
    { CVAR_HYPER_BOSSES_NAME, HyperSpeed::FullHealthCVar, HyperSpeed::ZeroHealthCVar });
