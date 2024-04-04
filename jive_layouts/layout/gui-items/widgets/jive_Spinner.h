#pragma once

#include "jive_Slider.h"

namespace jive
{
    class Spinner : public Slider
    {
    public:
        explicit Spinner(std::unique_ptr<GuiItem> itemToDecorate);

    private:
        juce::Slider::SliderStyle getStyleForOrientation(Orientation orientation) final;

        Property<bool> draggable;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Spinner)
    };
} // namespace jive
