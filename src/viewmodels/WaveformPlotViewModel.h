/**
 * @filename    : WaveformPlotViewModel.h
 * @brief   : Declares buffered waveform and loop data exposed to QML charts.
 *
 * @author  : Abhinay Chauhan (email: abhinay.chauhan1@gmail.com)
 * @version : 1.0.0
 * Copyright (c) 2024. All rights reserved.
 */

#pragma once

#include <QObject>
#include <QPointF>

#include "../domain/VentilatorConfig.h"

#include <array>
#include <cstddef>

namespace ventilator {

class SimulationEngine;

// Adapts queued telemetry into bounded Qt Graphs series and respiratory loops.
class WaveformPlotViewModel final : public QObject {
    Q_OBJECT

public:
    explicit WaveformPlotViewModel(SimulationEngine* engine, QObject* parent = nullptr);

    void resetHistory() noexcept;

    Q_INVOKABLE void renderSweepBatch(
        QObject* pawSeries,
        QObject* flowSeries,
        QObject* volSeries);
    Q_INVOKABLE void renderActiveLoops(QObject* pvSeries, QObject* fvSeries);
    Q_INVOKABLE void freezeReferenceLoop(QObject* pvSeries, QObject* fvSeries);

signals:
    void telemetrySample(
        double timestamp,
        double paw,
        double flow,
        double volume,
        double fio2,
        bool isInspiration);
    void waveformsUpdated();

private:
    static constexpr std::size_t waveformCapacity = config::waveformBufferCapacity;
    static constexpr std::size_t loopCapacity = config::loopBufferCapacity;

    void storeWaveformPoint(double x, double paw, double flow, double volume) noexcept;
    void storeLoopPoint(double volume, double paw, double flow, bool isInspiration) noexcept;

    SimulationEngine* engine_;

    // Circular history avoids allocations while retaining the latest sweep window.
    std::array<QPointF, waveformCapacity> pawPoints_{};
    std::array<QPointF, waveformCapacity> flowPoints_{};
    std::array<QPointF, waveformCapacity> volumePoints_{};
    std::size_t waveformHead_{0};
    std::size_t waveformCount_{0};

    // Separate loop snapshots support live, completed, and frozen-reference traces.
    std::array<QPointF, loopCapacity> currentPvLoop_{};
    std::array<QPointF, loopCapacity> currentFvLoop_{};
    std::array<QPointF, loopCapacity> completedPvLoop_{};
    std::array<QPointF, loopCapacity> completedFvLoop_{};
    std::array<QPointF, loopCapacity> referencePvLoop_{};
    std::array<QPointF, loopCapacity> referenceFvLoop_{};
    std::size_t currentLoopCount_{0};
    std::size_t completedLoopCount_{0};
    std::size_t referenceLoopCount_{0};
    bool hasBreathState_{false};
    bool previousInspiration_{false};
};

} // namespace ventilator