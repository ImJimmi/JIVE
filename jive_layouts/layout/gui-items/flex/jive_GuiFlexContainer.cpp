#include "jive_GuiFlexContainer.h"

#include "jive_GuiFlexItem.h"

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    GuiFlexContainer::GuiFlexContainer(std::unique_ptr<GuiItem> itemToDecorate)
        : GuiItemDecorator{ std::move(itemToDecorate) }
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

    GuiItem& GuiFlexContainer::getChild(int index) const
    {
        return item->getChild(index);
    }

    float GuiFlexContainer::getHeight() const
    {
        if (hasAutoHeight())
        {
            const auto contentHeight = getMinimumContentHeight();
            const auto boxModel = getBoxModel();

            return contentHeight + boxModel.getPadding().getTopAndBottom() + boxModel.getBorder().getTopAndBottom();
        }

        return item->getHeight();
    }

    //==================================================================================================================
    void GuiFlexContainer::updateLayout()
    {
        forceUpdateOfAllCachedValues();

        auto flex = static_cast<juce::FlexBox>(*this);
        flex.performLayout(getBoxModel().getContentBounds());
    }

    //==================================================================================================================
    GuiFlexContainer::operator juce::FlexBox() const
    {
        return getFlexBox();
    }

    //==================================================================================================================
    void GuiFlexContainer::valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyChanged,
                                                    const juce::Identifier& propertyID)
    {
        GuiItemDecorator::valueTreePropertyChanged(treeWhosePropertyChanged, propertyID);

        if (treeWhosePropertyChanged != tree)
            return;

        updateLayout();
    }

    //==================================================================================================================
    void appendChildren(const GuiFlexContainer& container, juce::FlexBox& flex)
    {
        for (auto i = 0; i < container.getNumChildren(); i++)
            flex.items.add(container.getChild(i));
    }

    juce::FlexBox GuiFlexContainer::getFlexBox() const
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
        auto flex = getFlexBox();

        for (auto& item : flex.items)
            item.associatedComponent = nullptr;

        return flex;
    }

    float GuiFlexContainer::getMinimumContentHeight() const
    {
        auto flex = getFlexBoxWithDummyItems();
        flex.performLayout(juce::Rectangle<float>{ 0.f, 0.f, getWidth(), std::numeric_limits<float>::max() });

        auto contentHeight = 0.f;

        for (const auto& item : flex.items)
        {
            if (item.currentBounds.getBottom() > contentHeight)
                contentHeight = item.currentBounds.getBottom();
        }

        return contentHeight;
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
