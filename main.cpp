#include "src/domain/SimulationEngine.h"
#include "src/viewmodels/DashboardViewModel.h"
#include "src/viewmodels/WaveformPlotViewModel.h"

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QVariant>

int main(int argc, char* argv[])
{
    qputenv("QT_QUICK_CONTROLS_STYLE", "Basic");

    QApplication application(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("SmartIcuVentilator"));
    QCoreApplication::setApplicationName(QStringLiteral("Smart ICU Ventilator"));

    ventilator::SimulationEngine simulationEngine;
    ventilator::WaveformPlotViewModel waveformPlotViewModel(&simulationEngine);
    ventilator::DashboardViewModel dashboardViewModel(
        &simulationEngine, &waveformPlotViewModel);

    QQmlApplicationEngine qmlEngine;
    qmlEngine.setInitialProperties({
        {QStringLiteral("dashboardViewModel"), QVariant::fromValue(&dashboardViewModel)},
        {QStringLiteral("waveformPlotViewModel"), QVariant::fromValue(&waveformPlotViewModel)}});
    qmlEngine.loadFromModule(QStringLiteral("VentilatorHMI"), QStringLiteral("Main"));

    if (qmlEngine.rootObjects().isEmpty()) {
        return EXIT_FAILURE;
    }

    return application.exec();
}