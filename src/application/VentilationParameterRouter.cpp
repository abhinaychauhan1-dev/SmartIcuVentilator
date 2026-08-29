/**
 * @filename    : VentilationParameterRouter.cpp
 * @brief   : Validates and applies named ventilation parameter updates.
 *
 * @author  : Abhinay Chauhan (email: abhinay.chauhan1@gmail.com)
 * @version : 1.0.0
 * Copyright (c) 2024. All rights reserved.
 */

#include "VentilationParameterRouter.h"

#include "../domain/SimulationEngine.h"

#include <cmath>

namespace ventilator {

// Validates and routes a named numeric parameter to the matching engine setter.
bool VentilationParameterRouter::apply(
    SimulationEngine& engine, const QString& parameterName, double value)
{
    if (!std::isfinite(value)) {
        return false;
    }

    // Accept display labels and API-style names through one canonical key.
    const QString name = normalizedName(parameterName);
    if (name == QStringLiteral("peep")) {
        engine.setPeep(value);
    } else if (name == QStringLiteral("tidalvolume")) {
        engine.setTidalVolume(value);
    } else if (name == QStringLiteral("rate")
               || name == QStringLiteral("respiratoryrate")) {
        engine.setRate(value);
    } else if (name == QStringLiteral("inspiratorytime")) {
        engine.setInspiratoryTime(value);
    } else if (name == QStringLiteral("fio2")) {
        engine.setFio2(value);
    } else if (name == QStringLiteral("compliance")) {
        engine.setCompliance(value);
    } else if (name == QStringLiteral("resistance")) {
        engine.setResistance(value);
    } else if (name == QStringLiteral("mode")) {
        // The numeric boundary provides a stable QML-friendly mode contract.
        const auto mode = value < 0.5
            ? SimulationEngine::VentilationMode::VolumeControl
            : SimulationEngine::VentilationMode::PressureControl;
        engine.setMode(mode);
    } else {
        return false;
    }

    return true;
}

// Removes display formatting so equivalent parameter labels share one routing key.
QString VentilationParameterRouter::normalizedName(const QString& parameterName)
{
    QString name = parameterName.trimmed().toLower();
    name.remove(QLatin1Char{'_'});
    name.remove(QLatin1Char{'-'});
    name.remove(QLatin1Char{' '});
    return name;
}

} // namespace ventilator