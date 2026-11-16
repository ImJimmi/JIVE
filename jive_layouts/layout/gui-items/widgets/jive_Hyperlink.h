#pragma once

#include "jive_Button.h"

namespace jive
{
    class Hyperlink : public Button
    {
    public:
        explicit Hyperlink(std::unique_ptr<GuiItem> itemToDecorate);

        bool isContainer() const override;

        juce::HyperlinkButton& getHyperlink();
        const juce::HyperlinkButton& getHyperlink() const;

    private:
        void valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property) final;

        PassiveProperty<juce::URL> url;
    };
} // namespace jive
