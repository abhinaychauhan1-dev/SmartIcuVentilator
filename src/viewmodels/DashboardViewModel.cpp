#include "DashboardViewModel.h"

#include "WaveformPlotViewModel.h"
#include "../application/AlarmEvaluator.h"
#include "../application/VentilationParameterRouter.h"
#include "../domain/SimulationEngine.h"

#include <algorithm>

namespace ventilator {

DashboardViewModel::DashboardViewModel(SimulationEngine* engine, QObject* parent)
    : QObject(parent)
    , engine_(engine)
    , silenceTimer_(this)
{
    silenceTimer_.setInterval(1000);
    connect(&silenceTimer_, &QTimer::timeout, this, [this] {
        if (silenceSecondsRemaining_ > 0) {
            --silenceSecondsRemaining_;
            emit silenceChanged();
        }
        if (silenceSecondsRemaining_ == 0) {
            silenceTimer_.stop();
        }
    });
}

DashboardViewModel::DashboardViewModel(
    SimulationEngine* engine,
    WaveformPlotViewModel* telemetrySource,
    QObject* parent)
    : DashboardViewModel(engine, parent)
{
    setTelemetrySource(telemetrySource);
}

double DashboardViewModel::peakPressure() const noexcept
{
    return peakPressure_;
}

double DashboardViewModel::meanPressure() const noexcept
{
    return meanPressure_;
}

double DashboardViewModel::measuredPeep() const noexcept
{
    return measuredPeep_;
}

double DashboardViewModel::tidalVolume() const noexcept
{
    return tidalVolume_;
}

double DashboardViewModel::minuteVolume() const noexcept
{
    return minuteVolume_;
}

double DashboardViewModel::respiratoryRate() const noexcept
{
    return respiratoryRate_;
}

double DashboardViewModel::lungExpansionScale() const noexcept
{
    return lungExpansionScale_;
}

double DashboardViewModel::complianceIndex() const noexcept
{
    return complianceIndex_;
}

int DashboardViewModel::highestAlarmPriority() const noexcept
{
    return highestAlarmPriority_;
}

QString DashboardViewModel::activeAlarmText() const
{
    return activeAlarmText_;
}

int DashboardViewModel::silenceSecondsRemaining() const noexcept
{
    return silenceSecondsRemaining_;
}

void DashboardViewModel::setTelemetrySource(WaveformPlotViewModel* telemetrySource)
{
    disconnect(telemetryConnection_);
    telemetryConnection_ = {};

    if (telemetrySource != nullptr) {
        telemetryConnection_ = connect(
            telemetrySource,
            &WaveformPlotViewModel::telemetrySample,
            this,
            &DashboardViewModel::ingestTelemetry);
    }
}

void DashboardViewModel::silenceAudioAlarm(int seconds)
{
    const int boundedSeconds = std::clamp(seconds, 0, 3600);
    if (silenceSecondsRemaining_ != boundedSeconds) {
        silenceSecondsRemaining_ = boundedSeconds;
        emit silenceChanged();
    }

    if (silenceSecondsRemaining_ > 0) {
        silenceTimer_.start();
    } else {
        silenceTimer_.stop();
    }
}

void DashboardViewModel::setVentilationParameter(const QString& name, double value)
{
    if (engine_ == nullptr) {
        return;
    }

    (void)VentilationParameterRouter::apply(*engine_, name, value);
}

void DashboardViewModel::ingestTelemetry(
    double timestamp,
    double paw,
    double flow,
    double volume,
    bool isInspiration)
{
    Q_UNUSED(flow);

    const BreathAnalysis analysis = metricsCalculator_.process(
        timestamp, paw, volume, isInspiration);
    if (!qFuzzyCompare(lungExpansionScale_, analysis.lungExpansionScale)) {
        lungExpansionScale_ = analysis.lungExpansionScale;
        emit metricsChanged();
    }

    if (analysis.completedBreath.has_value()) {
        applyCompletedBreath(*analysis.completedBreath);
    }
    updateAlarmState(paw, analysis.currentPeakPressure);
}

void DashboardViewModel::applyCompletedBreath(const BreathMetrics& metrics)
{
    peakPressure_ = metrics.peakPressure;
    meanPressure_ = metrics.meanPressure;
    measuredPeep_ = metrics.measuredPeep;
    tidalVolume_ = metrics.tidalVolume;
    minuteVolume_ = metrics.minuteVolume;
    respiratoryRate_ = metrics.respiratoryRate;
    complianceIndex_ = metrics.complianceIndex;

    hasCompletedBreath_ = true;
    emit metricsChanged();
}

void DashboardViewModel::updateAlarmState(double currentPaw, double currentPeakPressure)
{
    const AlarmState alarm = AlarmEvaluator::evaluate({
        currentPaw,
        currentPeakPressure,
        tidalVolume_,
        measuredPeep_,
        hasCompletedBreath_});

    if (highestAlarmPriority_ != alarm.priority || activeAlarmText_ != alarm.text) {
        highestAlarmPriority_ = alarm.priority;
        activeAlarmText_ = alarm.text;
        emit alarmStateChanged();
    }
}

} // namespace ventilator