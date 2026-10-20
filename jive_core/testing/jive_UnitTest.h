#pragma once

#include <juce_core/juce_core.h>

#include <utility>

namespace jive
{
    class UnitTest : public juce::UnitTest
    {
    public:
        UnitTest(const juce::String& categoryName,
                 const juce::String& unitName,
                 const juce::String& testName,
                 std::function<void(UnitTest&)> test)
            : juce::UnitTest{ unitName, categoryName }
            , runName{ testName }
            , run{ std::move(test) }
        {
        }

        void runTest() final
        {
            beginTest(runName);
            run(*this);
        }

    private:
        const juce::String runName;
        std::function<void(UnitTest&)> run;
    };
} // namespace jive
