#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <thread>
#include <type_traits>

namespace ventilator {

struct TelemetryFrame {
    double timestamp;
    double paw;
    double flow;
    double volume;
    double fio2;
    bool isInspiration;
};

class SimulationEngine final {
public:
    enum class VentilationMode : std::uint8_t {
        VolumeControl,
        PressureControl
    };

    static constexpr std::size_t telemetryCapacity = 1024;

    SimulationEngine();
    ~SimulationEngine() = default;

    SimulationEngine(const SimulationEngine&) = delete;
    SimulationEngine& operator=(const SimulationEngine&) = delete;
    SimulationEngine(SimulationEngine&&) = delete;
    SimulationEngine& operator=(SimulationEngine&&) = delete;

    [[nodiscard]] bool tryPopTelemetry(TelemetryFrame& frame) noexcept;
    [[nodiscard]] std::uint64_t droppedFrameCount() const noexcept;

    void setMode(VentilationMode mode) noexcept;
    void setPeep(double peep) noexcept;
    void setTidalVolume(double tidalVolume) noexcept;
    void setRate(double rate) noexcept;
    void setInspiratoryTime(double inspiratoryTime) noexcept;
    void setFio2(double fio2) noexcept;
    void setCompliance(double compliance) noexcept;
    void setResistance(double resistance) noexcept;

private:
    template<typename T, std::size_t Capacity>
    class SpscRingBuffer final {
        static_assert(Capacity > 1);
        static_assert(std::is_trivially_copyable_v<T>);
        static_assert(std::atomic<std::size_t>::is_always_lock_free);

    public:
        [[nodiscard]] bool tryPush(const T& item) noexcept
        {
            const auto head = head_.load(std::memory_order_relaxed);
            if (head - tail_.load(std::memory_order_acquire) == Capacity) {
                return false;
            }

            storage_[head % Capacity] = item;
            head_.store(head + 1, std::memory_order_release);
            return true;
        }

        [[nodiscard]] bool tryPop(T& item) noexcept
        {
            const auto tail = tail_.load(std::memory_order_relaxed);
            if (tail == head_.load(std::memory_order_acquire)) {
                return false;
            }

            item = storage_[tail % Capacity];
            tail_.store(tail + 1, std::memory_order_release);
            return true;
        }

    private:
        std::array<T, Capacity> storage_{};
        alignas(64) std::atomic<std::size_t> head_{0};
        alignas(64) std::atomic<std::size_t> tail_{0};
    };

    void run(std::stop_token stopToken) noexcept;

    SpscRingBuffer<TelemetryFrame, telemetryCapacity> telemetry_;
    std::atomic<std::uint64_t> droppedFrames_{0};

    std::atomic<VentilationMode> mode_{VentilationMode::VolumeControl};
    std::atomic<double> peep_{5.0};
    std::atomic<double> tidalVolume_{0.5};
    std::atomic<double> rate_{12.0};
    std::atomic<double> inspiratoryTime_{1.0};
    std::atomic<double> fio2_{40.0};
    std::atomic<double> compliance_{0.05};
    std::atomic<double> resistance_{10.0};

    std::jthread worker_;
};

} // namespace ventilator