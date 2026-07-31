#pragma once

#include <cstdint>

#include "soh/cvar_prefixes.h"

struct Actor;

namespace HyperSpeed {

inline constexpr const char* FullHealthCVar = CVAR_ENHANCEMENT("HyperEnemySpeedIncreasePercent");
inline constexpr const char* ZeroHealthCVar = CVAR_ENHANCEMENT("HyperEnemySpeedAtZeroHealthPercent");

inline constexpr int32_t FullHealthDefault = 100;
inline constexpr int32_t ZeroHealthDefault = 100;

enum class TrackerGroup {
    Bosses,
    Enemies,
};

struct Health {
    int32_t current;
    int32_t maximum;
};

int32_t GetFullHealthIncreasePercent();
int32_t GetZeroHealthIncreasePercent();
bool HasConfiguredExtraUpdates();
int32_t CalculateAdditionalUpdates(TrackerGroup group, Actor* actor);
int32_t CalculateAdditionalUpdates(TrackerGroup group, Actor* actor, Health health);

void Erase(Actor* actor);
void Reset(TrackerGroup group);
void ResetAll();

} // namespace HyperSpeed
