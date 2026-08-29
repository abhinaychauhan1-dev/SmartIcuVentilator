/**
 * @filename    : AlarmEvaluator.h
 * @brief   : Declares alarm inputs, state, and clinical alarm evaluation behavior.
 *
 * @author  : Abhinay Chauhan (email: abhinay.chauhan1@gmail.com)
 * @version : 1.0.0
 * Copyright (c) 2024. All rights reserved.
 */

#pragma once

#include <QString>

namespace ventilator {

// Snapshot of measurements required for one deterministic alarm decision.
struct AlarmInputs {
    double currentPressure;
    double breathPeakPressure;
    double tidalVolume;
    double measuredPeep;
    bool hasCompletedBreath;
};

// Highest active priority and its operator-facing message.
struct AlarmState {
    int priority;
    QString text;
};

// Evaluates alarms without retaining state so priority ordering stays explicit.
class AlarmEvaluator final {
public:
    [[nodiscard]] static AlarmState evaluate(const AlarmInputs& inputs);
};

} // namespace ventilator