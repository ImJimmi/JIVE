#include "jive_GuiItem.h"

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    GuiItem::GuiItem(juce::ValueTree valueTree, std::shared_ptr<juce::Component> comp, GuiItem* parentItem)
        : tree{ valueTree }
        , component{ std::move(comp) }
        , parent{ parentItem }
        , id{ tree, "id" }
        , display{ tree, "display", nullptr, Display::flex }
        , width{ tree, "width", -1 }
        , height{ tree, "height", -1 }
    {
        jassert(component != nullptr);

        id.onValueChange = [this]() {
            getComponent().setComponentID(id.get().toString());
        };
        getComponent().setComponentID(id.get().toString());

        width.onValueChange = [this]() {
            updateComponentSize();
        };
        height.onValueChange = [this]() {
            updateComponentSize();
        };
        updateComponentSize();

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

    GuiItem& GuiItem::getChild(int index) const
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

    BoxModel GuiItem::getBoxModel() const
    {
        return boxModel;
    }

    juce::Identifier GuiItem::getID() const
    {
        return id;
    }

    float GuiItem::getWidth() const
    {
        return width;
    }

    float GuiItem::getHeight() const
    {
        return height;
    }

    GuiItem::Display GuiItem::getDisplay() const
    {
        return display;
    }

    bool GuiItem::hasAutoWidth() const
    {
        return static_cast<int>(width.get()) == -1;
    }

    bool GuiItem::hasAutoHeight() const
    {
        return static_cast<int>(height.get()) == -1;
    }

    //==================================================================================================================
    GuiItem::operator juce::FlexBox() const
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
        jassertquiet(&componentThatWasMovedOrResized == component.get());

        if (!wasResized)
            return;

        updateLayout();
    }

    //==================================================================================================================
    void GuiItem::updateComponentSize()
    {
        const auto borderBounds = boxModel.getBorderBounds();
        component->setSize(juce::roundToInt(borderBounds.getWidth()), juce::roundToInt(borderBounds.getHeight()));
    }
} // namespace jive
