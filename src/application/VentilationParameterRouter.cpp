#include "VentilationParameterRouter.h"

#include "../domain/SimulationEngine.h"

#include <cmath>

namespace ventilator {

bool VentilationParameterRouter::apply(
    SimulationEngine& engine, const QString& parameterName, double value)
{
    if (!std::isfinite(value)) {
        return false;
    }

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
        const auto mode = value < 0.5
            ? SimulationEngine::VentilationMode::VolumeControl
            : SimulationEngine::VentilationMode::PressureControl;
        engine.setMode(mode);
    } else {
        return false;
    }

    return true;
}

QString VentilationParameterRouter::normalizedName(const QString& parameterName)
{
    QString name = parameterName.trimmed().toLower();
    name.remove(QLatin1Char{'_'});
    name.remove(QLatin1Char{'-'});
    name.remove(QLatin1Char{' '});
    return name;
}

} // namespace ventilator