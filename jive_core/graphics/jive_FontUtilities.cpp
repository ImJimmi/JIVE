#include "jive_FontUtilities.h"

#include <jive_core/compile-time-helpers/jive_JuceVersion.h>
#include <jive_core/graphics/jive_Styles.h>

namespace jive
{
    float calculateStringWidth(const juce::String& text, const juce::Font& font)
    {
#if JUCE_VERSION > JIVE_JUCE_VERSION(8, 0, 1)
        return juce::GlyphArrangement::getStringWidth(font, text);
#else
        return font.getStringWidthFloat(text);
#endif
    }

    static const std::unordered_map<juce::String, juce::Font::FontStyleFlags> namedFontStyles{
        { "normal", juce::Font::plain },
        { "italic", juce::Font::italic },
        { "bold", juce::Font::bold },
        { "underlined", juce::Font::underlined },
    };

    [[nodiscard]] static auto getDefaultFontSize()
    {
        return 13.0f;
    }

    [[nodiscard]] static auto getDefaultFontStyle()
    {
        return juce::Font::plain;
    }

    [[nodiscard]] static auto getDefaultFont()
    {
        return juce::Font{
#if JUCE_MAJOR_VERSION >= 8
            juce::FontOptions{
                juce::Font::getSystemUIFontName(),
                0.0f,
                getDefaultFontStyle(),
            },
#else
            juce::Font::getDefaultSansSerifFontName(),
#endif
        }
            .withPointHeight(getDefaultFontSize());
    }

    template <typename... Args>
    [[nodiscard]] static auto makeFont(const Args&... args)
    {
        return juce::Font{
#if JUCE_MAJOR_VERSION >= 8
            juce::FontOptions{
#endif
                args...
#if JUCE_MAJOR_VERSION >= 8
            },
#endif
        };
    }

    juce::Font parseFont(const juce::String& text)
    {
        if (text.isEmpty())
            return getDefaultFont();

        static const auto getFontFamily = [](const juce::String& id) {
            static const std::unordered_map<juce::String, juce::String> genericFontNames{
                { "serif", juce::Font::getDefaultSerifFontName() },
                { "sans-serif", juce::Font::getDefaultSansSerifFontName() },
#if JUCE_MAJOR_VERSION >= 8
                { "system-ui", juce::Font::getSystemUIFontName() },
#endif
                { "monospace", juce::Font::getDefaultMonospacedFontName() },
            };

            if (const auto name = genericFontNames.find(id); name != std::end(genericFontNames))
                return name->second;

            return id;
        };

        const auto tokens = juce::StringArray::fromTokens(text, " ", "");

        if (tokens.size() == 1)
        {
            return makeFont(getFontFamily(text), 0.0f, getDefaultFontStyle())
                .withPointHeight(getDefaultFontSize());
        }

        if (tokens.size() == 2)
        {
            const auto foundFontStyle = namedFontStyles.find(tokens[0]);
            const auto fontStyle = foundFontStyle != std::end(namedFontStyles)
                                     ? foundFontStyle->second
                                     : juce::Font::plain;
            const auto fontSize = foundFontStyle != std::end(namedFontStyles)
                                    ? getDefaultFontSize()
                                    : tokens[0].getFloatValue();

            return makeFont(getFontFamily(tokens[1]), 0.0f, fontStyle)
                .withPointHeight(fontSize);
        }

        const auto numTokens = tokens.size();
        const auto fontFamily = getFontFamily(tokens[numTokens - 1]);
        const auto fontSize = tokens[numTokens - 2].getFloatValue();
        int styleFlags = juce::Font::plain;

        for (auto i = 0; i < numTokens - 2; i++)
            styleFlags += namedFontStyles.find(tokens[i])->second;

        return makeFont(fontFamily, 0.0f, styleFlags)
            .withPointHeight(fontSize);
    }

