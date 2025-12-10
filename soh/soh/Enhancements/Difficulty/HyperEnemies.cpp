#include <libultraship/bridge.h>
#include <algorithm>
#include <cmath>
#include <unordered_map>
#include "soh/Enhancements/game-interactor/GameInteractor.h"
#include "soh/Enhancements/game-interactor/GameInteractor_Hooks.h"
#include "soh/Enhancements/mods.h"
#include "soh/ShipInit.hpp"
#include "functions.h"
#include "macros.h"

extern "C" PlayState* gPlayState;

static constexpr int32_t CVAR_HYPER_ENEMIES_DEFAULT = 0;
#define CVAR_HYPER_ENEMIES_NAME CVAR_ENHANCEMENT("HyperEnemies")
#define CVAR_HYPER_ENEMIES_VALUE CVarGetInteger(CVAR_HYPER_ENEMIES_NAME, CVAR_HYPER_ENEMIES_DEFAULT)

namespace {
std::unordered_map<Actor*, float> sFractionalEnemyUpdates;
std::unordered_map<Actor*, int32_t> sActorMaxHealth;
uint32_t sActorUpdateHookId = 0;

int32_t GetBaseHyperSpeedIncreasePercent() {
    int32_t speedIncreasePercent = CVarGetInteger(CVAR_ENHANCEMENT("HyperEnemySpeedIncreasePercent"), 100);

    speedIncreasePercent = std::clamp(speedIncreasePercent, 0, 400);

    return speedIncreasePercent;
}

int32_t GetZeroHealthHyperSpeedIncreasePercent() {
    int32_t speedIncreasePercent = CVarGetInteger(CVAR_ENHANCEMENT("HyperEnemySpeedAtZeroHealthPercent"), 100);

    speedIncreasePercent = std::clamp(speedIncreasePercent, -100, 400);

    return speedIncreasePercent;
}

float GetActorHealthRatio(Actor* actor) {
    if (actor == nullptr) {
        return 1.0f;
    }

    int32_t currentHealth = std::max<int32_t>(actor->colChkInfo.health, 0);
    int32_t& maxHealth = sActorMaxHealth[actor];

    if (maxHealth == 0 || currentHealth > maxHealth) {
        maxHealth = std::max(currentHealth, 1);
    }

    if (maxHealth <= 0) {
        return 1.0f;
    }

    const float healthRatio = static_cast<float>(currentHealth) / static_cast<float>(maxHealth);

    return std::clamp(healthRatio, 0.0f, 1.0f);
}

int32_t GetActorHyperSpeedIncreasePercent(Actor* actor) {
    const int32_t baseSpeedPercent = GetBaseHyperSpeedIncreasePercent();
    const int32_t zeroHealthSpeedPercent = GetZeroHealthHyperSpeedIncreasePercent();

    const float missingHealthRatio = 1.0f - GetActorHealthRatio(actor);
    const float lerpedSpeed = static_cast<float>(baseSpeedPercent) +
                              (static_cast<float>(zeroHealthSpeedPercent - baseSpeedPercent) * missingHealthRatio);

    return std::clamp(static_cast<int32_t>(std::round(lerpedSpeed)), -100, 400);
}

int32_t CalculateAdditionalHyperUpdates(int32_t speedIncreasePercent, Actor* actor,
                                        std::unordered_map<Actor*, float>& fractionalUpdates) {
    if (speedIncreasePercent <= 0) {
        return 0;
    }

    const float extraUpdatesPerFrame = static_cast<float>(speedIncreasePercent) / 100.0f;
    int32_t wholeExtraUpdates = static_cast<int32_t>(std::floor(extraUpdatesPerFrame));
    float& actorAccumulator = fractionalUpdates[actor];

    actorAccumulator += extraUpdatesPerFrame - static_cast<float>(wholeExtraUpdates);

    if (actorAccumulator >= 1.0f) {
        const int32_t fractionalCarry = static_cast<int32_t>(actorAccumulator);
        wholeExtraUpdates += fractionalCarry;
        actorAccumulator -= static_cast<float>(fractionalCarry);
    }

    return wholeExtraUpdates;
}
} // namespace

void UpdateHyperEnemiesState() {
    if (sActorUpdateHookId != 0) {
        GameInteractor::Instance->UnregisterGameHook<GameInteractor::OnActorUpdate>(sActorUpdateHookId);
        sActorUpdateHookId = 0;
    }

    sFractionalEnemyUpdates.clear();
    sActorMaxHealth.clear();

    const int32_t maxHyperSpeedPercent =
        std::max(GetBaseHyperSpeedIncreasePercent(), GetZeroHealthHyperSpeedIncreasePercent());

    if (CVAR_HYPER_ENEMIES_VALUE && maxHyperSpeedPercent > 0) {
        sActorUpdateHookId =
            GameInteractor::Instance->RegisterGameHook<GameInteractor::OnActorUpdate>([](void* refActor) {
                Player* player = GET_PLAYER(gPlayState);
                Actor* actor = static_cast<Actor*>(refActor);

                // Some enemies are not in the ACTORCAT_ENEMY category, and some are that aren't really enemies.
                bool isEnemy = actor->category == ACTORCAT_ENEMY || actor->id == ACTOR_EN_TORCH2;
                bool isExcludedEnemy = actor->id == ACTOR_EN_FIRE_ROCK || actor->id == ACTOR_EN_ENCOUNT2;

                const int32_t speedIncreasePercent = GetActorHyperSpeedIncreasePercent(actor);

                // Don't apply during cutscenes because it causes weird behaviour and/or crashes on some cutscenes.
                if (CVAR_HYPER_ENEMIES_VALUE && speedIncreasePercent > 0 && isEnemy && !isExcludedEnemy &&
                    !Player_InBlockingCsMode(gPlayState, player)) {
                    const int32_t additionalUpdates =
                        CalculateAdditionalHyperUpdates(speedIncreasePercent, actor, sFractionalEnemyUpdates);

                    for (int32_t i = 0; i < additionalUpdates; ++i) {
                        GameInteractor::RawAction::UpdateActor(actor);
                    }
                }
            });
    }
}

static RegisterShipInitFunc initFunc(UpdateHyperEnemiesState,
                                     { CVAR_HYPER_ENEMIES_NAME, CVAR_ENHANCEMENT("HyperEnemySpeedIncreasePercent"),
                                       CVAR_ENHANCEMENT("HyperEnemySpeedAtZeroHealthPercent") });
