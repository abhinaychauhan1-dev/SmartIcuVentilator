/**
 * @filename    : DashboardViewModel.h
 * @brief   : Declares dashboard state, metrics, alarms, and UI commands.
 *
 * @author  : Abhinay Chauhan (email: abhinay.chauhan1@gmail.com)
 * @version : 1.0.0
 * Copyright (c) 2024. All rights reserved.
 */

#pragma once

#include <QObject>
#include <QString>
#include <QTimer>

#include "../application/BreathMetricsCalculator.h"

namespace ventilator {

class SimulationEngine;
class WaveformPlotViewModel;

// QML-facing coordinator for measured data, controls, and alarm presentation.
class DashboardViewModel final : public QObject {
    Q_OBJECT
    Q_PROPERTY(double peakPressure READ peakPressure NOTIFY metricsChanged)
    Q_PROPERTY(double meanPressure READ meanPressure NOTIFY metricsChanged)
    Q_PROPERTY(double measuredPeep READ measuredPeep NOTIFY metricsChanged)
    Q_PROPERTY(double tidalVolume READ tidalVolume NOTIFY metricsChanged)
    Q_PROPERTY(double minuteVolume READ minuteVolume NOTIFY metricsChanged)
    Q_PROPERTY(double respiratoryRate READ respiratoryRate NOTIFY metricsChanged)
    Q_PROPERTY(double measuredFio2 READ measuredFio2 NOTIFY metricsChanged)
    Q_PROPERTY(double lungExpansionScale READ lungExpansionScale NOTIFY metricsChanged)
    Q_PROPERTY(double complianceIndex READ complianceIndex NOTIFY metricsChanged)
    Q_PROPERTY(int highestAlarmPriority READ highestAlarmPriority NOTIFY alarmStateChanged)
    Q_PROPERTY(QString activeAlarmText READ activeAlarmText NOTIFY alarmStateChanged)
    Q_PROPERTY(int silenceSecondsRemaining READ silenceSecondsRemaining NOTIFY silenceChanged)

public:
    explicit DashboardViewModel(SimulationEngine* engine, QObject* parent = nullptr);
    DashboardViewModel(
        SimulationEngine* engine,
        WaveformPlotViewModel* telemetrySource,
        QObject* parent = nullptr);

    [[nodiscard]] double peakPressure() const noexcept;
    [[nodiscard]] double meanPressure() const noexcept;
    [[nodiscard]] double measuredPeep() const noexcept;
    [[nodiscard]] double tidalVolume() const noexcept;
    [[nodiscard]] double minuteVolume() const noexcept;
    [[nodiscard]] double respiratoryRate() const noexcept;
    [[nodiscard]] double measuredFio2() const noexcept;
    [[nodiscard]] double lungExpansionScale() const noexcept;
    [[nodiscard]] double complianceIndex() const noexcept;
    [[nodiscard]] int highestAlarmPriority() const noexcept;
    [[nodiscard]] QString activeAlarmText() const;
    [[nodiscard]] int silenceSecondsRemaining() const noexcept;

    void setTelemetrySource(WaveformPlotViewModel* telemetrySource);

    Q_INVOKABLE void silenceAudioAlarm(int seconds);
    Q_INVOKABLE void setVentilationParameter(const QString& name, double value);

signals:
    void metricsChanged();
    void alarmStateChanged();
    void silenceChanged();

private slots:
    void ingestTelemetry(
        double timestamp,
        double paw,
        double flow,
        double volume,
        double fio2,
        bool isInspiration);

private:
    void applyCompletedBreath(const BreathMetrics& metrics);
    void updateAlarmState(double currentPaw, double currentPeakPressure);
    void synchronizeAudioAlarm(bool soundImmediately);

    SimulationEngine* engine_;
    WaveformPlotViewModel* telemetrySource_{nullptr};
    QMetaObject::Connection telemetryConnection_;
    QTimer silenceTimer_;
    QTimer alarmAudioTimer_;

    // Last completed-breath metrics exposed as stable dashboard values.
    double peakPressure_{0.0};
    double meanPressure_{0.0};
    double measuredPeep_{0.0};
    double tidalVolume_{0.0};
    double minuteVolume_{0.0};
    double respiratoryRate_{0.0};
    double measuredFio2_{40.0};
    double lungExpansionScale_{1.0};
    double complianceIndex_{0.0};

    // Alarm presentation state is updated independently from breath metrics.
    int highestAlarmPriority_{0};
    QString activeAlarmText_;
    int silenceSecondsRemaining_{0};

    BreathMetricsCalculator metricsCalculator_;
    bool hasCompletedBreath_{false};
    bool hasBreathAudioState_{false};
    bool previousAudioInspiration_{false};
};

} // namespace ventilator