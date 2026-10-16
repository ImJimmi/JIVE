#pragma once

#include <jive_layouts/layout/gui-items/jive_GuiItemDecorator.h>

namespace jive
{
    class ProgressBar
        : public GuiItemDecorator
        , private juce::ValueTree::Listener
    {
    public:
        explicit ProgressBar(std::unique_ptr<GuiItem> itemToDecorate);
        ~ProgressBar() override;

        bool isContainer() const override;

        NormalisedProgressBar& getProgressBar();
        const NormalisedProgressBar& getProgressBar() const;

    private:
        void valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property) final;

        PassiveProperty<double> value;
        PassiveProperty<Length> width;
        PassiveProperty<Length> height;
        PassiveProperty<bool> focusable;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProgressBar)
    };
} // namespace jive
