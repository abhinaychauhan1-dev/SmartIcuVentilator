/**
 * @filename    : AlarmEvaluator.cpp
 * @brief   : Evaluates ventilator telemetry against configured alarm thresholds.
 *
 * @author  : Abhinay Chauhan (email: abhinay.chauhan1@gmail.com)
 * @version : 1.0.0
 * Copyright (c) 2024. All rights reserved.
 */

#include "AlarmEvaluator.h"

#include "../domain/VentilatorConfig.h"

namespace ventilator {

// Selects the highest-priority alarm raised by the current measurement snapshot.
AlarmState AlarmEvaluator::evaluate(const AlarmInputs& inputs)
{
    // Return on the first match to enforce descending clinical priority.
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