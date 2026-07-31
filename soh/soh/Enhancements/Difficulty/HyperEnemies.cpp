#include "soh/Enhancements/game-interactor/GameInteractor_Hooks.h"
#include "soh/Enhancements/Difficulty/HyperSpeed.h"
#include "soh/ShipInit.hpp"
#include "functions.h"
#include "macros.h"

extern "C" PlayState* gPlayState;

static constexpr int32_t CVAR_HYPER_ENEMIES_DEFAULT = 0;
#define CVAR_HYPER_ENEMIES_NAME CVAR_ENHANCEMENT("HyperEnemies")
#define CVAR_HYPER_ENEMIES_VALUE CVarGetInteger(CVAR_HYPER_ENEMIES_NAME, CVAR_HYPER_ENEMIES_DEFAULT)

static void MakeHyperEnemies(void* refActor) {
    Actor* actor = static_cast<Actor*>(refActor);
    if (actor == nullptr || actor->update == nullptr) {
        HyperSpeed::Erase(actor);
        return;
    }

    Player* player = GET_PLAYER(gPlayState);

    // Dark Link (ACTOR_EN_TORCH2) is intentionally excluded from Hyper difficulty.
    bool isEnemy = actor->category == ACTORCAT_ENEMY;
    bool isExcludedEnemy = actor->id == ACTOR_EN_BDFIRE || // Scheduled with King Dodongo by Hyper Bosses.
                           actor->id == ACTOR_EN_FIRE_ROCK || actor->id == ACTOR_EN_ENCOUNT2 ||
                           actor->id == ACTOR_EN_TORCH2;

    // Don't apply during cutscenes because it causes weird behaviour and/or crashes on some cutscenes.
    if (isEnemy && !isExcludedEnemy && !Player_InBlockingCsMode(gPlayState, player)) {
        const int32_t additionalUpdates =
            HyperSpeed::CalculateAdditionalUpdates(HyperSpeed::TrackerGroup::Enemies, actor);

        for (int32_t i = 0; i < additionalUpdates; ++i) {
            GameInteractor::RawAction::UpdateActor(actor);
            if (actor->update == nullptr) {
                HyperSpeed::Erase(actor);
                break;
            }
        }
    }
}

static void UpdateHyperEnemiesState() {
    HyperSpeed::Reset(HyperSpeed::TrackerGroup::Enemies);
    COND_HOOK(OnActorUpdate, CVAR_HYPER_ENEMIES_VALUE && HyperSpeed::HasConfiguredExtraUpdates(), MakeHyperEnemies);
}

static RegisterShipInitFunc initFunc(
    UpdateHyperEnemiesState,
    { CVAR_HYPER_ENEMIES_NAME, HyperSpeed::FullHealthCVar, HyperSpeed::ZeroHealthCVar });
