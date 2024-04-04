#pragma once

#include <jive_layouts/layout/gui-items/jive_ContainerItem.h>

namespace jive
{
    class GridItem : public ContainerItem::Child
    {
    public:
        explicit GridItem(std::unique_ptr<GuiItem> itemToDecorate);

        [[nodiscard]] juce::GridItem toJuceGridItem(juce::Rectangle<float> parentContentBounds,
                                                    LayoutStrategy strategy);

    private:
        Property<int> order;
        Property<juce::GridItem::JustifySelf> justifySelf;
        Property<juce::GridItem::AlignSelf> alignSelf;
        Property<juce::GridItem::StartAndEndProperty> gridColumn;
        Property<juce::GridItem::StartAndEndProperty> gridRow;
        Property<juce::String> gridArea;
    };
} // namespace jive
