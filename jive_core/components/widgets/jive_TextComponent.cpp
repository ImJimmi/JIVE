#include "jive_TextComponent.h"

#include <jive_core/graphics/jive_FontUtilities.h>
#include <jive_core/graphics/jive_LookAndFeel.h>

namespace jive
{
    TextComponent::TextComponent()
    {
        setInterceptsMouseClicks(false, false);
    }

    void TextComponent::paint(juce::Graphics& g)
    {
        updateCachedFont();

        if (dynamic_cast<TextComponent*>(getParentComponent()) != nullptr)
            return;

        getAttributedString()
            .draw(g, getLocalBounds().toFloat());
    }

    void TextComponent::lookAndFeelChanged()
    {
        updateCachedFont();
    }

    void TextComponent::parentHierarchyChanged()
    {
        updateCachedFont();
    }

    const juce::String& TextComponent::getText() const
    {
        return text;
    }

    void TextComponent::setText(const juce::String& newText)
    {
        if (newText != text)
        {
            text = newText;
            repaint();
        }
    }

    void TextComponent::setLineSpacing(float newLineSpacing)
    {
        if (!juce::approximatelyEqual(newLineSpacing, lineSpacing))
        {
            lineSpacing = newLineSpacing;
            repaint();
        }
        updateCachedFont();
    }

    void TextComponent::setWordWrap(juce::AttributedString::WordWrap newWordWrap)
    {
        if (newWordWrap != wordWrap)
        {
            wordWrap = newWordWrap;
            repaint();
        }
    }

    void TextComponent::clearAttributes()
    {
        appendices.clear();
        repaint();
    }

    void TextComponent::append(const juce::AttributedString& attributedStringToAppend)
    {
        appendices.add(attributedStringToAppend);
        repaint();
    }

    juce::AttributedString TextComponent::getAttributedString() const
    {
        juce::AttributedString attributedString;

        attributedString.setText(text);
        attributedString.setLineSpacing(lineSpacing);
        attributedString.setWordWrap(wordWrap);

        for (const auto& appendix : appendices)
            attributedString.append(appendix);

        if (auto* lookAndFeel = dynamic_cast<LookAndFeel*>(&getLookAndFeel()))
        {
            const auto styles = lookAndFeel->findMostApplicableStyles(*this);

            attributedString.setReadingDirection(getReadingDirection(*this, styles));
            attributedString.setJustification(getTextAlignment(*this, styles));

            if (cachedFont.has_value())
                attributedString.setFont(*cachedFont);

            const auto foreground = getForegroundFill(*this, styles);

            if (foreground.isColour())
                attributedString.setColour(foreground.colour);
            else if (foreground.isGradient())
                attributedString.setColour(foreground.gradient->getColour(0));
        }

        return attributedString;
    }

    void TextComponent::addListener(Listener& listener) const
    {
        listeners.add(&listener);
    }

    void TextComponent::removeListener(Listener& listener) const
    {
        listeners.remove(&listener);
    }

    class TextComponent::AccessibilityInterface : public juce::AccessibilityTextValueInterface
    {
    public:
        explicit AccessibilityInterface(TextComponent& componentToRepresent)
            : textComponent{ componentToRepresent }
        {
        }

        bool isReadOnly() const override
        {
            return true;
        }

        juce::String getCurrentValueAsString() const override
        {
            return textComponent.getText();
        }

        void setValueAsString(const juce::String& newValue) override
        {
            textComponent.setText(newValue);
        }

    private:
        TextComponent& textComponent;
    };

    class TextComponent::AccessibilityHandler : public juce::AccessibilityHandler
    {
    public:
        explicit AccessibilityHandler(TextComponent& componentToRepresent)
            : juce::AccessibilityHandler{
                componentToRepresent,
                juce::AccessibilityRole::staticText,
                juce::AccessibilityActions{},
                {
                    std::make_unique<AccessibilityInterface>(componentToRepresent),
                },
            }
            , textComponent{ componentToRepresent }
        {
        }

        juce::String getTitle() const override
        {
            if (textComponent.getTitle().isNotEmpty())
                return textComponent.getTitle();

            return textComponent.getText();
        }

        juce::String getDescription() const override
        {
            if (textComponent.getDescription().isNotEmpty())
                return textComponent.getDescription();

            if (textComponent.getTitle().isNotEmpty())
                return textComponent.getText();

            return "";
        }

    private:
        TextComponent& textComponent;
    };

    std::unique_ptr<juce::AccessibilityHandler> TextComponent::createAccessibilityHandler()
    {
        return std::make_unique<AccessibilityHandler>(*this);
    }

    void TextComponent::updateCachedFont()
    {
        if (auto* lookAndFeel = dynamic_cast<LookAndFeel*>(&getLookAndFeel()))
        {
            if (auto newFont = getFont(*this, lookAndFeel->findMostApplicableStyles(*this));
                newFont != cachedFont)
            {
                cachedFont = newFont;
                listeners.call(&Listener::textFontChanged, *this);
            }
        }
    }
} // namespace jive
