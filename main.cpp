/**
 * @filename    : main.cpp
 * @brief   : Initializes the ventilator application and exposes view models to QML.
 *
 * @author  : Abhinay Chauhan (email: abhinay.chauhan1@gmail.com)
 * @version : 1.0.0
 * Copyright (c) 2024. All rights reserved.
 */

#include "src/domain/SimulationEngine.h"
#include "src/viewmodels/DashboardViewModel.h"
#include "src/viewmodels/WaveformPlotViewModel.h"

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QVariant>

// Creates the backend object graph, loads the QML module, and starts the event loop.
int main(int argc, char* argv[])
{
    qputenv("QT_QUICK_CONTROLS_STYLE", "Basic");

    QApplication application(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("SmartIcuVentilator"));
    QCoreApplication::setApplicationName(QStringLiteral("Smart ICU Ventilator"));

    // Keep the simulation and view models alive for the full QML engine lifetime.
    ventilator::SimulationEngine simulationEngine;
    ventilator::WaveformPlotViewModel waveformPlotViewModel(&simulationEngine);
    ventilator::DashboardViewModel dashboardViewModel(
        &simulationEngine, &waveformPlotViewModel);

    // Inject backend objects before constructing the root QML component.
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