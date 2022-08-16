#include <jive_layouts/jive_layouts.h>

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    TextComponent::TextComponent()
    {
        setInterceptsMouseClicks(false, false);
    }

    //==================================================================================================================
    void TextComponent::paint(juce::Graphics& g)
    {
        attributedString.draw(g, getLocalBounds().toFloat());
    }

    //==================================================================================================================
    void TextComponent::setText(const juce::String& text)
    {
        attributedString.setText(text);
        repaint();
    }

    const juce::String& TextComponent::getText() const
    {
        return attributedString.getText();
    }

    void TextComponent::setFont(const juce::Font& font)
    {
        attributedString.setFont(font);
        repaint();
    }

    void TextComponent::setJustification(juce::Justification justification)
    {
        attributedString.setJustification(justification);
        repaint();
    }

    void TextComponent::setWordWrap(juce::AttributedString::WordWrap wrap)
    {
        attributedString.setWordWrap(wrap);
        repaint();
    }

    void TextComponent::setDirection(juce::AttributedString::ReadingDirection direction)
    {
        attributedString.setReadingDirection(direction);
        repaint();
    }

    void TextComponent::setLineSpacing(float spacing)
    {
        attributedString.setLineSpacing(spacing);
        repaint();
    }

    void TextComponent::clearAttributes()
    {
        attributedString.clear();
    }

    void TextComponent::append(const juce::AttributedString& attributedStringToAppend)
    {
        attributedString.append(attributedStringToAppend);
    }

    const juce::AttributedString& TextComponent::getAttributedString() const
    {
        return attributedString;
    }

    //==================================================================================================================
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
