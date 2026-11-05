#pragma once

#include <jive_core/compile-time-helpers/jive_JuceVersion.h>
#include <jive_style_sheets/graphics/jive_LookAndFeel.h>
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

        // Lets the elements of an inline SVG be styled like components. Each
        // shape's fill and stroke in the given markup is replaced with a
        // marker colour, which is swapped for the element's styles when it's
        // drawn. Call before parsing the markup, with an invalid tree for
        // anything other than an inline SVG.
        void setSVGElements(juce::ValueTree svg);

        void resized() override;

        struct MarkedShape
        {
            std::optional<std::size_t> fillElement;
            std::optional<std::size_t> strokeElement;
            float fillOpacity;
            float strokeOpacity;
        };

        struct ElementStyles
        {
            Fill fill;
            Fill stroke;
        };

        [[nodiscard]] float getMinimumRequiredWidth() const;
        [[nodiscard]] float getMinimumRequiredHeight() const;

    private:
        [[nodiscard]] std::vector<ElementStyles> resolveElementStyles(const LookAndFeel& laf,
                                                                      const std::optional<Fill>& svgFill,
                                                                      const std::optional<Fill>& svgStroke,
                                                                      const Fill& currentColour) const;

        std::unique_ptr<juce::Drawable> drawable;

#if JIVE_JUCE_9
        std::unique_ptr<juce::DrawableComponent> drawableComponent;
#endif

        juce::Rectangle<float> intrinsicBounds;
        juce::Rectangle<float> viewBox;

        std::vector<juce::DrawableShape*> currentColourFills;
        std::vector<juce::DrawableShape*> currentColourStrokes;

        struct SVGElement
        {
            std::unique_ptr<juce::Component> standIn;
            juce::String markupFill;
            juce::String markupStroke;
        };

        std::vector<SVGElement> svgElements;
        std::unordered_map<juce::DrawableShape*, MarkedShape> markedShapes;
    };
} // namespace jive
