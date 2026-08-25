#pragma once

#include <QString>

namespace ventilator {

class SimulationEngine;

class VentilationParameterRouter final {
public:
    [[nodiscard]] static bool apply(
        SimulationEngine& engine, const QString& parameterName, double value);

private:
    [[nodiscard]] static QString normalizedName(const QString& parameterName);
};

} // namespace ventilator