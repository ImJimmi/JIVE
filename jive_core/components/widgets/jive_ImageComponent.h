#pragma once

#include <jive_core/graphics/jive_LookAndFeel.h>
#include <juce_gui_basics/juce_gui_basics.h>

namespace jive
{
    class ImageComponent : public juce::Component
    {
    public:
        ImageComponent();

        void paint(juce::Graphics& g) override;
        void setDrawable(std::unique_ptr<juce::Drawable>&& d);

        void resized() override;

        [[nodiscard]] float getMinimumRequiredWidth() const;
        [[nodiscard]] float getMinimumRequiredHeight() const;

    private:
        std::unique_ptr<juce::Drawable> drawable;
    };
} // namespace jive
