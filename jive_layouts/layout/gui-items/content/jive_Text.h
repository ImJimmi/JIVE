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

    private:
        //==============================================================================================================
        juce::TextLayout buildTextLayout(float maxWidth = -1.0f) const;

        void updateFont();
        void updateTextComponent();

        //==============================================================================================================
        Property<juce::String> text;
        Property<juce::String, HereditaryValueBehaviour::inheritFromAncestors> typefaceName;
        Property<juce::String, HereditaryValueBehaviour::inheritFromAncestors> fontWeight;
        Property<float, HereditaryValueBehaviour::inheritFromAncestors> fontHeight;
        Property<juce::String, HereditaryValueBehaviour::inheritFromAncestors> fontStyle;
        Property<float, HereditaryValueBehaviour::inheritFromAncestors> kerning;
        Property<float, HereditaryValueBehaviour::inheritFromAncestors> horizontalScale;
        Property<float, HereditaryValueBehaviour::inheritFromAncestors> lineSpacing;
        Property<juce::Justification> justification;
        Property<juce::AttributedString::WordWrap> wordWrap;
        Property<juce::AttributedString::ReadingDirection> direction;
        Property<float> idealWidth;
        Property<float> idealHeight;

        //==============================================================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Text)
    };

    //==================================================================================================================
    const Text* findFirstTextContent(const GuiItem& item);
} // namespace jive
