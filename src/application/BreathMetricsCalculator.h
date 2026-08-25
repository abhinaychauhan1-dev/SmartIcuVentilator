#pragma once

#include <cstddef>
#include <limits>
#include <optional>

namespace ventilator {

struct BreathMetrics {
    double peakPressure;
    double meanPressure;
    double measuredPeep;
    double tidalVolume;
    double minuteVolume;
    double respiratoryRate;
    double complianceIndex;
};

struct BreathAnalysis {
    double lungExpansionScale;
    double currentPeakPressure;
    std::optional<BreathMetrics> completedBreath;
};

class BreathMetricsCalculator final {
public:
    [[nodiscard]] BreathAnalysis process(
        double timestamp, double paw, double volume, bool isInspiration) noexcept;

private:
    [[nodiscard]] BreathMetrics completeBreath(double timestamp) const noexcept;
    void reset(double timestamp) noexcept;

    double breathStartTimestamp_{0.0};
    double pressureSum_{0.0};
    double currentPeakPressure_{0.0};
    double currentMeasuredPeep_{std::numeric_limits<double>::max()};
    double currentMaximumVolume_{0.0};
    std::size_t pressureSampleCount_{0};
    bool hasTelemetry_{false};
    bool previousInspiration_{false};
};

} // namespace ventilator