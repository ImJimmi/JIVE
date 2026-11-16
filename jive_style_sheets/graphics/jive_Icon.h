#pragma once

#include "jive_Fill.h"

#include <jive_core/algorithms/jive_Interpolate.h>

#include <juce_gui_basics/juce_gui_basics.h>

namespace jive
{
    // An SVG or image drawn as part of a component, like the arrow that shows
    // a combo box can be opened. Copies share the same drawable.
    class Icon
    {
    public:
        Icon() = default;

        // Takes either SVG markup, or the full path of an SVG or image file.
        // If neither can be loaded, the icon will be empty.
        explicit Icon(const juce::String& source);

        [[nodiscard]] const juce::String& getSource() const;
        [[nodiscard]] bool isEmpty() const;

        // The width of the icon relative to its height.
        [[nodiscard]] float getAspectRatio() const;

        // Draws the icon to fit within the given area, using the given colour
        // for any of the SVG's currentColor. A fill or stroke then replaces the
        // colour of whichever of the icon's fills or strokes are visible.
        void draw(juce::Graphics& g,
                  juce::Rectangle<float> area,
                  juce::Colour currentColour,
                  const std::optional<Fill>& fill,
                  const std::optional<Fill>& stroke) const;

        [[nodiscard]] bool operator==(const Icon& other) const;
        [[nodiscard]] bool operator!=(const Icon& other) const;

    private:
        juce::String source;
        std::shared_ptr<const juce::Drawable> drawable;
        juce::Rectangle<float> intrinsicBounds;
        bool usesCurrentColour = false;
    };

    template <>
    struct Interpolate<Icon>
    {
        [[nodiscard]] Icon operator()(const Icon&, const Icon& end, double) const
        {
            return end;
        }
    };
} // namespace jive

namespace juce
{
    template <>
    class VariantConverter<jive::Icon>
    {
    public:
        static jive::Icon fromVar(const var& v);
        static var toVar(const jive::Icon& icon);
    };
} // namespace juce
