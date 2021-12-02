#include "jive_GuiItem.h"

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    GuiItem::GuiItem(juce::ValueTree valueTree, std::shared_ptr<juce::Component> comp, GuiItem* parentItem)
        : tree{ valueTree }
        , component{ std::move(comp) }
        , parent{ parentItem }
        , width{ tree, "width", nullptr, -1 }
        , height{ tree, "height", nullptr, -1 }
        , padding{ tree, "padding", nullptr }
        , display{ tree, "display", nullptr, Display::flex }
    {
        jassert(component != nullptr);

        component->addComponentListener(this);
        tree.addListener(this);
    }

    GuiItem::GuiItem(std::unique_ptr<juce::Component> comp, juce::ValueTree valueTree, GuiItem* parentItem)
        : GuiItem{ valueTree, std::move(comp), parentItem }
    {
    }

    GuiItem::GuiItem(const GuiItem& other)
        : GuiItem{ other.tree, other.component, other.parent }
    {
    }

    GuiItem::~GuiItem()
    {
        component->removeComponentListener(this);
    }

    //==================================================================================================================
    void GuiItem::updateLayout()
    {
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

    void GuiItem::addChild(std::unique_ptr<GuiItem> child)
    {
        auto* newlyAddedChild = children.add(std::move(child));
        component->addAndMakeVisible(newlyAddedChild->getComponent());
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

    const GuiItem* GuiItem::getParent() const
    {
        return parent;
    }

    GuiItem* GuiItem::getParent()
    {
        return parent;
    }

    bool GuiItem::isContainer() const
    {
        return true;
    }

    float GuiItem::getWidth() const
    {
        return width;
    }

    float GuiItem::getHeight() const
    {
        return height;
    }

    juce::BorderSize<float> GuiItem::getPadding() const
    {
        return padding;
    }

    GuiItem::Display GuiItem::getDisplay() const
    {
        return display;
    }

    //==================================================================================================================
    GuiItem::operator juce::FlexBox()
    {
        // This item isn't a flex container!
        jassertfalse;

        return {};
    }

    GuiItem::operator juce::FlexItem()
    {
        // This item isn't a flex item!
        jassertfalse;

        return {};
    }

    //==================================================================================================================
    void GuiItem::valueTreePropertyChanged(juce::ValueTree& /*treeWhosePropertyChanged*/,
                                           const juce::Identifier& /*propertyID*/)
    {
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
} // namespace jive
