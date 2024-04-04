#pragma once

#include <jive_layouts/layout/gui-items/jive_GuiItemDecorator.h>

#include <jive_components/jive_components.h>

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
        Property<bool> focusable;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProgressBar)
    };
} // namespace jive
