#include "jive_GuiFlexContainer.h"

#include "jive_GuiFlexItem.h"

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    GuiFlexContainer::GuiFlexContainer(std::unique_ptr<GuiItem> itemToDecorate)
        : GuiItemDecorator{ std::move(itemToDecorate) }
        , flexDirection{ tree, "flex-direction", juce::FlexBox::Direction::column }
        , flexWrap{ tree, "flex-wrap" }
        , flexJustifyContent{ tree, "justify-content" }
        , flexAlignItems{ tree, "align-items" }
        , flexAlignContent{ tree, "align-content" }
    {
        flexDirection.onValueChange = [this]() {
            updateLayout();
        };
        flexWrap.onValueChange = [this]() {
            updateLayout();
        };
        flexJustifyContent.onValueChange = [this]() {
            updateLayout();
        };
        flexAlignItems.onValueChange = [this]() {
            updateLayout();
        };
        flexAlignContent.onValueChange = [this]() {
            updateLayout();
        };
    }

    //==================================================================================================================
    float GuiFlexContainer::getHeight() const
    {
        if (hasAutoHeight())
        {
            const auto contentHeight = getMinimumContentHeight();
            const auto box = getBoxModel();

            return contentHeight + box.getPadding().getTopAndBottom() + box.getBorder().getTopAndBottom();
        }

        return item->getHeight();
    }

    //==================================================================================================================
    void GuiFlexContainer::updateLayout()
    {
        auto flex = static_cast<juce::FlexBox>(*this);
        flex.performLayout(getBoxModel().getContentBounds());
    }

    //==================================================================================================================
    GuiFlexContainer::operator juce::FlexBox()
    {
        return getFlexBox();
    }

    //==================================================================================================================
    void appendChildren(GuiFlexContainer& container, juce::FlexBox& flex)
    {
        for (auto& child : container)
        {
            if (auto* decoratedItem = dynamic_cast<GuiItemDecorator*>(&child))
            {
                if (auto* flexItem = decoratedItem->toType<GuiFlexItem>())
                    flex.items.add(*flexItem);
            }
        }
    }

    juce::FlexBox GuiFlexContainer::getFlexBox()
    {
        juce::FlexBox flex;

        flex.flexDirection = flexDirection;
        flex.flexWrap = flexWrap;
        flex.justifyContent = flexJustifyContent;
        flex.alignItems = flexAlignItems;
        flex.alignContent = flexAlignContent;

        appendChildren(*this, flex);

        return flex;
    }

    juce::FlexBox GuiFlexContainer::getFlexBoxWithDummyItems() const
    {
        auto flex = const_cast<GuiFlexContainer*>(this)->getFlexBox();

        for (auto& flexItem : flex.items)
            flexItem.associatedComponent = nullptr;

        return flex;
    }

    float GuiFlexContainer::getMinimumContentHeight() const
    {
        auto flex = getFlexBoxWithDummyItems();
        flex.performLayout(juce::Rectangle<float>{ 0.f, 0.f, getWidth(), std::numeric_limits<float>::max() });

        auto contentHeight = 0.f;

        for (const auto& flexItem : flex.items)
        {
            if (flexItem.currentBounds.getBottom() > contentHeight)
                contentHeight = flexItem.currentBounds.getBottom();
        }

        return contentHeight;
    }
} // namespace jive
