#include "jive_GuiItem.h"

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    GuiItem::GuiItem(std::unique_ptr<juce::Component> comp, juce::ValueTree valueTree, GuiItem* parentItem)
        : component{ std::move(comp) }
        , tree{ valueTree }
        , parent{ parentItem }
        , width{ tree, "width", nullptr, -1 }
        , height{ tree, "height", nullptr, -1 }
        , display{ tree, "display", nullptr, Display::flex }
        , flexDirection{ tree, "flex-direction", nullptr, juce::FlexBox::Direction::column }
        , flexWrap{ tree, "flex-wrap", nullptr }
        , flexJustifyContent{ tree, "justify-content", nullptr }
        , flexAlignItems{ tree, "align-items", nullptr }
        , flexAlignContent{ tree, "align-content", nullptr }
        , flexItemOrder{ tree, "order", nullptr }
        , flexItemGrow{ tree, "flex-grow", nullptr }
        , flexItemAlignSelf{ tree, "align-self", nullptr, juce::FlexItem::AlignSelf::autoAlign }
    {
        jassert(component != nullptr);

        component->addComponentListener(this);
        tree.addListener(this);
    }

    //==================================================================================================================
    void GuiItem::addChild(std::unique_ptr<GuiItem> child)
    {
        auto* newlyAddedChild = children.add(std::move(child));
        component->addAndMakeVisible(newlyAddedChild->getComponent());
    }

    //==================================================================================================================
    juce::Component& GuiItem::getComponent()
    {
        return *component;
    }

    const juce::Component& GuiItem::getComponent() const
    {
        return *component;
    }

    int GuiItem::getNumChildren() const
    {
        return children.size();
    }

    GuiItem& GuiItem::getChild(int index)
    {
        jassert(index < children.size());
        return *children[index];
    }

    const GuiItem* GuiItem::getParent()
    {
        return parent;
    }

    //==================================================================================================================
    GuiItem::operator juce::FlexBox()
    {
        juce::FlexBox flex;
        flex.flexDirection = flexDirection;
        flex.flexWrap = flexWrap;
        flex.justifyContent = flexJustifyContent;
        flex.alignItems = flexAlignItems;
        flex.alignContent = flexAlignContent;

        for (auto* child : children)
            flex.items.add(*child);

        return flex;
    }

    GuiItem::operator juce::FlexItem()
    {
        juce::FlexItem item{ *component };

        item.width = static_cast<float>(width);
        item.height = static_cast<float>(height);

        item.order = flexItemOrder;
        item.flexGrow = flexItemGrow;
        item.alignSelf = flexItemAlignSelf;

        return item;
    }

    //==================================================================================================================
    void GuiItem::valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyChanged,
                                           const juce::Identifier& propertyID)
    {
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
        else if (propertyID == flexItemOrder.getPropertyID())
            flexItemOrderChanged();
        else if (propertyID == flexItemGrow.getPropertyID())
            flexItemGrowChanged();
        else if (propertyID == flexItemAlignSelf.getPropertyID())
            flexItemAlignSelfChanged();
    }

    void GuiItem::componentMovedOrResized(juce::Component& componentThatWasMovedOrResized,
                                          bool /*wasMoved*/,
                                          bool wasResized)
    {
        jassert(&componentThatWasMovedOrResized == component.get());

        if (!wasResized)
            return;

        updateLayout();
    }

    //==================================================================================================================
    void GuiItem::flexDirectionChanged()
    {
        flexDirection.forceUpdateOfCachedValue();
        updateLayout();
    }

    void GuiItem::flexWrapChanged()
    {
        flexWrap.forceUpdateOfCachedValue();
        updateLayout();
    }

    void GuiItem::flexJustifyContentChanged()
    {
        flexJustifyContent.forceUpdateOfCachedValue();
        updateLayout();
    }

    void GuiItem::flexAlignItemsChanged()
    {
        flexAlignItems.forceUpdateOfCachedValue();
        updateLayout();
    }

    void GuiItem::flexAlignContentChanged()
    {
        flexAlignContent.forceUpdateOfCachedValue();
        updateLayout();
    }

    void GuiItem::flexItemOrderChanged()
    {
        // You can only set a flex-item property on an item that has a parent!
        jassert(parent != nullptr);

        flexItemOrder.forceUpdateOfCachedValue();
        parent->updateLayout();
    }

    void GuiItem::flexItemGrowChanged()
    {
        // You can only set a flex-item property on an item that has a parent!
        jassert(parent != nullptr);

        flexItemGrow.forceUpdateOfCachedValue();
        parent->updateLayout();
    }

    void GuiItem::flexItemAlignSelfChanged()
    {
        // You can only set a flex-item property on an item that has a parent!
        jassert(parent != nullptr);

        flexItemAlignSelf.forceUpdateOfCachedValue();
        parent->updateLayout();
    }

    void performFlexLayout(GuiItem& item)
    {
        auto flex = static_cast<juce::FlexBox>(item);
        flex.performLayout(item.getComponent().getLocalBounds());
    }

    void GuiItem::updateLayout()
    {
        switch (display)
        {
        case Display::flex:
            performFlexLayout(*this);
            break;
        default:
            // Unhandled display type
            jassertfalse;
        }
    }
} // namespace jive

//======================================================================================================================
namespace juce
{
    //==================================================================================================================
    String& operator<<(String& text, jive::GuiItem::Display display)
    {
        text << juce::VariantConverter<jive::GuiItem::Display>::toVar(display).toString();
        return text;
    }
} // namespace juce
