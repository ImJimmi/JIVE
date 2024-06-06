#include "jive_StyleCache.h"

#include "jive_StyleSheet.h"

namespace jive
{
    [[nodiscard]] static auto makeCachedPropertyEntry(const juce::String& id, std::function<void()> update)
    {
        StyleCache::CachedProperty property;
        property.update = update;

        return std::make_pair(id, property);
    }

    StyleCache::StyleCache(const StyleSheet& sheet)
        : styleSheet{ sheet }
        , properties{
            makeCachedPropertyEntry("font-family", [this] {
                properties["font-family"].value = styleSheet.findHierarchicalStyleProperty("font-family");
            }),
            makeCachedPropertyEntry("font-style", [this] {
                properties["font-style"].value = styleSheet.findHierarchicalStyleProperty("font-style");
            }),
            makeCachedPropertyEntry("font-weight", [this] {
                properties["font-weight"].value = styleSheet.findHierarchicalStyleProperty("font-weight");
            }),
            makeCachedPropertyEntry("font-size", [this] {
                properties["font-size"].value = styleSheet.findHierarchicalStyleProperty("font-size");
            }),
            makeCachedPropertyEntry("letter-spacing", [this] {
                properties["letter-spacing"].value = styleSheet.findHierarchicalStyleProperty("letter-spacing");
            }),
            makeCachedPropertyEntry("text-decoration", [this] {
                properties["text-decoration"].value = styleSheet.findHierarchicalStyleProperty("text-decoration");
            }),
            makeCachedPropertyEntry("font-stretch", [this] {
                properties["font-stretch"].value = styleSheet.findHierarchicalStyleProperty("font-stretch");
            }),
            makeCachedPropertyEntry("border", [this] {
                properties["border"].value = styleSheet.findStyleProperty("border");
            }),
            makeCachedPropertyEntry("background", [this] {
                properties["background"].value = styleSheet.findStyleProperty("background");
            }),
            makeCachedPropertyEntry("foreground", [this] {
                properties["foreground"].value = styleSheet.findHierarchicalStyleProperty("foreground");
            }),
            makeCachedPropertyEntry("border-radius", [this] {
                properties["border-radius"].value = styleSheet.findStyleProperty("border-radius");
            }),
        }
    {
    }

    juce::Font StyleCache::getFont() const
    {
        juce::Font font;

        static constexpr auto isNumeric = [](const juce::var& v) {
            return v.isBool() || v.isDouble() || v.isInt64() || v.isInt();
        };

        if (const auto fontFamily = properties.at("font-family").value;
            fontFamily.isString())
        {
            font.setTypefaceName(fontFamily.toString());
        }
        if (const auto fontStyle = properties.at("font-style").value;
            fontStyle.isString())
        {
            font.setItalic(fontStyle.toString().compareIgnoreCase("italic") == 0);
        }
        if (const auto fontWeight = properties.at("font-weight").value;
            fontWeight.isString())
        {
            font.setBold(fontWeight.toString().compareIgnoreCase("bold") == 0);
        }
        if (const auto textDecoration = properties.at("text-decoration").value;
            textDecoration.isString())
        {
            font.setUnderline(textDecoration.toString().compareIgnoreCase("underlined") == 0);
        }
        if (const auto fontStretch = properties.at("font-stretch").value;
            isNumeric(fontStretch))
        {
            font.setHorizontalScale(static_cast<float>(fontStretch));
        }
        if (const auto fontSize = properties.at("font-size").value;
            isNumeric(fontSize))
        {
            font = font.withPointHeight(static_cast<float>(fontSize));
        }
        if (const auto letterSpacing = properties.at("letter-spacing").value;
            isNumeric(letterSpacing))
        {
            const auto extraKerning = static_cast<float>(letterSpacing) / font.getHeight();
            font.setExtraKerningFactor(extraKerning);
        }

        return font;
    }

    Fill StyleCache::getBorderFill() const
    {
        return fromVar<Fill>(properties.at("border").value);
    }

    Fill StyleCache::getBackground() const
    {
        return fromVar<Fill>(properties.at("background").value);
    }

    Fill StyleCache::getForeground() const
    {
        return fromVar<Fill>(properties.at("foreground").value);
    }

    BorderRadii<float> StyleCache::getBorderRadii() const
    {
        return fromVar<BorderRadii<float>>(properties.at("border-radius").value);
    }

    void StyleCache::update()
    {
        for (const auto& pair : properties)
            pair.second.update();
    }

    void StyleCache::update(const juce::Identifier& id)
    {
        if (auto property = properties.find(id.toString());
            property != std::end(properties))
        {
            property->second.update();
        }
    }
} // namespace jive
