#pragma once

// This header exposes a small POD that external tools (such as ReShade addons)
// can locate via the unmangled C symbol `gExternalTelemetry`.
// Layout (8 bytes total, standard alignment):
//   uint32_t sceneId;  // Active scene id
//   uint32_t dayTime;  // Current in-game dayTime value
// Both fields use little-endian ordering on supported platforms and are kept
// in sync by the game code whenever either value changes.

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ExternalTelemetry {
    uint32_t sceneId;
    uint32_t dayTime;
} ExternalTelemetry;

extern ExternalTelemetry gExternalTelemetry;

void ExternalTelemetry_SetSceneId(uint32_t sceneId);
void ExternalTelemetry_SetDayTime(uint32_t dayTime);

#ifdef __cplusplus
}
#endif

