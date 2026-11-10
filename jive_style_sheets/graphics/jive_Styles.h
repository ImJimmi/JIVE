#pragma once

#include "jive_Fill.h"
#include "jive_Icon.h"
#include "jive_Shadow.h"

#include <jive_core/algorithms/jive_Interpolate.h>
#include <jive_core/errors/jive_ErrorHandling.h>
#include <jive_core/geometry/jive_BorderRadii.h>
#include <jive_core/kinetics/jive_Transitions.h>
#include <jive_core/time/jive_Timer.h>
#include <jive_core/values/jive_Identifiers.h>
#include <jive_core/values/jive_Number.h>
#include <jive_core/values/variant-converters/jive_VariantConvertion.h>

#include <juce_graphics/juce_graphics.h>

namespace jive
{
    // A set of styles that might be applied to a component.
    // Each of the styles defaults to being undefined so you can bevery explicit
    // about what you want to define. When receiving a Styles object, you should
    // decide on a sensible default if a style is undefined, or use some other
    // sort of error handling in that case.
    using FontAscentOverride = Number<float, struct FontAscentOverrideTag>;
    using FontDescentOverride = Number<float, struct FontDescentOverrideTag>;
    using FontKerning = Number<float, struct FontKerningTag>;
    using FontScale = Number<float, struct FontScaleTag>;
    using FontSize = Number<float, struct FontSizeTag>;
    using LineSpacing = Number<float, struct LineSpacingTag>;
    using Opacity = Number<float, struct OpacityTag>;

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

            // Sets a new target value while explicitly specifying the value to
            // transition away from. This is used to restart an in-progress
            // transition from its current (interpolated) value so that
            // interrupting one transition with another continues smoothly
            // rather than snapping back to the previous target.
            void setTransitioningFrom(const T& from, const T& newValue)
            {
                previousValue = from;
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

        StyleProperty<Fill> accent{ ids::accent() };
        StyleProperty<Fill> background{ ids::background() };
        StyleProperty<Fill> borderFill{ ids::border() };
        StyleProperty<BorderRadii<float>> borderRadius{ ids::borderRadius() };
        StyleProperty<juce::BorderSize<float>> borderWidth{ ids::borderWidth() };
        StyleProperty<juce::AttributedString::ReadingDirection> direction{ ids::direction() };
        StyleProperty<Fill> fill{ ids::fill() };
#if JUCE_MAJOR_VERSION >= 8
        StyleProperty<FontAscentOverride> fontAscentOverride{ ids::fontAscentOverride() };
        StyleProperty<FontDescentOverride> fontDescentOverride{ ids::fontDescentOverride() };
#endif
        StyleProperty<FontKerning> fontExtraKerningFactor{ ids::fontKerning() };
        StyleProperty<juce::String> fontFamily{ ids::fontFamily() };
        StyleProperty<FontScale> fontHorizontalScale{ ids::fontScale() };
        StyleProperty<FontSize> fontPointSize{ ids::fontSize() };
        StyleProperty<std::unordered_set<juce::Font::FontStyleFlags>> fontStyleFlags{ ids::fontStyles() };
        StyleProperty<Fill> foreground{ ids::foreground() };
        StyleProperty<Icon> icon{ ids::icon() };
        StyleProperty<LineSpacing> lineSpacing{ ids::lineSpacing() };
        StyleProperty<Opacity> opacity{ ids::opacity() };
        StyleProperty<Shadow> shadow{ ids::shadow() };
        StyleProperty<Fill> stroke{ ids::stroke() };
        StyleProperty<juce::Justification> textAlign{ ids::textAlign() };
        StyleProperty<Fill> thumb{ ids::thumb() };
        StyleProperty<Fill> track{ ids::track() };

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
            "icon",
            "line-spacing",
            "opacity",
            "shadow",
            "stroke",
            "text-align",
            "thumb",
            "track",
        };

