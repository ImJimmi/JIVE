#pragma once

#include <jive_layouts/layout/gui-items/jive_GuiItemDecorator.h>

namespace jive
{
    class Label
        : public GuiItemDecorator
        , private juce::ValueTree::Listener
    {
    public:
        explicit Label(std::unique_ptr<GuiItem> itemToDecorate);
        ~Label() override;

        bool isContainer() const override;

        juce::Label& getLabel();
        const juce::Label& getLabel() const;

    private:
        void valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property) final;

        PassiveProperty<juce::BorderSize<float>> border;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Label)
    };
} // namespace jive
