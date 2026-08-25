#pragma once

#include <cstddef>

namespace ventilator::config {

inline constexpr double waveformWindowSeconds = 10.0;
inline constexpr std::size_t waveformBufferCapacity = 1000;
inline constexpr std::size_t loopBufferCapacity = 6000;

inline constexpr double referenceTidalVolumeLitres = 0.5;
inline constexpr double maximumLungExpansion = 0.3;
inline constexpr double healthyCompliance = 0.05;
inline constexpr double stiffCompliance = 0.015;

inline constexpr double highPressureAlarmThreshold = 40.0;
inline constexpr double lowTidalVolumeAlarmThreshold = 0.2;
inline constexpr double lowPeepAlarmThreshold = 3.0;

} // namespace ventilator::config