#include "jive_GuiFlexContainer.h"

#include "jive_GuiFlexItem.h"

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    GuiFlexContainer::GuiFlexContainer(std::unique_ptr<GuiItem> itemToDecorate)
        : GuiItem{ *itemToDecorate }
        , item{ std::move(itemToDecorate) }
        , flexDirection{ tree, "flex-direction", nullptr, juce::FlexBox::Direction::column }
        , flexWrap{ tree, "flex-wrap", nullptr }
        , flexJustifyContent{ tree, "justify-content", nullptr }
        , flexAlignItems{ tree, "align-items", nullptr }
        , flexAlignContent{ tree, "align-content", nullptr }
    {
    }

    //==================================================================================================================
    void GuiFlexContainer::addChild(std::unique_ptr<GuiItem> child)
    {
        item->addChild(std::move(child));
    }

    int GuiFlexContainer::getNumChildren() const
    {
        return item->getNumChildren();
    }

    GuiItem& GuiFlexContainer::getChild(int index)
    {
        return item->getChild(index);
    }

    //==================================================================================================================
    void GuiFlexContainer::updateLayout()
    {
        forceUpdateOfAllCachedValues();

        auto flex = static_cast<juce::FlexBox>(*this);
        flex.performLayout(getComponent().getLocalBounds());
    }

    //==================================================================================================================
    void appendChildren (GuiFlexContainer& container, juce::FlexBox& flex)
    {
        for (auto i = 0; i < container.getNumChildren(); i++)
        {
            if (auto* flexItem = dynamic_cast<GuiFlexItem*>(&container.getChild(i)))
            {
                flex.items.add(*flexItem);
            }
            else
            {
                // Each child of a GuiFlexContainer should be a GuiFlexItem!
                jassertfalse;
            }
        }
    }

    GuiFlexContainer::operator juce::FlexBox()
    {
        juce::FlexBox flex;

        flex.flexDirection = flexDirection;
        flex.flexWrap = flexWrap;
        flex.justifyContent = flexJustifyContent;
        flex.alignItems = flexAlignItems;
        flex.alignContent = flexAlignContent;

        appendChildren (*this, flex);

        return flex;
    }

    //==================================================================================================================
    void GuiFlexContainer::valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyChanged,
                                                    const juce::Identifier& propertyID)
    {
        jassert(treeWhosePropertyChanged == tree);

        item->valueTreePropertyChanged (treeWhosePropertyChanged, propertyID);
        updateLayout();
    }

    void GuiFlexContainer::componentMovedOrResized(juce::Component& componentThatWasMovedOrResized,
                                                   bool wasMoved,
                                                   bool wasResized)
    {
        item->componentMovedOrResized (componentThatWasMovedOrResized, wasMoved, wasResized);
        GuiItem::componentMovedOrResized (componentThatWasMovedOrResized, wasMoved, wasResized);
    }

    //==================================================================================================================
    void GuiFlexContainer::forceUpdateOfAllCachedValues()
    {
        flexDirection.forceUpdateOfCachedValue();
        flexWrap.forceUpdateOfCachedValue();
        flexJustifyContent.forceUpdateOfCachedValue();
        flexAlignItems.forceUpdateOfCachedValue();
        flexAlignContent.forceUpdateOfCachedValue();
    }
} // namespace jive
