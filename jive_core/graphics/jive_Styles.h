#pragma once

#include "jive_Fill.h"
#include "jive_Shadow.h"

#include <jive_core/geometry/jive_BorderRadii.h>
#include <jive_core/values/variant-converters/jive_VariantConvertion.h>

#include <juce_graphics/juce_graphics.h>

namespace jive
{
    // A set of styles that might be applied to a component.
    // Each of the styles defaults to being undefined so you can bevery explicit
    // about what you want to define. When receiving a Styles object, you should
    // decide on a sensible default if a style is undefined, or use some other
    // sort of error handling in that case.
    struct Styles
    {
        std::optional<Fill> accent;                                        // "accent"
        std::optional<Fill> background;                                    // "background"
        std::optional<Fill> borderFill;                                    // "border"
        std::optional<BorderRadii<float>> borderRadius;                    // "border-radius"
        std::optional<juce::BorderSize<float>> borderWidth;                // "border-width"
        std::optional<juce::AttributedString::ReadingDirection> direction; // "direction"
        std::optional<Fill> fill;                                          // "fill"
#if JUCE_MAJOR_VERSION >= 8
        std::optional<float> fontAscentOverride;  // "font-ascent-override"
        std::optional<float> fontDescentOverride; // "font-descent-override"
#endif
        std::optional<float> fontExtraKerningFactor;                   // "font-kerning"
        std::optional<juce::String> fontFamily;                        // "font-family"
        std::optional<float> fontHorizontalScale;                      // "font-scale"
        std::optional<float> fontPointSize;                            // "font-size"
        std::unordered_set<juce::Font::FontStyleFlags> fontStyleFlags; // "font-styles"
        std::optional<Fill> foreground;                                // "foreground"
        std::optional<Shadow> shadow;                                  // "shadow"
        std::optional<Fill> stroke;                                    // "stroke"
        std::optional<juce::Justification> textAlign;                  // "text-align"
        std::optional<Fill> thumb;                                     // "thumb"
        std::optional<Fill> track;                                     // "track"

        static inline const std::unordered_set<juce::String> propertyNames{
            "accent",
            "background",
            "border",
            "border-radius",
            "border-width",
            "direction",
            "fill",
#if JUCE_MAJOR_VERSION >= 8
            "font-ascent-override",
            "font-descent-override",
#endif
            "font-kerning",
            "font-family",
            "font-scale",
            "font-size",
            "font-styles",
            "foreground",
            "shadow",
            "stroke",
            "text-align",
            "thumb",
            "track",
        };

        [[nodiscard]] Styles& with(const juce::Identifier& propertyName,
                                   const juce::var& value)
        {
            if (propertyName == juce::Identifier{ "accent" })
                return withAccent(fromVar<Fill>(value));
            if (propertyName == juce::Identifier{ "background" })
                return withBackground(fromVar<Fill>(value));
            if (propertyName == juce::Identifier{ "border" })
                return withBorderFill(fromVar<Fill>(value));
            if (propertyName == juce::Identifier{ "border-radius" })
                return withBorderRadius(fromVar<BorderRadii<float>>(value));
            if (propertyName == juce::Identifier{ "border-width" })
                return withBorderWidth(fromVar<juce::BorderSize<float>>(value));
            if (propertyName == juce::Identifier{ "direction" })
                return withReadingDirection(fromVar<juce::AttributedString::ReadingDirection>(value));
            if (propertyName == juce::Identifier{ "fill" })
                return withFill(fromVar<Fill>(value));
#if JUCE_MAJOR_VERSION >= 8
            if (propertyName == juce::Identifier{ "font-ascent-override" })
                return withFontAscentOverride(fromVar<float>(value));
            if (propertyName == juce::Identifier{ "font-descent-override" })
                return withFontDescentOverride(fromVar<float>(value));
#endif
            if (propertyName == juce::Identifier{ "font-kerning" })
                return withFontExtraKerningFactor(fromVar<float>(value));
            if (propertyName == juce::Identifier{ "font-family" })
                return withFontFamily(value.toString());
            if (propertyName == juce::Identifier{ "font-scale" })
                return withFontHorizontalScale(fromVar<float>(value));
            if (propertyName == juce::Identifier{ "font-size" })
                return withFontPointSize(fromVar<float>(value));
            if (propertyName == juce::Identifier{ "font-styles" })
                return withFontStyleFlags(fromVar<std::unordered_set<juce::Font::FontStyleFlags>>(value));
            if (propertyName == juce::Identifier{ "foreground" })
                return withForeground(fromVar<Fill>(value));
            if (propertyName == juce::Identifier{ "shadow" })
                return withShadow(fromVar<Shadow>(value));
            if (propertyName == juce::Identifier{ "stroke" })
                return withStroke(fromVar<Fill>(value));
            if (propertyName == juce::Identifier{ "text-align" })
                return withTextAlignment(fromVar<juce::Justification>(value));
            if (propertyName == juce::Identifier{ "thumb" })
                return withThumb(fromVar<Fill>(value));
            if (propertyName == juce::Identifier{ "track" })
                return withTrack(fromVar<Fill>(value));

            jassertfalse;
            return *this;
        }

        [[nodiscard]] Styles& withAccent(const Fill& value)
        {
            accent = value;
            return *this;
        }

        [[nodiscard]] Styles& withBackground(const Fill& value)
        {
            background = value;
            return *this;
        }

