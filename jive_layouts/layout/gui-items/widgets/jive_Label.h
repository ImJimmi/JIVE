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
    };
} // namespace jive
