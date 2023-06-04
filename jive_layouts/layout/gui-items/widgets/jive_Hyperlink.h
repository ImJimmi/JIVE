#pragma once

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
        Property<juce::URL> url;
    };
} // namespace jive
