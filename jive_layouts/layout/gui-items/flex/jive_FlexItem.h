#pragma once

#include <jive_layouts/layout/gui-items/jive_ContainerItem.h>

namespace jive
{
    class FlexItem : public ContainerItem::Child
    {
    public:
        explicit FlexItem(std::unique_ptr<GuiItem> itemToDecorate);

        [[nodiscard]] juce::FlexItem toJuceFlexItem(juce::Rectangle<float> parentContentBounds,
                                                    LayoutStrategy strategy);

    private:
        Property<int> order;
        Property<float> flexGrow;
        Property<float> flexShrink;
        Property<float> flexBasis;
        Property<juce::FlexItem::AlignSelf> alignSelf;

        const std::unique_ptr<juce::Component> layoutDummy;
    };
} // namespace jive