        [[nodiscard]] Styles& with(const juce::Identifier& propertyName,
                                   const juce::var& value,
                                   const Object::ReferenceCountedPointer& source = nullptr,
                                   const SourceFile& sourceFile = {})
        {
            if (propertyName == accent.id)
                return withValid<Fill>(&Styles::withAccent, propertyName, value, source, sourceFile);
            if (propertyName == background.id)
                return withValid<Fill>(&Styles::withBackground, propertyName, value, source, sourceFile);
            if (propertyName == borderFill.id)
                return withValid<Fill>(&Styles::withBorderFill, propertyName, value, source, sourceFile);
            if (propertyName == borderRadius.id)
                return withValid<BorderRadii<float>>(&Styles::withBorderRadius, propertyName, value, source, sourceFile);
            if (propertyName == borderWidth.id)
                return withValid<juce::BorderSize<float>>(&Styles::withBorderWidth, propertyName, value, source, sourceFile);
            if (propertyName == direction.id)
                return withValid<juce::AttributedString::ReadingDirection>(&Styles::withReadingDirection, propertyName, value, source, sourceFile);
            if (propertyName == fill.id)
                return withValid<Fill>(&Styles::withFill, propertyName, value, source, sourceFile);
#if JUCE_MAJOR_VERSION >= 8
            if (propertyName == fontAscentOverride.id)
                return withValid<FontAscentOverride>(&Styles::withFontAscentOverride, propertyName, value, source, sourceFile);
            if (propertyName == fontDescentOverride.id)
                return withValid<FontDescentOverride>(&Styles::withFontDescentOverride, propertyName, value, source, sourceFile);
#endif
            if (propertyName == fontExtraKerningFactor.id)
                return withValid<FontKerning>(&Styles::withFontExtraKerningFactor, propertyName, value, source, sourceFile);
            if (propertyName == fontFamily.id)
                return withFontFamily(value.toString());
            if (propertyName == fontHorizontalScale.id)
                return withValid<FontScale>(&Styles::withFontHorizontalScale, propertyName, value, source, sourceFile);
            if (propertyName == fontPointSize.id)
                return withValid<FontSize>(&Styles::withFontPointSize, propertyName, value, source, sourceFile);
            if (propertyName == fontStyleFlags.id)
                return withValid<std::unordered_set<juce::Font::FontStyleFlags>>(&Styles::withFontStyleFlags, propertyName, value, source, sourceFile);
            if (propertyName == foreground.id)
                return withValid<Fill>(&Styles::withForeground, propertyName, value, source, sourceFile);
            if (propertyName == icon.id)
                return withValid<Icon>(&Styles::withIcon, propertyName, value, source, sourceFile);
            if (propertyName == lineSpacing.id)
                return withValid<LineSpacing>(&Styles::withLineSpacing, propertyName, value, source, sourceFile);
            if (propertyName == opacity.id)
                return withValid<Opacity>(&Styles::withOpacity, propertyName, value, source, sourceFile);
            if (propertyName == shadow.id)
                return withValid<Shadow>(&Styles::withShadow, propertyName, value, source, sourceFile);
            if (propertyName == stroke.id)
                return withValid<Fill>(&Styles::withStroke, propertyName, value, source, sourceFile);
            if (propertyName == textAlign.id)
                return withValid<juce::Justification>(&Styles::withTextAlignment, propertyName, value, source, sourceFile);
            if (propertyName == thumb.id)
                return withValid<Fill>(&Styles::withThumb, propertyName, value, source, sourceFile);
            if (propertyName == track.id)
                return withValid<Fill>(&Styles::withTrack, propertyName, value, source, sourceFile);

            jassertfalse;
            return *this;
        }

