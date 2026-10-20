#pragma once

#include <jive_core/components/accessibility/jive_IgnoredComponent.h>
#include <jive_core/geometry/jive_BorderRadii.h>
#include <jive_core/graphics/jive_Fill.h>

namespace jive
{
    class BackgroundCanvas : public IgnoredComponent
    {
    public:
        BackgroundCanvas();

        void paint(juce::Graphics& g) final;
        void resized() final;

        [[nodiscard]] Fill getFill() const;
        void setFill(const Fill& newFill);

        [[nodiscard]] Fill getBorderFill() const;
        void setBorderFill(const Fill& newFill);

        [[nodiscard]] float getBorderWidth() const;
        void setBorderWidth(float newWidth);

        [[nodiscard]] BorderRadii<float> getBorderRadii() const;
        void setBorderRadii(BorderRadii<float> radii);

    private:
        void updateShape();

        Fill background{ juce::Colours::transparentBlack };
        Fill borderFill{ juce::Colours::transparentBlack };
        float borderWidth{ 0.0f };
        BorderRadii<float> borderRadii;

        juce::Path shape;

        JUCE_LEAK_DETECTOR(BackgroundCanvas)
    };
} // namespace jive
