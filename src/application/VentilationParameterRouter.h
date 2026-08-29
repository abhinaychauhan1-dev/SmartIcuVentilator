/**
 * @filename    : VentilationParameterRouter.h
 * @brief   : Declares routing of UI parameter changes to the simulation engine.
 *
 * @author  : Abhinay Chauhan (email: abhinay.chauhan1@gmail.com)
 * @version : 1.0.0
 * Copyright (c) 2024. All rights reserved.
 */

#pragma once

#include <QString>

namespace ventilator {

class SimulationEngine;

// Translates loosely formatted UI names into strongly typed engine setters.
class VentilationParameterRouter final {
public:
    [[nodiscard]] static bool apply(
        SimulationEngine& engine, const QString& parameterName, double value);

private:
    [[nodiscard]] static QString normalizedName(const QString& parameterName);
};

} // namespace ventilator