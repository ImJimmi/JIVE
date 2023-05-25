#pragma once

namespace jive
{
    class Label : public GuiItemDecorator
    {
    public:
        explicit Label(std::unique_ptr<GuiItem> itemToDecorate);

        bool isContainer() const override;

        juce::Label& getLabel();
        const juce::Label& getLabel() const;

    private:
        Property<juce::BorderSize<float>> border;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Label)
    };
} // namespace jive