        template <typename T, typename Setter>
        [[nodiscard]] Styles& withValid(Setter setter,
                                        const juce::Identifier& propertyName,
                                        const juce::var& value,
                                        const Object::ReferenceCountedPointer& source,
                                        const SourceFile& sourceFile)
        {
            if (isValidVar<T>(value))
                return (this->*setter)(fromVar<T>(value));

            if constexpr (hasVariantSyntax<T>())
                logValueError(source, propertyName, "Expected " + juce::VariantConverter<T>::getSyntax(), sourceFile);
            else
                logValueError(source, propertyName, "", sourceFile);

            if constexpr (hasVariantDefault<T>())
                return (this->*setter)(juce::VariantConverter<T>::defaultValue());
            else
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
        [[nodiscard]] Styles& withFontAscentOverride(FontAscentOverride value)
        {
            fontAscentOverride = value;
            return *this;
        }

        [[nodiscard]] Styles& withFontDescentOverride(FontDescentOverride value)
        {
            fontDescentOverride = value;
            return *this;
        }
#endif

        [[nodiscard]] Styles& withFontExtraKerningFactor(FontKerning value)
        {
            fontExtraKerningFactor = value;
            return *this;
        }

        [[nodiscard]] Styles& withFontFamily(const juce::String& value)
        {
            fontFamily = value;
            return *this;
        }

        [[nodiscard]] Styles& withFontHorizontalScale(FontScale value)
        {
            fontHorizontalScale = value;
            return *this;
        }

        [[nodiscard]] Styles& withFontPointSize(FontSize value)
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

        [[nodiscard]] Styles& withIcon(const Icon& value)
        {
            icon = value;
            return *this;
        }

        [[nodiscard]] Styles& withLineSpacing(LineSpacing value)
        {
            lineSpacing = value;
            return *this;
        }

        [[nodiscard]] Styles& withOpacity(Opacity value)
        {
            opacity = value;
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
            const auto current = [this](const auto& property) -> std::optional<float> {
                if (!property.hasValue())
                    return std::nullopt;

                return calculateCurrent(property);
            };

            if (styleName == "font-size")
                return current(fontPointSize);
            if (styleName == "font-scale")
                return current(fontHorizontalScale);
            if (styleName == "font-kerning")
                return current(fontExtraKerningFactor);
            if (styleName == "line-spacing")
                return current(lineSpacing);
            if (styleName == "opacity")
                return current(opacity);
#if JUCE_MAJOR_VERSION >= 8
            if (styleName == "font-ascent-override")
                return current(fontAscentOverride);
            if (styleName == "font-descent-override")
                return current(fontDescentOverride);
#endif

            jassertfalse;
            return std::nullopt;
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
                                       styles.icon,
                                       styles.lineSpacing,
                                       styles.opacity,
                                       styles.shadow,
                                       styles.stroke,
                                       styles.textAlign,
                                       styles.thumb,
                                       styles.track);
            };

            return tie(*this) == tie(other);
        }

        [[nodiscard]] auto hasTransitionsInProgress() const
        {
            if (transitions == nullptr)
                return false;

            bool anyTransitioning = false;

            forEach([this, &anyTransitioning](auto property) {
                if (auto* transition = (*transitions)[property.id.toString()])
                {
                    const auto proportion = transition->calculateProgress(property.getLastUpdated());

                    if (!juce::approximatelyEqual(proportion, 1.0))
                        anyTransitioning = true;
                }
            });

            return anyTransitioning;
        }

    private:
        template <typename UnaryFunction>
        void forEach(UnaryFunction function)
        {
            function(accent);
            function(background);
            function(borderFill);
            function(borderRadius);
            function(borderWidth);
            function(direction);
            function(fill);
#if JUCE_MAJOR_VERSION >= 8
            function(fontAscentOverride);
            function(fontDescentOverride);
#endif
            function(fontExtraKerningFactor);
            function(fontFamily);
            function(fontHorizontalScale);
            function(fontPointSize);
            function(fontStyleFlags);
            function(foreground);
            function(icon);
            function(lineSpacing);
            function(opacity);
            function(shadow);
            function(stroke);
            function(textAlign);
            function(thumb);
            function(track);
        }

        template <typename UnaryFunction>
        void forEach(UnaryFunction function) const
        {
            const_cast<Styles*>(this)->forEach(std::move(function));
        }

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

namespace juce
{
    template <>
    struct VariantConverter<jive::Opacity>
    {
        static jive::Opacity fromVar(const var& v)
        {
            if (v.isString() && v.toString().trim().endsWithChar('%'))
                return v.toString().getFloatValue() / 100.0f;

            return static_cast<float>(v);
        }

        static var toVar(jive::Opacity opacity)
        {
            return opacity.value;
        }

        static bool isValid(const var& v)
        {
            return v.isVoid() || jive::isNumber(v, { "%" });
        }

        static String getSyntax()
        {
            return "a number or percentage";
        }
    };
} // namespace juce
