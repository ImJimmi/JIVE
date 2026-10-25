#pragma once

#include <jive_layouts/layout/gui-items/jive_ContainerItem.h>

namespace jive
{
    class BlockItem
        : public ContainerItem::Child
        , private juce::ValueTree::Listener
    {
    public:
        explicit BlockItem(std::unique_ptr<GuiItem> itemToDecorate);
        ~BlockItem() override;

        juce::Rectangle<int> calculateBounds() const;

    private:
        void valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property) final;

        int calculateX(float itemWidth) const;
        int calculateY(float itemHeight) const;

        PassiveProperty<Length> x;
        PassiveProperty<Length> y;
        PassiveProperty<Anchor> anchor;
        PassiveProperty<Length> width;
        PassiveProperty<Length> height;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BlockItem)
    };
} // namespace jive
