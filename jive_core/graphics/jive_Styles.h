#pragma once

#include "jive_Fill.h"
#include "jive_Shadow.h"

#include <jive_core/algorithms/jive_Interpolate.h>
#include <jive_core/geometry/jive_BorderRadii.h>
#include <jive_core/kinetics/jive_Transitions.h>
#include <jive_core/time/jive_Timer.h>
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
        template <typename T>
        class StyleProperty
        {
        public:
            StyleProperty(const StyleProperty& other) = default;
            StyleProperty(StyleProperty&&) = default;
            StyleProperty& operator=(StyleProperty&&) = default;

            StyleProperty& operator=(const StyleProperty& other)
            {
                *const_cast<juce::Identifier*>(&id) = other.id;
                currentValue = other.currentValue;
                previousValue = other.previousValue;
                lastUpdated = other.lastUpdated;

                return *this;
            }

            explicit StyleProperty(const juce::Identifier& propertyID)
                : id{ propertyID }
            {
            }

            StyleProperty(const juce::Identifier& propertyID, const T& value)
                : id{ propertyID }
                , currentValue{ value }
                , previousValue{ value }
            {
            }

            void set(const T& newValue)
            {
                if (newValue == currentValue)
                    return;

                if (currentValue.has_value())
                    previousValue = currentValue;
                else
                    previousValue = newValue;

                currentValue = newValue;
                lastUpdated = now();
            }

            [[nodiscard]] const T& get() const
            {
                return currentValue.value();
            }

            [[nodiscard]] T& get()
            {
                return currentValue.value();
            }

            [[nodiscard]] T getValueOr(const T& fallback) const
            {
                return currentValue.value_or(fallback);
            }

            [[nodiscard]] T interpolated(double proportion) const
            {
                return interpolate(previousValue.value(), currentValue.value(), proportion);
            }

            [[nodiscard]] T interpolatedOr(double proportion, const T& fallback) const
            {
                if (currentValue.has_value() && previousValue.has_value())
                    return interpolated(proportion);

                return fallback;
            }

            [[nodiscard]] bool hasValue() const
            {
                return currentValue.has_value();
            }

            void overwriteLastUpdated(juce::Time lastUpdatedOverwrite)
            {
                lastUpdated = lastUpdatedOverwrite;
            }

            [[nodiscard]] juce::Time getLastUpdated() const
            {
                return lastUpdated;
            }

            StyleProperty& operator=(const T& other)
            {
                set(other);
                return *this;
            }

            [[nodiscard]] auto operator==(const StyleProperty<T>& other) const
            {
                return id == other.id && currentValue == other.currentValue;
            }

            const juce::Identifier id;

        private:
            std::optional<T> currentValue;
            std::optional<T> previousValue;
            juce::Time lastUpdated = now();
        };

        StyleProperty<Fill> accent{ "accent" };
        StyleProperty<Fill> background{ "background" };
        StyleProperty<Fill> borderFill{ "border" };
        StyleProperty<BorderRadii<float>> borderRadius{ "border-radius" };
        StyleProperty<juce::BorderSize<float>> borderWidth{ "border-width" };
        StyleProperty<juce::AttributedString::ReadingDirection> direction{ "direction" };
        StyleProperty<Fill> fill{ "fill" };
#if JUCE_MAJOR_VERSION >= 8
        StyleProperty<float> fontAscentOverride{ "font-ascent-override" };
        StyleProperty<float> fontDescentOverride{ "font-descent-override" };
#endif
        StyleProperty<float> fontExtraKerningFactor{ "font-kerning" };
        StyleProperty<juce::String> fontFamily{ "font-family" };
        StyleProperty<float> fontHorizontalScale{ "font-scale" };
        StyleProperty<float> fontPointSize{ "font-size" };
        StyleProperty<std::unordered_set<juce::Font::FontStyleFlags>> fontStyleFlags{ "font-styles" };
        StyleProperty<Fill> foreground{ "foreground" };
        StyleProperty<Shadow> shadow{ "shadow" };
        StyleProperty<Fill> stroke{ "stroke" };
        StyleProperty<juce::Justification> textAlign{ "text-align" };
        StyleProperty<Fill> thumb{ "thumb" };
        StyleProperty<Fill> track{ "track" };

        Transitions::ReferenceCountedPointer transitions;

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
            if (propertyName == accent.id)
                return withAccent(fromVar<Fill>(value));
            if (propertyName == background.id)
                return withBackground(fromVar<Fill>(value));
            if (propertyName == borderFill.id)
                return withBorderFill(fromVar<Fill>(value));
            if (propertyName == borderRadius.id)
                return withBorderRadius(fromVar<BorderRadii<float>>(value));
            if (propertyName == borderWidth.id)
                return withBorderWidth(fromVar<juce::BorderSize<float>>(value));
            if (propertyName == direction.id)
                return withReadingDirection(fromVar<juce::AttributedString::ReadingDirection>(value));
            if (propertyName == fill.id)
                return withFill(fromVar<Fill>(value));
#if JUCE_MAJOR_VERSION >= 8
            if (propertyName == fontAscentOverride.id)
                return withFontAscentOverride(fromVar<float>(value));
            if (propertyName == fontDescentOverride.id)
                return withFontDescentOverride(fromVar<float>(value));