    [[nodiscard]] static auto findFont(const juce::Component& component,
                                       const Styles& styles,
                                       const juce::Font& baseFont = getDefaultFont(),
                                       const juce::String& propertyNamesPrefix = "")
    {
        std::optional<juce::String> fontFamily;
        std::optional<float> fontPointSize;
        std::unordered_set<juce::Font::FontStyleFlags> fontStyleFlags;
        std::optional<float> fontHorizontalScale;
        std::optional<float> fontExtraKerningFactor;

#if JUCE_MAJOR_VERSION >= 8
        std::optional<float> fontAscentOverride;
        std::optional<float> fontDescentOverride;
#endif
        const InteractionState state{ component };

        for (const auto* currentComponent = &component;
             currentComponent != nullptr;
             currentComponent = currentComponent->getParentComponent())
        {
            if (const auto property = state.findMostApplicable(currentComponent->getProperties(),
                                                               propertyNamesPrefix + "font-family");
                property.has_value())
            {
                fontFamily = property->toString();
            }
            if (const auto property = state.findMostApplicable(currentComponent->getProperties(),
                                                               propertyNamesPrefix + "font-size");
                property.has_value())
            {
                fontPointSize = static_cast<float>(*property);
            }
            if (const auto property = state.findMostApplicable(currentComponent->getProperties(),
                                                               propertyNamesPrefix + "font-styles");
                property.has_value())
            {
                for (const auto& token : juce::StringArray::fromTokens(property->toString(), " ", ""))
                {
                    if (const auto namedStyle = namedFontStyles.find(token);
                        namedStyle != std::end(namedFontStyles))
                    {
                        fontStyleFlags.emplace(namedStyle->second);
                    }
                }
            }
            if (const auto property = state.findMostApplicable(currentComponent->getProperties(),
                                                               propertyNamesPrefix + "font-scale");
                property.has_value())
            {
                fontHorizontalScale = static_cast<float>(*property);
            }
            if (const auto property = state.findMostApplicable(currentComponent->getProperties(),
                                                               propertyNamesPrefix + "font-kerning");
                property.has_value())
            {
                fontExtraKerningFactor = static_cast<float>(*property);
            }
#if JUCE_MAJOR_VERSION >= 8
            if (const auto property = state.findMostApplicable(currentComponent->getProperties(),
                                                               propertyNamesPrefix + "font-ascent-override");
                property.has_value())
            {
                fontAscentOverride = static_cast<float>(*property);
            }
            if (const auto property = state.findMostApplicable(currentComponent->getProperties(),
                                                               propertyNamesPrefix + "font-descent-override");
                property.has_value())
            {
                fontDescentOverride = static_cast<float>(*property);
            }
#endif
        }

        auto result = baseFont;

        result.setTypefaceName(fontFamily.value_or(styles.fontFamily.value_or(result.getTypefaceName())));
        result = result.withPointHeight(fontPointSize.value_or(styles.fontPointSize.value_or(result.getHeightInPoints())));
        result.setHorizontalScale(fontHorizontalScale.value_or(styles.fontHorizontalScale.value_or(result.getHorizontalScale())));
        result.setExtraKerningFactor(fontExtraKerningFactor.value_or(styles.fontExtraKerningFactor.value_or(result.getExtraKerningFactor())));

        for (const auto& flag : styles.fontStyleFlags)
            fontStyleFlags.emplace(flag);
        for (const auto& flag : fontStyleFlags)
            result.setStyleFlags(result.getStyleFlags() + flag);

#if JUCE_MAJOR_VERSION >= 8
        if (const auto ascent = fontAscentOverride; ascent.has_value())
            result.setAscentOverride(ascent);
        else if (const auto a = styles.fontAscentOverride; ascent.has_value())
            result.setAscentOverride(a);

        if (const auto descent = fontDescentOverride; descent.has_value())
            result.setDescentOverride(descent);
        else if (const auto d = styles.fontDescentOverride; descent.has_value())
            result.setDescentOverride(d);
#endif

        return result;
    }

    juce::Font getFont(const juce::Component& component,
                       const Styles& styles,
                       juce::Font defaultFont)
    {
        return findFont(component, styles, defaultFont);
    }

    juce::Font getFont(const juce::Label& label,
                       const Styles& styles)
    {
        return findFont(label,
                        styles,
                        label
                            .getLookAndFeel()
                            .getLabelFont(*const_cast<juce::Label*>(&label)));
    }

    juce::Font getFont(const juce::PopupMenu::Item&,
                       const juce::Component& component,
                       const Styles& styles)
    {
        auto baseFont = component.getLookAndFeel().getPopupMenuFont();

        if (const auto* comboBox = dynamic_cast<const juce::ComboBox*>(&component))
            baseFont = findFont(*comboBox, styles, baseFont, "popup.");

        return findFont(component, styles, baseFont, "popup.");
    }

    juce::Font getFont(const juce::ComboBox& comboBox,
                       const Styles& styles)
    {
        return findFont(comboBox,
                        styles,
                        comboBox
                            .getLookAndFeel()
                            .getComboBoxFont(*const_cast<juce::ComboBox*>(&comboBox)));
    }
} // namespace jive
