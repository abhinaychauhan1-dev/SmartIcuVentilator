/**
 * @filename    : BreathMetricsCalculator.cpp
 * @brief   : Computes breath metrics from sampled pressure, volume, and phase data.
 *
 * @author  : Abhinay Chauhan (email: abhinay.chauhan1@gmail.com)
 * @version : 1.0.0
 * Copyright (c) 2024. All rights reserved.
 */

#include "BreathMetricsCalculator.h"

#include "../domain/VentilatorConfig.h"

#include <algorithm>

namespace ventilator {

// Accumulates one telemetry sample and publishes metrics at a new-breath boundary.
BreathAnalysis BreathMetricsCalculator::process(
    double timestamp, double paw, double volume, bool isInspiration) noexcept
{
    std::optional<BreathMetrics> completedBreath;
    // An expiration-to-inspiration transition closes the previous breath.
    const bool startsNewBreath = hasTelemetry_ && isInspiration && !previousInspiration_;
    if (!hasTelemetry_) {
        reset(timestamp);
    } else if (startsNewBreath) {
        if (pressureSampleCount_ > 0) {
            completedBreath = completeBreath(timestamp);
        }
        reset(timestamp);
    }

    pressureSum_ += paw;
    ++pressureSampleCount_;
    currentPeakPressure_ = std::max(currentPeakPressure_, paw);
    currentMaximumVolume_ = std::max(currentMaximumVolume_, volume);
    if (!isInspiration) {
        currentMeasuredPeep_ = std::min(currentMeasuredPeep_, paw);
    }

    hasTelemetry_ = true;
    previousInspiration_ = isInspiration;

    const double expansionScale = 1.0 + config::maximumLungExpansion
        * std::clamp(volume / config::referenceTidalVolumeLitres, 0.0, 1.0);
    return {expansionScale, currentPeakPressure_, completedBreath};
}

// Derives respiratory rate, minute volume, and compliance from one completed breath.
BreathMetrics BreathMetricsCalculator::completeBreath(double timestamp) const noexcept
{
    const double measuredPeep = currentMeasuredPeep_ == std::numeric_limits<double>::max()
        ? 0.0
        : currentMeasuredPeep_;
    const double breathDuration = timestamp - breathStartTimestamp_;
    const double respiratoryRate = breathDuration > 0.0 ? 60.0 / breathDuration : 0.0;
    const double drivingPressure = currentPeakPressure_ - measuredPeep;
    const double dynamicCompliance = drivingPressure > 0.0
        ? currentMaximumVolume_ / drivingPressure
        : config::healthyCompliance;
    // Normalize compliance from healthy (0) to stiff (1) for UI visualization.
    const double complianceIndex = std::clamp(
        (config::healthyCompliance - dynamicCompliance)
            / (config::healthyCompliance - config::stiffCompliance),
        0.0,
        1.0);

    return {
        currentPeakPressure_,
        pressureSum_ / static_cast<double>(pressureSampleCount_),
        measuredPeep,
        currentMaximumVolume_,
        currentMaximumVolume_ * respiratoryRate,
        respiratoryRate,
        complianceIndex};
}

    // Clears per-breath accumulators while preserving stream transition state.
void BreathMetricsCalculator::reset(double timestamp) noexcept
{
    breathStartTimestamp_ = timestamp;
    pressureSum_ = 0.0;
    currentPeakPressure_ = 0.0;
    currentMeasuredPeep_ = std::numeric_limits<double>::max();
    currentMaximumVolume_ = 0.0;
    pressureSampleCount_ = 0;
}

} // namespace ventilator