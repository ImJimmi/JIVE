#include <jive_layouts/jive_layouts.h>

namespace jive
{
    TextComponent::TextComponent()
    {
        canvas.onPaint = [this](juce::Graphics& g) {
            if (dynamic_cast<TextComponent*>(getParentComponent()) != nullptr)
                return;

            getAttributedString()
                .draw(g, getLocalBounds().toFloat());
        };
        canvas.setAlwaysOnTop(true);
        canvas.setBufferedToImage(true);
        addAndMakeVisible(canvas);

        setInterceptsMouseClicks(false, false);
    }

    void TextComponent::resized()
    {
        canvas.setBounds(getLocalBounds());
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
            canvas.repaint();
        }
    }

    juce::Font TextComponent::getFont() const
    {
        return font;
    }

    void TextComponent::setFont(const juce::Font& newFont)
    {
        if (newFont != font)
        {
            font = newFont;
            listeners.call(&Listener::textFontChanged, *this);
            canvas.repaint();
        }
    }

    void TextComponent::setJustification(juce::Justification newJustification)
    {
        if (newJustification != justification)
        {
            justification = newJustification;
            canvas.repaint();
        }
    }

    void TextComponent::setWordWrap(juce::AttributedString::WordWrap newWordWrap)
    {
        if (newWordWrap != wordWrap)
        {
            wordWrap = newWordWrap;
            canvas.repaint();
        }
    }

    void TextComponent::setDirection(juce::AttributedString::ReadingDirection newDirection)
    {
        if (newDirection != direction)
        {
            direction = newDirection;
            canvas.repaint();
        }
    }

    void TextComponent::setLineSpacing(float newLineSpacing)
    {
        if (newLineSpacing != lineSpacing)
        {
            lineSpacing = newLineSpacing;
            canvas.repaint();
        }
    }

    void TextComponent::setTextColour(juce::Colour newColour)
    {
        if (newColour != textColour)
        {
            textColour = newColour;
            canvas.repaint();
        }
    }

    void TextComponent::clearAttributes()
    {
        appendices.clear();
        canvas.repaint();
    }

    void TextComponent::append(const juce::AttributedString& attributedStringToAppend)
    {
        appendices.add(attributedStringToAppend);
        canvas.repaint();
    }

    juce::AttributedString TextComponent::getAttributedString() const
    {
        juce::AttributedString attributedString;

        attributedString.setText(text);

        attributedString.setFont(font);
        attributedString.setJustification(justification);
        attributedString.setLineSpacing(lineSpacing);
        attributedString.setReadingDirection(direction);
        attributedString.setWordWrap(wordWrap);

        for (const auto& appendix : appendices)
            attributedString.append(appendix);

        if (textColour.has_value())
            attributedString.setColour(*textColour);
        else if (appendices.size() == 0)
            attributedString.setColour(juce::Colours::black);

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

    std::unique_ptr<juce::AccessibilityHandler>
    TextComponent::createAccessibilityHandler()
    {
        return std::make_unique<AccessibilityHandler>(*this);
    }
} // namespace jive
