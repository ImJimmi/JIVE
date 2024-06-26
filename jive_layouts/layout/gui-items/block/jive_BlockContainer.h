#pragma once

#include <jive_layouts/layout/gui-items/jive_ContainerItem.h>

namespace jive
{
    class BlockContainer : public ContainerItem
    {
    public:
        explicit BlockContainer(std::unique_ptr<GuiItem> itemToDecorate);

        void layOutChildren() override;

    protected:
        juce::Rectangle<float> calculateIdealSize(juce::Rectangle<float> constraints) const override;

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BlockContainer)
    };
} // namespace jive