#endif
            if (propertyName == fontExtraKerningFactor.id)
                return withFontExtraKerningFactor(fromVar<float>(value));
            if (propertyName == fontFamily.id)
                return withFontFamily(value.toString());
            if (propertyName == fontHorizontalScale.id)
                return withFontHorizontalScale(fromVar<float>(value));
            if (propertyName == fontPointSize.id)
                return withFontPointSize(fromVar<float>(value));
            if (propertyName == fontStyleFlags.id)
                return withFontStyleFlags(fromVar<std::unordered_set<juce::Font::FontStyleFlags>>(value));
            if (propertyName == foreground.id)
                return withForeground(fromVar<Fill>(value));
            if (propertyName == shadow.id)
                return withShadow(fromVar<Shadow>(value));
            if (propertyName == stroke.id)
                return withStroke(fromVar<Fill>(value));
            if (propertyName == textAlign.id)
                return withTextAlignment(fromVar<juce::Justification>(value));
            if (propertyName == thumb.id)
                return withThumb(fromVar<Fill>(value));
            if (propertyName == track.id)
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

        [[nodiscard]] Styles& withTransitions(Transitions::ReferenceCountedPointer value)
        {
            transitions = value;
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
            const StyleProperty<Fill>* property = nullptr;

            if (styleName == "accent")
                property = &accent;
            else if (styleName == "background")
                property = &background;
            else if (styleName == "border")
                property = &borderFill;
            else if (styleName == "fill")
                property = &fill;
            else if (styleName == "foreground")
                property = &foreground;
            else if (styleName == "stroke")
                property = &stroke;
            else if (styleName == "thumb")
                property = &thumb;
            else if (styleName == "track")
                property = &track;

            jassert(property != nullptr);

            if (property == nullptr || !property->hasValue())
                return std::nullopt;

            return calculateCurrent(*property);
        }

        template <>
        [[nodiscard]] std::optional<juce::BorderSize<float>> find(const juce::String& styleName) const
        {
            const StyleProperty<juce::BorderSize<float>>* property = nullptr;

            if (styleName == "border-width")
                property = &borderWidth;

            jassert(property != nullptr);

            if (property == nullptr || !property->hasValue())
                return std::nullopt;

            return calculateCurrent(*property);
        }

        template <>
        [[nodiscard]] std::optional<BorderRadii<float>> find(const juce::String& styleName) const
        {
            const StyleProperty<BorderRadii<float>>* property = nullptr;

            if (styleName == "border-radius")
                property = &borderRadius;

            jassert(property != nullptr);

            if (property == nullptr || !property->hasValue())
                return std::nullopt;

            return calculateCurrent(*property);
        }

        template <>
        [[nodiscard]] std::optional<juce::String> find(const juce::String& styleName) const
        {
            if (styleName == "font-family" && fontFamily.hasValue())
                return fontFamily.get();

            jassertfalse;
            return std::nullopt;
        }

        template <>
        [[nodiscard]] std::optional<float> find(const juce::String& styleName) const
        {
            const StyleProperty<float>* property = nullptr;

            if (styleName == "font-size")
                property = &fontPointSize;
            else if (styleName == "font-scale")
                property = &fontHorizontalScale;
            else if (styleName == "font-kerning")
                property = &fontExtraKerningFactor;
#if JUCE_MAJOR_VERSION
            else if (styleName == "font-ascent-override")
                property = &fontAscentOverride;
            else if (styleName == "font-descent-override")
                property = &fontDescentOverride;
#endif

            jassert(property != nullptr);

            if (property == nullptr || !property->hasValue())
                return std::nullopt;

            return calculateCurrent(*property);
        }

        template <>
        [[nodiscard]] std::optional<std::unordered_set<juce::Font::FontStyleFlags>> find(const juce::String& styleName) const
        {
            if (styleName == "font-styles" && fontStyleFlags.hasValue())
                return fontStyleFlags.get();

            jassertfalse;
            return std::nullopt;
        }

        template <>
        [[nodiscard]] std::optional<Shadow> find(const juce::String& styleName) const
        {
            const StyleProperty<Shadow>* property = nullptr;

            if (styleName == "shadow")
                property = &shadow;

            if (property == nullptr || !property->hasValue())
            {
                jassertfalse;
                return std::nullopt;
            }

            return calculateCurrent(*property);
        }

        template <>
        [[nodiscard]] std::optional<juce::AttributedString::ReadingDirection> find(const juce::String& styleName) const
        {
            if (styleName == "direction" && direction.hasValue())
                return direction.get();

            jassertfalse;
            return std::nullopt;
        }

        template <>
        [[nodiscard]] std::optional<juce::Justification> find(const juce::String& styleName) const
        {
            if (styleName == "text-align" && textAlign.hasValue())
                return textAlign.get();

            jassertfalse;
            return std::nullopt;
        }

        [[nodiscard]] auto operator==(const Styles& other) const
        {
            static constexpr auto tie = [](const Styles& styles) {
                return std::make_tuple(styles.accent,
                                       styles.background,
                                       styles.borderFill,
                                       styles.borderRadius,
                                       styles.borderWidth,
                                       styles.direction,
                                       styles.fill,
#if JUCE_MAJOR_VERSION >= 8
                                       styles.fontAscentOverride,
                                       styles.fontDescentOverride,
#endif
                                       styles.fontExtraKerningFactor,
                                       styles.fontFamily,
                                       styles.fontHorizontalScale,
                                       styles.fontPointSize,
                                       styles.fontStyleFlags,
                                       styles.foreground,
                                       styles.shadow,
                                       styles.stroke,
                                       styles.textAlign,
                                       styles.thumb,
                                       styles.track);
            };

            return tie(*this) == tie(other);
        }

    private:
        template <typename T>
        [[nodiscard]] std::optional<T> calculateCurrent(const StyleProperty<T>& property) const
        {
            double proportion = 1.0;

            if (transitions != nullptr)
            {
                if (auto* transition = (*transitions)[property.id.toString()])
                    proportion = transition->calculateProgress(property.getLastUpdated());
            }

            return property.interpolated(proportion);
        }
    };
} // namespace jive
