#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace jive
{
    class NormalisedProgressBar : public juce::ProgressBar
    {
    public:
        NormalisedProgressBar();

        void setValue(double normalisedValue);
        double getValue() const;

    private:
        double value{ 0.0 };

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NormalisedProgressBar)
    };
} // namespace jive
