#pragma once

#include "jive_Slider.h"

namespace jive
{
    class Knob : public Slider
    {
    public:
        explicit Knob(std::unique_ptr<GuiItem> itemToDecorate);

    private:
        juce::Slider::SliderStyle getStyleForOrientation(Orientation orientation) final;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Knob)
    };
} // namespace jive
