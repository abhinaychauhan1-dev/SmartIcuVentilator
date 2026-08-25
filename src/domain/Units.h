#pragma once

namespace ventilator::units {

[[nodiscard]] constexpr double litresToMillilitres(double litres) noexcept
{
    return litres * 1000.0;
}

[[nodiscard]] constexpr double litresPerSecondToLitresPerMinute(
    double litresPerSecond) noexcept
{
    return litresPerSecond * 60.0;
}

} // namespace ventilator::units