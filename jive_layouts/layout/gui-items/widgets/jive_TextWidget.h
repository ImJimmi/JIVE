#pragma once

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    class TextWidget
    {
    public:
        //==============================================================================================================
        explicit TextWidget(juce::ValueTree tree);

        //==============================================================================================================
        juce::String getText() const;
        juce::Font getFont() const;
        juce::Justification getTextJustification() const;

    protected:
        //==============================================================================================================
        std::function<void(void)> onTextChanged = nullptr;
        std::function<void(void)> onFontChanged = nullptr;
        std::function<void(void)> onJustificationChanged = nullptr;

    private:
        //==============================================================================================================
        void updateFont();

        //==============================================================================================================
        juce::ValueTree widgetTree;

        TypedValue<juce::String> text;
        TypedValue<juce::String> typefaceName;
        TypedValue<juce::String> fontWeight;
        TypedValue<float> fontHeight;
        TypedValue<juce::String> fontStyle;
        TypedValue<juce::Justification> justification;

        juce::Font font;

        //==============================================================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TextWidget)
    };
} // namespace jive
