#include "jive_Label.h"

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    Label::Label(std::unique_ptr<GuiItem> itemToDecorate)
        : GuiItemDecorator{ std::move(itemToDecorate) }
        , text{ tree, "text" }
        , typefaceName{ tree, "typeface-name" }
        , fontWeight{ tree, "font-weight", "Regular" }
        , fontHeight{ tree, "font-height", 12.f }
        , fontStyle{ tree, "font-style", "plain" }
        , justification{ tree, "justification", juce::Justification::centredLeft }
    {
        text.onValueChange = [this]() {
            getLabel().setText(text, juce::sendNotification);
        };
        typefaceName.onValueChange = [this]() {
            fontChanged();
        };
        fontWeight.onValueChange = [this]() {
            fontChanged();
        };
        fontHeight.onValueChange = [this]() {
            fontChanged();
        };
        fontStyle.onValueChange = [this]() {
            fontChanged();
        };
        justification.onValueChange = [this]() {
            getLabel().setJustificationType(justification);
        };

        getLabel().setText(text, juce::sendNotification);
        getLabel().setJustificationType(justification);
        fontChanged();
    }

    //==================================================================================================================
    bool Label::isContainer() const
    {
        return false;
    }

    float Label::getWidth() const
    {
        const auto w = GuiItemDecorator::getWidth();

        if (w < 0.f)
        {
            const auto textWidth = getFont().getStringWidthFloat(text);
            const auto borderWidth = const_cast<Label*>(this)->getLabel().getBorderSize().getLeftAndRight();

            return textWidth + borderWidth;
        }

        return w;
    }

    float Label::getHeight() const
    {
        const auto h = GuiItemDecorator::getHeight();

        if (h < 0.f)
        {
            const auto textHeight = getFont().getHeight();
            const auto borderHeight = const_cast<Label*>(this)->getLabel().getBorderSize().getTopAndBottom();

            return textHeight + borderHeight;
        }

        return h;
    }

    //==================================================================================================================
    juce::Label& Label::getLabel()
    {
        auto* label = dynamic_cast<juce::Label*>(&getComponent());
        jassert(label != nullptr);

        return *label;
    }

    juce::Font Label::getFont() const
    {
        return const_cast<Label*>(this)->getLabel().getFont();
    }

    juce::String Label::getText() const
    {
        return text;
    }

    //==================================================================================================================
    int parseFontStyleFlags(const juce::String& styleString)
    {
        int flags = juce::Font::plain;
        const auto tokens = juce::StringArray::fromTokens(styleString, false);

        if (tokens.contains("bold"))
            flags += juce::Font::bold;
        if (tokens.contains("italic"))
            flags += juce::Font::italic;
        if (tokens.contains("underlined"))
            flags += juce::Font::underlined;

        return flags;
    }

    void Label::fontChanged()
    {
        juce::Font font;

        if (typefaceName.get().isNotEmpty())
            font.setTypefaceName(typefaceName);

        font.setTypefaceStyle(fontWeight);
        font.setStyleFlags(parseFontStyleFlags(fontStyle));
        font = font.withPointHeight(fontHeight);

        getLabel().setFont(font);
    }
} // namespace jive
