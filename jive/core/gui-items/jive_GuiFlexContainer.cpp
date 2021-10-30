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
        item->valueTreePropertyChanged (treeWhosePropertyChanged, propertyID);

        jassert(treeWhosePropertyChanged == tree);

        if (propertyID == flexDirection.getPropertyID())
            flexDirectionChanged();
        else if (propertyID == flexWrap.getPropertyID())
            flexWrapChanged();
        else if (propertyID == flexJustifyContent.getPropertyID())
            flexJustifyContentChanged();
        else if (propertyID == flexAlignItems.getPropertyID())
            flexAlignItemsChanged();
        else if (propertyID == flexAlignContent.getPropertyID())
            flexAlignContentChanged();
    }

    void GuiFlexContainer::componentMovedOrResized(juce::Component& componentThatWasMovedOrResized,
                                                   bool wasMoved,
                                                   bool wasResized)
    {
        item->componentMovedOrResized (componentThatWasMovedOrResized, wasMoved, wasResized);

        jassert(&componentThatWasMovedOrResized == component.get());

        if (!wasResized)
            return;

        updateLayout();
    }

    //==================================================================================================================
    void GuiFlexContainer::flexDirectionChanged()
    {
        flexDirection.forceUpdateOfCachedValue();
        updateLayout();
    }

    void GuiFlexContainer::flexWrapChanged()
    {
        flexWrap.forceUpdateOfCachedValue();
        updateLayout();
    }

    void GuiFlexContainer::flexJustifyContentChanged()
    {
        flexJustifyContent.forceUpdateOfCachedValue();
        updateLayout();
    }

    void GuiFlexContainer::flexAlignItemsChanged()
    {
        flexAlignItems.forceUpdateOfCachedValue();
        updateLayout();
    }

    void GuiFlexContainer::flexAlignContentChanged()
    {
        flexAlignContent.forceUpdateOfCachedValue();
        updateLayout();
    }
} // namespace jive
