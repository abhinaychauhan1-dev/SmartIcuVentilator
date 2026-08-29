/**
 * @filename    : SimulationEngine.cpp
 * @brief   : Generates simulated respiratory waveforms and telemetry frames.
 *
 * @author  : Abhinay Chauhan (email: abhinay.chauhan1@gmail.com)
 * @version : 1.0.0
 * Copyright (c) 2024. All rights reserved.
 */

#include "SimulationEngine.h"

#include <algorithm>
#include <chrono>
#include <cmath>

namespace ventilator {
namespace {

constexpr auto simulationInterval = std::chrono::milliseconds{10};
constexpr double minimumPositiveValue = 1.0e-6;

// Clamps finite settings and substitutes a safe default for invalid values.
[[nodiscard]] double boundedParameter(
    double value, double minimum, double maximum, double fallback) noexcept
{
    return std::isfinite(value) ? std::clamp(value, minimum, maximum) : fallback;
}

} // namespace

// Starts the simulation worker, which stops automatically with the jthread.
SimulationEngine::SimulationEngine()
    : worker_([this](std::stop_token stopToken) { run(stopToken); })
{
}

// Transfers the oldest queued telemetry frame to the UI consumer.
bool SimulationEngine::tryPopTelemetry(TelemetryFrame& frame) noexcept
{
    return telemetry_.tryPop(frame);
}

// Reports frames discarded because the telemetry queue was full.
std::uint64_t SimulationEngine::droppedFrameCount() const noexcept
{
    return droppedFrames_.load(std::memory_order_relaxed);
}

// Atomically updates the active ventilation mode for the worker thread.
void SimulationEngine::setMode(VentilationMode mode) noexcept
{
    mode_.exchange(mode, std::memory_order_relaxed);
}

// Atomically updates the requested positive end-expiratory pressure.
void SimulationEngine::setPeep(double peep) noexcept
{
    peep_.exchange(peep, std::memory_order_relaxed);
}

// Atomically updates the requested tidal volume in litres.
void SimulationEngine::setTidalVolume(double tidalVolume) noexcept
{
    tidalVolume_.exchange(tidalVolume, std::memory_order_relaxed);
}

// Atomically updates the requested respiratory rate in breaths per minute.
void SimulationEngine::setRate(double rate) noexcept
{
    rate_.exchange(rate, std::memory_order_relaxed);
}

// Atomically updates the inspiratory phase duration in seconds.
void SimulationEngine::setInspiratoryTime(double inspiratoryTime) noexcept
{
    inspiratoryTime_.exchange(inspiratoryTime, std::memory_order_relaxed);
}

// Atomically updates the requested oxygen concentration percentage.
void SimulationEngine::setFio2(double fio2) noexcept
{
    fio2_.exchange(fio2, std::memory_order_relaxed);
}

// Atomically updates respiratory-system compliance.
void SimulationEngine::setCompliance(double compliance) noexcept
{
    compliance_.exchange(compliance, std::memory_order_relaxed);
}

// Atomically updates airway resistance.
void SimulationEngine::setResistance(double resistance) noexcept
{
    resistance_.exchange(resistance, std::memory_order_relaxed);
}

// Generates respiratory telemetry on a fixed cadence until cancellation.
void SimulationEngine::run(std::stop_token stopToken) noexcept
{
    using Clock = std::chrono::steady_clock;

    // Advance from a fixed schedule to avoid accumulating processing-time drift.
    const auto startTime = Clock::now();
    auto nextTick = startTime;

    while (!stopToken.stop_requested()) {
        const auto now = Clock::now();
        const double timestamp = std::chrono::duration<double>(now - startTime).count();

        const double peep = boundedParameter(
            peep_.load(std::memory_order_relaxed), 0.0, 30.0, 5.0);
        const double tidalVolume = boundedParameter(
            tidalVolume_.load(std::memory_order_relaxed), 0.05, 2.0, 0.5);
        const double rate = boundedParameter(
            rate_.load(std::memory_order_relaxed), 1.0, 60.0, 12.0);
        const double fio2 = boundedParameter(
            fio2_.load(std::memory_order_relaxed), 21.0, 100.0, 40.0);
        const double compliance = boundedParameter(
            compliance_.load(std::memory_order_relaxed), 0.005, 0.2, 0.05);
        const double resistance = boundedParameter(
            resistance_.load(std::memory_order_relaxed), 1.0, 100.0, 10.0);

        const double cycleDuration = 60.0 / rate;
        const double maximumInspiratoryTime = std::max(
            minimumPositiveValue,
            cycleDuration - std::chrono::duration<double>(simulationInterval).count());
        const double inspiratoryTime = boundedParameter(
            inspiratoryTime_.load(std::memory_order_relaxed),
            minimumPositiveValue,
            maximumInspiratoryTime,
            std::min(1.0, maximumInspiratoryTime));
        const double phase = std::fmod(timestamp, cycleDuration);
        const bool isInspiration = phase < inspiratoryTime;
        // The RC time constant controls filling and passive expiratory decay.
        const double timeConstant = resistance * compliance;

        double volume = 0.0;
        double flow = 0.0;

        if (mode_.load(std::memory_order_relaxed) == VentilationMode::VolumeControl) {
            const double inspiratoryResponse =
                1.0 - std::exp(-inspiratoryTime / timeConstant);
            const double pressureStep = tidalVolume
                / (compliance * std::max(inspiratoryResponse, minimumPositiveValue));

            if (isInspiration) {
                const double decay = std::exp(-phase / timeConstant);
                volume = compliance * pressureStep * (1.0 - decay);
                flow = (pressureStep / resistance) * decay;
            } else {
                const double expirationTime = phase - inspiratoryTime;
                volume = tidalVolume * std::exp(-expirationTime / timeConstant);
                flow = -volume / timeConstant;
            }
        } else {
            const double inspiratoryResponse =
                1.0 - std::exp(-inspiratoryTime / timeConstant);
            const double pressureStep = tidalVolume
                / (compliance * std::max(inspiratoryResponse, minimumPositiveValue));

            if (isInspiration) {
                const double decay = std::exp(-phase / timeConstant);
                volume = compliance * pressureStep * (1.0 - decay);
                flow = (pressureStep / resistance) * decay;
            } else {
                const double expirationTime = phase - inspiratoryTime;
                volume = tidalVolume * std::exp(-expirationTime / timeConstant);
                flow = -volume / timeConstant;
            }
        }

        // Single-compartment equation of motion: elastic + resistive + PEEP.
        const double paw = (volume / compliance) + (resistance * flow) + peep;
        const TelemetryFrame frame{timestamp, paw, flow, volume, fio2, isInspiration};
        if (!telemetry_.tryPush(frame)) {
            droppedFrames_.fetch_add(1, std::memory_order_relaxed);
        }

        nextTick += simulationInterval;
        std::this_thread::sleep_until(nextTick);
    }
}

} // namespace ventilator