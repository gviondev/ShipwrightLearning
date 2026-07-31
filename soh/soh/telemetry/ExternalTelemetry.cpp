#include "soh/telemetry/ExternalTelemetry.h"

#include <cstdint>

extern "C" {

EXTERNAL_TELEMETRY_API ExternalTelemetry gExternalTelemetry = { 0, 0 };

EXTERNAL_TELEMETRY_API void ExternalTelemetry_SetSceneId(uint32_t sceneId) {
    gExternalTelemetry.sceneId = sceneId;
}

EXTERNAL_TELEMETRY_API void ExternalTelemetry_SetDayTime(uint32_t dayTime) {
    gExternalTelemetry.dayTime = dayTime;
}

}

static_assert(sizeof(ExternalTelemetry) == sizeof(uint32_t) * 2,
              "ExternalTelemetry layout must remain two packed uint32_t fields");
