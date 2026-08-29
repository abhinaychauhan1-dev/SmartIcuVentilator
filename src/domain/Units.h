/**
 * @filename    : Units.h
 * @brief   : Provides unit conversion helpers for respiratory measurements.
 *
 * @author  : Abhinay Chauhan (email: abhinay.chauhan1@gmail.com)
 * @version : 1.0.0
 * Copyright (c) 2024. All rights reserved.
 */

#pragma once

// Centralized conversions keep simulation values in SI-scale litres internally.
namespace ventilator::units {

// Converts a volume from litres to millilitres.
[[nodiscard]] constexpr double litresToMillilitres(double litres) noexcept
{
    return litres * 1000.0;
}

// Converts flow from litres per second to litres per minute.
[[nodiscard]] constexpr double litresPerSecondToLitresPerMinute(
    double litresPerSecond) noexcept
{
    return litresPerSecond * 60.0;
}

} // namespace ventilator::units