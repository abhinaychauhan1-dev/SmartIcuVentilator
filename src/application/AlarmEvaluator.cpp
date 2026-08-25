#include "AlarmEvaluator.h"

#include "../domain/VentilatorConfig.h"

namespace ventilator {

AlarmState AlarmEvaluator::evaluate(const AlarmInputs& inputs)
{
    if (inputs.currentPressure >= config::highPressureAlarmThreshold
        || inputs.breathPeakPressure >= config::highPressureAlarmThreshold) {
        return {3, QStringLiteral("HIGH AIRWAY PRESSURE")};
    }

    if (inputs.hasCompletedBreath
        && inputs.tidalVolume < config::lowTidalVolumeAlarmThreshold) {
        return {2, QStringLiteral("LOW TIDAL VOLUME")};
    }

    if (inputs.hasCompletedBreath && inputs.measuredPeep < config::lowPeepAlarmThreshold) {
        return {1, QStringLiteral("LOW PEEP")};
    }

    return {0, {}};
}

} // namespace ventilator