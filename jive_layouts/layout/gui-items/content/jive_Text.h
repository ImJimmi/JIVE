#pragma once

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    class Text : public GuiItemDecorator
    {
    public:
        //==============================================================================================================
        explicit Text(std::unique_ptr<GuiItem> itemToDecorate);

        //==============================================================================================================
        void addChild(std::unique_ptr<GuiItem> child) override;

        bool isContainer() const override;
        bool isContent() const override;

        //==============================================================================================================
        TextComponent& getTextComponent();
        const TextComponent& getTextComponent() const;

    protected:
        //==============================================================================================================
        void valueTreeChildAdded(juce::ValueTree& parent, juce::ValueTree& child) override;
        void componentParentHierarchyChanged(juce::Component& componentWhoseParentHierarchyChanged) override;

    private:
        //==============================================================================================================
        juce::TextLayout buildTextLayout() const;

        void updateFont();
        void updateTextComponent();
        void updateExplicitSize();

        //==============================================================================================================
        TypedValue<juce::String> text;
        TypedValue<juce::String> typefaceName;
        TypedValue<juce::String> fontWeight;
        TypedValue<float> fontHeight;
        TypedValue<juce::String> fontStyle;
        TypedValue<float> kerning;
        TypedValue<float> horizontalScale;
        TypedValue<float> lineSpacing;
        TypedValue<juce::Justification> justification;
        TypedValue<juce::AttributedString::WordWrap> wordWrap;
        TypedValue<juce::AttributedString::ReadingDirection> direction;
        TypedValue<float> explicitWidth;
        TypedValue<float> explicitHeight;

        //==============================================================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Text)
    };

    //==================================================================================================================
    const Text* findFirstTextContent(const GuiItem& item);
} // namespace jive
