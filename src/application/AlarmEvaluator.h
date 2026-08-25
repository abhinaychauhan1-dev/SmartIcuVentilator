#pragma once

#include <QString>

namespace ventilator {

struct AlarmInputs {
    double currentPressure;
    double breathPeakPressure;
    double tidalVolume;
    double measuredPeep;
    bool hasCompletedBreath;
};

struct AlarmState {
    int priority;
    QString text;
};

class AlarmEvaluator final {
public:
    [[nodiscard]] static AlarmState evaluate(const AlarmInputs& inputs);
};

} // namespace ventilator