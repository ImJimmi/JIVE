#pragma once

namespace jive
{
    class ProgressBar : public GuiItemDecorator
    {
    public:
        explicit ProgressBar(std::unique_ptr<GuiItem> itemToDecorate);

        bool isContainer() const override;

        NormalisedProgressBar& getProgressBar();
        const NormalisedProgressBar& getProgressBar() const;

    private:
        Property<double> value;
        Length width;
        Length height;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProgressBar)
    };
} // namespace jive
