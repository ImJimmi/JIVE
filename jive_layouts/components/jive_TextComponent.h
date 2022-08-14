#pragma once

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    class TextComponent : public juce::Component
    {
    public:
        //==============================================================================================================
        TextComponent();

        //==============================================================================================================
        void paint(juce::Graphics& g) override;

        //==============================================================================================================
        void setText(const juce::String& text);
        void setFont(const juce::Font& font);
        void setJustification(juce::Justification justification);
        void setWordWrap(juce::AttributedString::WordWrap wrap);
        void setDirection(juce::AttributedString::ReadingDirection direction);
        void setLineSpacing(float spacing);

        void clearAttributes();
        void append(const juce::AttributedString& attributedStringToAppend);

        const juce::AttributedString& getAttributedString() const;

    private:
        //==============================================================================================================
        juce::AttributedString attributedString;

        //==============================================================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TextComponent)
    };
} // namespace jive
