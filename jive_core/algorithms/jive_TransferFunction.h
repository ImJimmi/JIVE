#pragma once

namespace jive
{
    template <typename FloatingPoint>
    [[nodiscard]] static auto solveFor(const juce::Path& transferFunction,
                                       FloatingPoint x)
    {
        static_assert(std::is_floating_point<FloatingPoint>());

        x = juce::jlimit<FloatingPoint>(0, 1, x);

        // Given transfer function is undefined at the given x position!
        jassert(transferFunction.intersectsLine({ { static_cast<float>(x), 0.0f }, { static_cast<float>(x), 1.0f } }));

        juce::PathFlatteningIterator iterator{ transferFunction, {}, 0.001f };

        while (iterator.next())
        {
            if (iterator.x2 < x)
                continue;

            const auto proportion = (x - iterator.x1) / (iterator.x2 - iterator.x1);
            return static_cast<FloatingPoint>(iterator.y2 * proportion + iterator.y1 * (1.0 - proportion));
        }

        jassertfalse;
        return x;
    }
} // namespace jive
