#pragma once

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    class Label : public GuiItemDecorator
    {
    public:
        //==============================================================================================================
        explicit Label(std::unique_ptr<GuiItem> itemToDecorate);

        //==============================================================================================================
        bool isContainer() const override;

        float getWidth() const override;
        float getHeight() const override;

        //==============================================================================================================
        juce::Label& getLabel();
        const juce::Label& getLabel() const;

        juce::Font getFont() const;
        juce::String getText() const;

    private:
        //==============================================================================================================
        void fontChanged();

        //==============================================================================================================
        const std::unique_ptr<GuiItem> item;

        TypedValue<juce::String> text;
        TypedValue<juce::String> typefaceName;
        TypedValue<juce::String> fontWeight;
        TypedValue<float> fontHeight;
        TypedValue<juce::String> fontStyle;
        TypedValue<juce::Justification> justification;
        TypedValue<juce::BorderSize<float>> border;

        //==============================================================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Label)
    };
} // namespace jive
