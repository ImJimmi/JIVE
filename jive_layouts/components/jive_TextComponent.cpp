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
} // namespace jive
