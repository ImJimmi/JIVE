#pragma once

#include <jive_components/jive_components.h>

#include <jive_layouts/layout/gui-items/jive_GuiItemDecorator.h>

namespace jive
{
    class Text
        : public GuiItemDecorator
        , private TextComponent::Listener
    {
    public:
        explicit Text(std::unique_ptr<GuiItem> itemToDecorate);
        ~Text() override;

        void insertChild(std::unique_ptr<GuiItem> child, int index) override;
        void setChildren(std::vector<std::unique_ptr<GuiItem>>&& newChildren) override;

        bool isContainer() const override;
        bool isContent() const override;

        TextComponent& getTextComponent();
        const TextComponent& getTextComponent() const;

    private:
        void textFontChanged(TextComponent& text) final;

        juce::TextLayout buildTextLayout(float maxWidth = -1.0f) const;

        void updateTextComponent();

        Property<juce::String> text;
        Property<float, Inheritance::inheritFromAncestors> lineSpacing;
        Property<juce::Justification> justification;
        Property<juce::AttributedString::WordWrap> wordWrap;
        Property<juce::AttributedString::ReadingDirection> direction;
        Property<float> idealWidth;
        Property<float> idealHeight;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Text)
    };

    const Text* findFirstTextContent(const GuiItem& item);
} // namespace jive
