#include <jive_layouts/jive_layouts.h>

namespace jive
{
    NormalisedProgressBar::NormalisedProgressBar()
        : juce::ProgressBar{ value }
    {
    }

    void NormalisedProgressBar::setValue(double normalisedValue)
    {
        jassert(normalisedValue >= 0.0 && normalisedValue <= 1.0);
        value = normalisedValue;
    }

    double NormalisedProgressBar::getValue() const
    {
        return value;
    }
} // namespace jive
