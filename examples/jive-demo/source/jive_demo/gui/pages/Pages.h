#pragma once

#include <juce_core/juce_core.h>

namespace jive_demo
{
    enum class Page
    {
        home = 0,
        layouts,
        styleSheets,
        development,
        animations,

        numPages,
    };

    [[nodiscard]] static auto next(Page current) noexcept
    {
        const auto nextPage = static_cast<Page>(static_cast<int>(current) + 1);

        if (nextPage == Page::numPages)
            return static_cast<Page>(0);

        return nextPage;
    }

    [[nodiscard]] static auto previous(Page current) noexcept
    {
        return static_cast<Page>(juce::jmax(0, static_cast<int>(current) - 1));
    }
} // namespace jive_demo
