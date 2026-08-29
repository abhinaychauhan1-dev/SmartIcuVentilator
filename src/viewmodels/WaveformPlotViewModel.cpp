/**
 * @filename    : WaveformPlotViewModel.cpp
 * @brief   : Buffers telemetry and updates waveform and respiratory loop series.
 *
 * @author  : Abhinay Chauhan (email: abhinay.chauhan1@gmail.com)
 * @version : 1.0.0
 * Copyright (c) 2024. All rights reserved.
 */

#include "WaveformPlotViewModel.h"

#include "../domain/SimulationEngine.h"
#include "../domain/Units.h"

#include <QList>
#include <QXYSeries>

#include <algorithm>

namespace ventilator {

// Binds the plot adapter to the telemetry-producing simulation engine.
WaveformPlotViewModel::WaveformPlotViewModel(SimulationEngine* engine, QObject* parent)
    : QObject(parent)
    , engine_(engine)
{
}

// Clears live and completed histories before a new treatment setting takes effect.
void WaveformPlotViewModel::resetHistory() noexcept
{
    waveformHead_ = 0;
    waveformCount_ = 0;
    currentLoopCount_ = 0;
    completedLoopCount_ = 0;
    hasBreathState_ = false;
    previousInspiration_ = false;
}

// Drains telemetry and replaces all three time-series charts as one batch.
void WaveformPlotViewModel::renderSweepBatch(
    QObject* pawSeriesObject,
    QObject* flowSeriesObject,
    QObject* volSeriesObject)
{
    auto* pawSeries = qobject_cast<QXYSeries*>(pawSeriesObject);
    auto* flowSeries = qobject_cast<QXYSeries*>(flowSeriesObject);
    auto* volSeries = qobject_cast<QXYSeries*>(volSeriesObject);
    if (engine_ == nullptr || pawSeries == nullptr || flowSeries == nullptr
        || volSeries == nullptr) {
        return;
    }

    bool receivedSamples = false;
    TelemetryFrame frame{};
    // Drain the queue in one UI update so all charts share the same sample batch.
    while (engine_->tryPopTelemetry(frame)) {
        const double flowLitresPerMinute =
            units::litresPerSecondToLitresPerMinute(frame.flow);
        const double volumeMillilitres = units::litresToMillilitres(frame.volume);
        storeWaveformPoint(
            frame.timestamp, frame.paw, flowLitresPerMinute, volumeMillilitres);
        storeLoopPoint(frame.volume, frame.paw, frame.flow, frame.isInspiration);

        receivedSamples = true;
        emit telemetrySample(
            frame.timestamp,
            frame.paw,
            frame.flow,
            frame.volume,
            frame.fio2,
            frame.isInspiration);
    }

    if (!receivedSamples || waveformCount_ == 0) {
        return;
    }

    QList<QPointF> pawHistory;
    QList<QPointF> flowHistory;
    QList<QPointF> volumeHistory;
    const auto historyCapacity = static_cast<qsizetype>(waveformCount_ + 1);
    pawHistory.reserve(historyCapacity);
    flowHistory.reserve(historyCapacity);
    volumeHistory.reserve(historyCapacity);

    const std::size_t oldestIndex =
        (waveformHead_ + waveformCapacity - waveformCount_) % waveformCapacity;
    const std::size_t newestIndex =
        (waveformHead_ + waveformCapacity - 1) % waveformCapacity;
    const double newestTimestamp = pawPoints_[newestIndex].x();

    // Rebuild circular storage as elapsed sweep time, newest at zero seconds.
    for (std::size_t offset = 0; offset < waveformCount_; ++offset) {
        const std::size_t index = (oldestIndex + offset) % waveformCapacity;
        const double x = newestTimestamp - pawPoints_[index].x();
        if (x > config::waveformWindowSeconds) {
            continue;
        }

        pawHistory.append(QPointF{x, pawPoints_[index].y()});
        flowHistory.append(QPointF{x, flowPoints_[index].y()});
        volumeHistory.append(QPointF{x, volumePoints_[index].y()});
    }

    // Qt Graphs requires ascending X values for continuous line rendering.
    auto sortByX = [](const QPointF& a, const QPointF& b) { return a.x() < b.x(); };
    std::sort(pawHistory.begin(), pawHistory.end(), sortByX);
    std::sort(flowHistory.begin(), flowHistory.end(), sortByX);
    std::sort(volumeHistory.begin(), volumeHistory.end(), sortByX);

    pawSeries->replace(pawHistory);
    flowSeries->replace(flowHistory);
    volSeries->replace(volumeHistory);

    emit waveformsUpdated();
}

// Replaces pressure-volume and flow-volume series with the current breath trace.
void WaveformPlotViewModel::renderActiveLoops(QObject* pvSeriesObject, QObject* fvSeriesObject)
{
    auto* pvSeries = qobject_cast<QXYSeries*>(pvSeriesObject);
    auto* fvSeries = qobject_cast<QXYSeries*>(fvSeriesObject);
    if (pvSeries == nullptr || fvSeries == nullptr) {
        return;
    }

    QList<QPointF> pvPoints;
    QList<QPointF> fvPoints;
    const auto loopPointCount = static_cast<qsizetype>(currentLoopCount_);
    pvPoints.reserve(loopPointCount);
    fvPoints.reserve(loopPointCount);
    for (std::size_t index = 0; index < currentLoopCount_; ++index) {
        pvPoints.append(currentPvLoop_[index]);
        fvPoints.append(currentFvLoop_[index]);
    }

    pvSeries->replace(pvPoints);
    fvSeries->replace(fvPoints);
}

// Captures a completed or active breath as an immutable visual reference.
void WaveformPlotViewModel::freezeReferenceLoop(QObject* pvSeriesObject, QObject* fvSeriesObject)
{
    auto* pvSeries = qobject_cast<QXYSeries*>(pvSeriesObject);
    auto* fvSeries = qobject_cast<QXYSeries*>(fvSeriesObject);
    if (pvSeries == nullptr || fvSeries == nullptr) {
        return;
    }

    // Prefer the last complete breath so a reference cannot change mid-cycle.
    const bool hasCompletedLoop = completedLoopCount_ > 0;
    referenceLoopCount_ = hasCompletedLoop ? completedLoopCount_ : currentLoopCount_;
    const auto& sourcePv = hasCompletedLoop ? completedPvLoop_ : currentPvLoop_;
    const auto& sourceFv = hasCompletedLoop ? completedFvLoop_ : currentFvLoop_;
    std::copy_n(sourcePv.begin(), referenceLoopCount_, referencePvLoop_.begin());
    std::copy_n(sourceFv.begin(), referenceLoopCount_, referenceFvLoop_.begin());

    QList<QPointF> pvPoints;
    QList<QPointF> fvPoints;
    const auto loopPointCount = static_cast<qsizetype>(referenceLoopCount_);
    pvPoints.reserve(loopPointCount);
    fvPoints.reserve(loopPointCount);
    for (std::size_t index = 0; index < referenceLoopCount_; ++index) {
        pvPoints.append(referencePvLoop_[index]);
        fvPoints.append(referenceFvLoop_[index]);
    }

    pvSeries->replace(pvPoints);
    fvSeries->replace(fvPoints);
}

// Appends one sample to each synchronized circular waveform buffer.
void WaveformPlotViewModel::storeWaveformPoint(
    double x, double paw, double flow, double volume) noexcept
{
    pawPoints_[waveformHead_] = QPointF{x, paw};
    flowPoints_[waveformHead_] = QPointF{x, flow};
    volumePoints_[waveformHead_] = QPointF{x, volume};
    waveformHead_ = (waveformHead_ + 1) % waveformCapacity;
    waveformCount_ = std::min(waveformCount_ + 1, waveformCapacity);
}

// Appends loop coordinates and rolls the trace at a new inspiration.
void WaveformPlotViewModel::storeLoopPoint(
    double volume, double paw, double flow, bool isInspiration) noexcept
{
    // Roll the active trace into the completed snapshot at each new inspiration.
    if (hasBreathState_ && isInspiration && !previousInspiration_) {
        completedLoopCount_ = currentLoopCount_;
        std::copy_n(currentPvLoop_.begin(), completedLoopCount_, completedPvLoop_.begin());
        std::copy_n(currentFvLoop_.begin(), completedLoopCount_, completedFvLoop_.begin());
        currentLoopCount_ = 0;
    }

    if (currentLoopCount_ < loopCapacity) {
        const double volumeMillilitres = units::litresToMillilitres(volume);
        currentPvLoop_[currentLoopCount_] = QPointF{paw, volumeMillilitres};
        currentFvLoop_[currentLoopCount_] = QPointF{
            volumeMillilitres, units::litresPerSecondToLitresPerMinute(flow)};
        ++currentLoopCount_;
    }

    hasBreathState_ = true;
    previousInspiration_ = isInspiration;
}

} // namespace ventilator