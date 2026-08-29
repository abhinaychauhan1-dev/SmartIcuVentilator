#include "DashboardViewModel.h"

#include "WaveformPlotViewModel.h"
#include "../application/AlarmEvaluator.h"
#include "../application/VentilationParameterRouter.h"
#include "../domain/SimulationEngine.h"

#include <QApplication>
#include <QByteArray>
#include <QDataStream>
#include <QIODevice>

#include <algorithm>
#include <cmath>
#include <numbers>

#ifdef Q_OS_WIN
#include <qt_windows.h>
#include <mmsystem.h>
#endif

namespace ventilator {
namespace {

[[nodiscard]] int alarmInterval(int priority) noexcept
{
    switch (priority) {
    case 3:
        return 500;
    case 2:
        return 1000;
    default:
        return 2000;
    }
}

[[nodiscard]] QByteArray createAlarmTone(double frequency, int durationMs)
{
    constexpr quint32 sampleRate = 44100;
    constexpr quint16 channelCount = 1;
    constexpr quint16 bitsPerSample = 16;
    constexpr quint16 blockAlign = channelCount * bitsPerSample / 8;
    constexpr double amplitude = 0.28 * 32767.0;

    const quint32 sampleCount = sampleRate * static_cast<quint32>(durationMs) / 1000;
    const quint32 dataSize = sampleCount * blockAlign;

    QByteArray waveData;
    waveData.reserve(static_cast<qsizetype>(44 + dataSize));
    QDataStream stream(&waveData, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream.writeRawData("RIFF", 4);
    stream << quint32{36 + dataSize};
    stream.writeRawData("WAVEfmt ", 8);
    stream << quint32{16} << quint16{1} << channelCount << sampleRate;
    stream << quint32{sampleRate * blockAlign} << blockAlign << bitsPerSample;
    stream.writeRawData("data", 4);
    stream << dataSize;

    for (quint32 index = 0; index < sampleCount; ++index) {
        const double time = static_cast<double>(index) / sampleRate;
        const double edge = std::min(index, sampleCount - index - 1);
        const double envelope = std::min(1.0, edge / (sampleRate * 0.01));
        const auto sample = static_cast<qint16>(
            amplitude * envelope * std::sin(2.0 * std::numbers::pi * frequency * time));
        stream << sample;
    }

    return waveData;
}

void playAlarmTone(int priority)
{
#ifdef Q_OS_WIN
    static const QByteArray lowPriorityTone = createAlarmTone(660.0, 140);
    static const QByteArray mediumPriorityTone = createAlarmTone(880.0, 160);
    static const QByteArray highPriorityTone = createAlarmTone(1100.0, 180);

    const QByteArray& tone = priority >= 3
        ? highPriorityTone
        : priority == 2 ? mediumPriorityTone : lowPriorityTone;
    PlaySoundA(tone.constData(), nullptr, SND_MEMORY | SND_ASYNC | SND_NODEFAULT);
#else
    QApplication::beep();
#endif
}

void playBreathTone()
{
#ifdef Q_OS_WIN
    static const QByteArray breathTone = createAlarmTone(520.0, 90);
    PlaySoundA(
        breathTone.constData(), nullptr, SND_MEMORY | SND_ASYNC | SND_NODEFAULT);
#else
    QApplication::beep();
#endif
}

} // namespace

DashboardViewModel::DashboardViewModel(SimulationEngine* engine, QObject* parent)
    : QObject(parent)
    , engine_(engine)
    , silenceTimer_(this)
    , alarmAudioTimer_(this)
{
    silenceTimer_.setInterval(1000);
    connect(&silenceTimer_, &QTimer::timeout, this, [this] {
        if (silenceSecondsRemaining_ > 0) {
            --silenceSecondsRemaining_;
            emit silenceChanged();
        }
        if (silenceSecondsRemaining_ == 0) {
            silenceTimer_.stop();
            synchronizeAudioAlarm(true);
        }
    });

    connect(&alarmAudioTimer_, &QTimer::timeout, this, [this] {
        if (highestAlarmPriority_ > 0 && silenceSecondsRemaining_ == 0) {
            playAlarmTone(highestAlarmPriority_);
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

double DashboardViewModel::measuredFio2() const noexcept
{
    return measuredFio2_;
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
    telemetrySource_ = telemetrySource;

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
        alarmAudioTimer_.stop();
    } else {
        silenceTimer_.stop();
        synchronizeAudioAlarm(true);
    }
}

void DashboardViewModel::setVentilationParameter(const QString& name, double value)
{
    if (engine_ == nullptr) {
        return;
    }

    if (VentilationParameterRouter::apply(*engine_, name, value)
        && telemetrySource_ != nullptr) {
        telemetrySource_->resetHistory();
    }
}

void DashboardViewModel::ingestTelemetry(
    double timestamp,
    double paw,
    double flow,
    double volume,
    double fio2,
    bool isInspiration)
{
    Q_UNUSED(flow);

    if (!qFuzzyCompare(measuredFio2_, fio2)) {
        measuredFio2_ = fio2;
        emit metricsChanged();
    }

    const bool startsNewBreath = hasBreathAudioState_
        && isInspiration
        && !previousAudioInspiration_;
    hasBreathAudioState_ = true;
    previousAudioInspiration_ = isInspiration;
    if (startsNewBreath
        && highestAlarmPriority_ == 0
        && silenceSecondsRemaining_ == 0) {
        playBreathTone();
    }

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
        synchronizeAudioAlarm(true);
    }
}

void DashboardViewModel::synchronizeAudioAlarm(bool soundImmediately)
{
    if (highestAlarmPriority_ == 0 || silenceSecondsRemaining_ > 0) {
        alarmAudioTimer_.stop();
        return;
    }

    alarmAudioTimer_.start(alarmInterval(highestAlarmPriority_));
    if (soundImmediately) {
        playAlarmTone(highestAlarmPriority_);
    }
}

} // namespace ventilator