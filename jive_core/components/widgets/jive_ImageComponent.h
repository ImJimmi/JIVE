#pragma once

#include <jive_core/compile-time-helpers/jive_JuceVersion.h>
#include <jive_core/graphics/jive_LookAndFeel.h>
#include <juce_gui_basics/juce_gui_basics.h>

namespace jive
{
    class ImageComponent : public juce::Component
    {
    public:
        ImageComponent();

        void paint(juce::Graphics& g) override;
        void setDrawable(std::unique_ptr<juce::Drawable>&& d,
                         juce::Rectangle<float> intrinsicBounds = {},
                         juce::Rectangle<float> viewBox = {});

        void resized() override;

        [[nodiscard]] float getMinimumRequiredWidth() const;
        [[nodiscard]] float getMinimumRequiredHeight() const;

    private:
        std::unique_ptr<juce::Drawable> drawable;

#if JIVE_JUCE_9
        std::unique_ptr<juce::DrawableComponent> drawableComponent;
#endif

        juce::Rectangle<float> intrinsicBounds;
        juce::Rectangle<float> viewBox;

        std::vector<juce::DrawableShape*> currentColourFills;
        std::vector<juce::DrawableShape*> currentColourStrokes;

        JUCE_LEAK_DETECTOR(ImageComponent)
    };
} // namespace jive
