#pragma once

namespace jive
{
    template <typename T>
    [[nodiscard]] static inline auto interpolate(T start, T end, double proportion)
    {
        static_assert(std::is_arithmetic<T>());

        if constexpr (std::is_integral<T>())
            return static_cast<T>(std::round(start + (end - start) * proportion));
        else
            return static_cast<T>(start + (end - start) * proportion);
    }
} // namespace jive
