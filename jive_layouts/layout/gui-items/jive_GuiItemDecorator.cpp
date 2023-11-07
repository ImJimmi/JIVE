#include <jive_layouts/jive_layouts.h>

namespace jive
{
    GuiItemDecorator::GuiItemDecorator(std::unique_ptr<GuiItem> itemToDecorate)
        : GuiItem{ *itemToDecorate }
        , item{ std::move(itemToDecorate) }
    {
        if (auto* decorator = dynamic_cast<GuiItemDecorator*>(item.get()))
            decorator->owner = this;
    }

    void GuiItemDecorator::insertChild(std::unique_ptr<GuiItem> child, int index)
    {
        item->insertChild(std::move(child), index);
        childrenChanged();
    }

    void GuiItemDecorator::setChildren(std::vector<std::unique_ptr<GuiItem>>&& newChildren)
    {
        item->setChildren(std::move(newChildren));
        childrenChanged();
    }

    void GuiItemDecorator::removeChild(GuiItem& child)
    {
        item->removeChild(child);
    }

    juce::Array<GuiItem*> GuiItemDecorator::getChildren()
    {
        return item->getChildren();
    }

    juce::Array<const GuiItem*> GuiItemDecorator::getChildren() const
    {
        return juce::Array<const GuiItem*>{ item->getChildren().getRawDataPointer(), item->getChildren().size() };
    }

    const GuiItem* GuiItemDecorator::getParent() const
    {
        return const_cast<GuiItemDecorator*>(this)->getParent();
    }

    bool GuiItemDecorator::isContainer() const
    {
        return item->isContainer();
    }

    bool GuiItemDecorator::isContent() const
    {
        return item->isContent();
    }

    GuiItem* GuiItemDecorator::getParent()
    {
        if (auto* parentItem = item->getParent())
        {
            if (auto* decoratedParent = dynamic_cast<GuiItemDecorator*>(parentItem))
                return &decoratedParent->getTopLevelDecorator();
        }

        return item->getParent();
    }

    GuiItemDecorator& GuiItemDecorator::getTopLevelDecorator()
    {
        if (owner != nullptr)
            return owner->getTopLevelDecorator();

        return *this;
    }

    const GuiItemDecorator& GuiItemDecorator::getTopLevelDecorator() const
    {
        if (owner != nullptr)
            return owner->getTopLevelDecorator();

        return *this;
    }

    void GuiItemDecorator::layOutChildren()
    {
        item->layOutChildren();
    }
} // namespace jive
