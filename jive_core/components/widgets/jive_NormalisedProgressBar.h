#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace jive
{
    class NormalisedProgressBar : public juce::ProgressBar
    {
    public:
        NormalisedProgressBar();

        void setValue(double normalisedValue);
        [[nodiscard]] double getValue() const;

    private:
        double value{ 0.0 };

        JUCE_LEAK_DETECTOR(NormalisedProgressBar)
    };
} // namespace jive
