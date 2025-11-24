#include "soh/telemetry/ExternalTelemetry.h"

#include <cstdint>

extern "C" ExternalTelemetry gExternalTelemetry = { 0, 0 };

extern "C" void ExternalTelemetry_SetSceneId(uint32_t sceneId) {
    gExternalTelemetry.sceneId = sceneId;
}

extern "C" void ExternalTelemetry_SetDayTime(uint32_t dayTime) {
    gExternalTelemetry.dayTime = dayTime;
}

static_assert(sizeof(ExternalTelemetry) == sizeof(uint32_t) * 2,
              "ExternalTelemetry layout must remain two packed uint32_t fields");
