/**
 * @filename    : VentilatorConfig.h
 * @brief   : Defines shared simulation limits, capacities, and alarm thresholds.
 *
 * @author  : Abhinay Chauhan (email: abhinay.chauhan1@gmail.com)
 * @version : 1.0.0
 * Copyright (c) 2024. All rights reserved.
 */

#pragma once

#include <cstddef>

namespace ventilator::config {

// Display history and fixed-buffer capacities.
inline constexpr double waveformWindowSeconds = 10.0;
inline constexpr std::size_t waveformBufferCapacity = 1000;
inline constexpr std::size_t loopBufferCapacity = 6000;

// Reference physiology used by simulation and visual normalization.
inline constexpr double referenceTidalVolumeLitres = 0.5;
inline constexpr double maximumLungExpansion = 0.3;
inline constexpr double healthyCompliance = 0.05;
inline constexpr double stiffCompliance = 0.015;

// Alarm limits are ordered independently by AlarmEvaluator priority.
inline constexpr double highPressureAlarmThreshold = 40.0;
inline constexpr double lowTidalVolumeAlarmThreshold = 0.2;
inline constexpr double lowPeepAlarmThreshold = 3.0;

} // namespace ventilator::config