        [[nodiscard]] Styles& withBorderFill(const Fill& value)
        {
            borderFill = value;
            return *this;
        }

        [[nodiscard]] Styles& withBorderRadius(const BorderRadii<float>& value)
        {
            borderRadius = value;
            return *this;
        }

        [[nodiscard]] Styles& withBorderWidth(const juce::BorderSize<float>& value)
        {
            borderWidth = value;
            return *this;
        }

        [[nodiscard]] Styles& withReadingDirection(juce::AttributedString::ReadingDirection value)
        {
            direction = value;
            return *this;
        }

        [[nodiscard]] Styles& withFill(const Fill& value)
        {
            fill = value;
            return *this;
        }

#if JUCE_MAJOR_VERSION >= 8
        [[nodiscard]] Styles& withFontAscentOverride(float value)
        {
            fontAscentOverride = value;
            return *this;
        }

        [[nodiscard]] Styles& withFontDescentOverride(float value)
        {
            fontDescentOverride = value;
            return *this;
        }
#endif

        [[nodiscard]] Styles& withFontExtraKerningFactor(float value)
        {
            fontExtraKerningFactor = value;
            return *this;
        }

        [[nodiscard]] Styles& withFontFamily(const juce::String& value)
        {
            fontFamily = value;
            return *this;
        }

        [[nodiscard]] Styles& withFontHorizontalScale(float value)
        {
            fontHorizontalScale = value;
            return *this;
        }

        [[nodiscard]] Styles& withFontPointSize(float value)
        {
            fontPointSize = value;
            return *this;
        }

        [[nodiscard]] Styles& withFontStyleFlags(const std::unordered_set<juce::Font::FontStyleFlags>& value)
        {
            fontStyleFlags = value;
            return *this;
        }

        [[nodiscard]] Styles& withForeground(const Fill& value)
        {
            foreground = value;
            return *this;
        }

        [[nodiscard]] Styles& withShadow(const Shadow& value)
        {
            shadow = value;
            return *this;
        }

        [[nodiscard]] Styles& withStroke(const Fill& value)
        {
            stroke = value;
            return *this;
        }

        [[nodiscard]] Styles& withTextAlignment(juce::Justification value)
        {
            textAlign = value;
            return *this;
        }

        [[nodiscard]] Styles& withThumb(const Fill& value)
        {
            thumb = value;
            return *this;
        }

        [[nodiscard]] Styles& withTrack(const Fill& value)
        {
            track = value;
            return *this;
        }

        // Returns a style in this object with the given name, or nullptr if no
        // style with that name exists. The comments next to the properties
        // indicate the name that should be passed to this function.
        // Passing an incorrect name will cause an assertion failure and return
        // nullptr.
        template <typename T>
        [[nodiscard]] std::optional<T> find(const juce::String& styleName) const;

        template <>
        [[nodiscard]] std::optional<Fill> find(const juce::String& styleName) const
        {
            if (styleName == "accent")
                return accent;
            if (styleName == "background")
                return background;
            if (styleName == "border")
                return borderFill;
            if (styleName == "fill")
                return fill;
            if (styleName == "foreground")
                return foreground;
            if (styleName == "stroke")
                return stroke;
            if (styleName == "thumb")
                return thumb;
            if (styleName == "track")
                return track;

            jassertfalse;
            return std::nullopt;
        }

        template <>
        [[nodiscard]] std::optional<juce::BorderSize<float>> find(const juce::String& styleName) const
        {
            if (styleName == "border-width")
                return borderWidth;

            jassertfalse;
            return std::nullopt;
        }

        template <>
        [[nodiscard]] std::optional<BorderRadii<float>> find(const juce::String& styleName) const
        {
            if (styleName == "border-radius")
                return borderRadius;

            jassertfalse;
            return std::nullopt;
        }

        template <>
        [[nodiscard]] std::optional<juce::String> find(const juce::String& styleName) const
        {
            if (styleName == "font-family")
                return fontFamily;

            jassertfalse;
            return std::nullopt;
        }

        template <>
        [[nodiscard]] std::optional<float> find(const juce::String& styleName) const
        {
            if (styleName == "font-size")
                return fontPointSize;
            if (styleName == "font-scale")
                return fontHorizontalScale;
            if (styleName == "font-kerning")
                return fontExtraKerningFactor;

#if JUCE_MAJOR_VERSION
            if (styleName == "font-ascent-override")
                return fontAscentOverride;
            if (styleName == "font-descent-override")
                return fontDescentOverride;
#endif

            jassertfalse;
            return std::nullopt;
        }

        template <>
        [[nodiscard]] std::optional<std::unordered_set<juce::Font::FontStyleFlags>> find(const juce::String& styleName) const
        {
            if (styleName == "font-styles")
                return fontStyleFlags;

            jassertfalse;
            return std::nullopt;
        }

        template <>
        [[nodiscard]] std::optional<Shadow> find(const juce::String& styleName) const
        {
            if (styleName == "shadow")
                return shadow;

            jassertfalse;
            return std::nullopt;
        }

        template <>
        [[nodiscard]] std::optional<juce::AttributedString::ReadingDirection> find(const juce::String& styleName) const
        {
            if (styleName == "direction")
                return direction;

            jassertfalse;
            return std::nullopt;
        }

        template <>
        [[nodiscard]] std::optional<juce::Justification> find(const juce::String& styleName) const
        {
            if (styleName == "text-align")
                return textAlign;

            jassertfalse;
            return std::nullopt;
        }
    };
} // namespace jive
