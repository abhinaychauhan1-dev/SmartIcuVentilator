#pragma once

#include <QObject>
#include <QPointF>

#include "../domain/VentilatorConfig.h"

#include <array>
#include <cstddef>

namespace ventilator {

class SimulationEngine;

class WaveformPlotViewModel final : public QObject {
    Q_OBJECT

public:
    explicit WaveformPlotViewModel(SimulationEngine* engine, QObject* parent = nullptr);

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
        bool isInspiration);

private:
    static constexpr std::size_t waveformCapacity = config::waveformBufferCapacity;
    static constexpr std::size_t loopCapacity = config::loopBufferCapacity;

    void storeWaveformPoint(double x, double paw, double flow, double volume) noexcept;
    void storeLoopPoint(double volume, double paw, double flow, bool isInspiration) noexcept;

    SimulationEngine* engine_;

    std::array<QPointF, waveformCapacity> pawPoints_{};
    std::array<QPointF, waveformCapacity> flowPoints_{};
    std::array<QPointF, waveformCapacity> volumePoints_{};
    std::size_t waveformHead_{0};
    std::size_t waveformCount_{0};

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