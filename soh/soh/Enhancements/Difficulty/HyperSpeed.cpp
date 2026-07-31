#include "HyperSpeed.h"

#include <algorithm>
#include <cmath>
#include <unordered_map>

#include <libultraship/bridge/consolevariablebridge.h>

#include "functions.h"
#include "soh/Enhancements/game-interactor/GameInteractor_Hooks.h"
#include "soh/ShipInit.hpp"

namespace {

struct ActorUpdateState {
    int32_t maxHealth = 0;
    int32_t updateRemainderPercent = 0;
};

using Tracker = std::unordered_map<Actor*, ActorUpdateState>;

Tracker sBossTracker;
Tracker sEnemyTracker;
std::unordered_map<uint32_t, int32_t> sInitialEnemyHealth;
std::unordered_map<uint16_t, int32_t> sInitialEnemyHealthById;

Tracker& GetTracker(HyperSpeed::TrackerGroup group) {
    return group == HyperSpeed::TrackerGroup::Bosses ? sBossTracker : sEnemyTracker;
}

int32_t GetCurrentHealth(const Actor* actor) {
    const int32_t rawHealth = actor->colChkInfo.health;

    // Actor health is stored as a u8, but vanilla damage code commonly detects death by interpreting an
    // underflowed value as signed. Treat that representation as zero instead of mistaking 0xFF for a heal.
    return rawHealth <= 0x7F ? rawHealth : 0;
}

uint32_t GetEnemyHealthKey(const Actor* actor) {
    return (static_cast<uint32_t>(static_cast<uint16_t>(actor->id)) << 16) |
           static_cast<uint16_t>(actor->params);
}

void RememberEnemyHealth(Actor* actor, int32_t health) {
    if ((actor == nullptr) || (actor->category != ACTORCAT_ENEMY) || (health <= 0)) {
        return;
    }

    int32_t& storedHealth = sInitialEnemyHealth[GetEnemyHealthKey(actor)];
    storedHealth = std::max(storedHealth, health);

    int32_t& storedTypeHealth = sInitialEnemyHealthById[static_cast<uint16_t>(actor->id)];
    storedTypeHealth = std::max(storedTypeHealth, health);
}

void RememberInitialEnemyHealth(Actor* actor) {
    RememberEnemyHealth(actor, actor != nullptr ? GetCurrentHealth(actor) : 0);
}

int32_t GetInitialEnemyHealth(const Actor* actor) {
    auto it = sInitialEnemyHealth.find(GetEnemyHealthKey(actor));
    if (it != sInitialEnemyHealth.end()) {
        return it->second;
    }

    auto typeIt = sInitialEnemyHealthById.find(static_cast<uint16_t>(actor->id));
    return typeIt != sInitialEnemyHealthById.end() ? typeIt->second : 0;
}

float GetHealthRatio(ActorUpdateState& state, int32_t currentHealth, int32_t configuredMaxHealth) {
    if (configuredMaxHealth > 0) {
        state.maxHealth = configuredMaxHealth;
        if ((state.maxHealth <= 0x7F) && (currentHealth > 0x7F) && (currentHealth <= 0xFF)) {
            currentHealth = 0;
        }
    } else if ((state.maxHealth == 0) || (currentHealth > state.maxHealth)) {
        state.maxHealth = std::max(currentHealth, 1);
    }

    currentHealth = std::clamp(currentHealth, 0, state.maxHealth);
    const float healthRatio = static_cast<float>(currentHealth) / static_cast<float>(state.maxHealth);
    return std::clamp(healthRatio, 0.0f, 1.0f);
}

void RegisterHyperSpeedLifecycleHooks() {
    COND_HOOK(OnActorInit, true,
              [](void* refActor) { RememberInitialEnemyHealth(static_cast<Actor*>(refActor)); });
    COND_HOOK(OnActorDestroy, true, [](void* refActor) { HyperSpeed::Erase(static_cast<Actor*>(refActor)); });
    COND_HOOK(OnSceneInit, true, [](int16_t) {
        HyperSpeed::ResetAll();
        sInitialEnemyHealth.clear();
        sInitialEnemyHealthById.clear();
    });
    COND_HOOK(OnLoadGame, true, [](int32_t) {
        HyperSpeed::ResetAll();
        sInitialEnemyHealth.clear();
        sInitialEnemyHealthById.clear();
    });
}

static RegisterShipInitFunc initFunc(RegisterHyperSpeedLifecycleHooks);

} // namespace

namespace HyperSpeed {

int32_t GetFullHealthIncreasePercent() {
    return std::clamp(CVarGetInteger(FullHealthCVar, FullHealthDefault), 0, 400);
}

int32_t GetZeroHealthIncreasePercent() {
    return std::clamp(CVarGetInteger(ZeroHealthCVar, ZeroHealthDefault), -100, 400);
}

bool HasConfiguredExtraUpdates() {
    return std::max(GetFullHealthIncreasePercent(), GetZeroHealthIncreasePercent()) > 0;
}

int32_t CalculateAdditionalUpdatesInternal(TrackerGroup group, Actor* actor, Health health) {
    if (actor == nullptr || actor->update == nullptr) {
        Erase(actor);
        return 0;
    }

    ActorUpdateState& state = GetTracker(group)[actor];
    const int32_t fullHealthIncrease = GetFullHealthIncreasePercent();
    const int32_t zeroHealthIncrease = GetZeroHealthIncreasePercent();
    const float missingHealthRatio = 1.0f - GetHealthRatio(state, health.current, health.maximum);
    const float interpolatedIncrease =
        static_cast<float>(fullHealthIncrease) +
        (static_cast<float>(zeroHealthIncrease - fullHealthIncrease) * missingHealthRatio);
    const int32_t speedIncreasePercent =
        std::clamp(static_cast<int32_t>(std::round(interpolatedIncrease)), -100, 400);

    if (speedIncreasePercent <= 0) {
        state.updateRemainderPercent = 0;
        return 0;
    }

    int32_t additionalUpdates = speedIncreasePercent / 100;
    state.updateRemainderPercent += speedIncreasePercent % 100;
    if (state.updateRemainderPercent >= 100) {
        additionalUpdates++;
        state.updateRemainderPercent -= 100;
    }

    return additionalUpdates;
}

int32_t CalculateAdditionalUpdates(TrackerGroup group, Actor* actor) {
    const int32_t currentHealth = actor != nullptr ? GetCurrentHealth(actor) : 0;
    int32_t maximumHealth = (actor != nullptr && group == TrackerGroup::Enemies) ? GetInitialEnemyHealth(actor) : 0;
    if ((group == TrackerGroup::Enemies) && (currentHealth > maximumHealth)) {
        RememberEnemyHealth(actor, currentHealth);
        maximumHealth = currentHealth;
    }

    return CalculateAdditionalUpdatesInternal(group, actor, { currentHealth, maximumHealth });
}

int32_t CalculateAdditionalUpdates(TrackerGroup group, Actor* actor, Health health) {
    return CalculateAdditionalUpdatesInternal(group, actor, health);
}

void Erase(Actor* actor) {
    sBossTracker.erase(actor);
    sEnemyTracker.erase(actor);
}

void Reset(TrackerGroup group) {
    GetTracker(group).clear();
}

void ResetAll() {
    sBossTracker.clear();
    sEnemyTracker.clear();
}

} // namespace HyperSpeed
