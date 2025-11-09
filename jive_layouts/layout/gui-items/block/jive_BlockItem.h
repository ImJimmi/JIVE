#pragma once

#include <jive_layouts/layout/gui-items/jive_ContainerItem.h>

namespace jive
{
    class BlockItem : public ContainerItem::Child
    {
    public:
        explicit BlockItem(std::unique_ptr<GuiItem> itemToDecorate);

        juce::Rectangle<int> calculateBounds() const;

    private:
        int calculateX() const;
        int calculateY() const;

        Property<Length> x;
        Property<Length> y;
        Property<Length> centreX;
        Property<Length> centreY;
        Property<Length> width;
        Property<Length> height;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BlockItem)
    };
} // namespace jive
