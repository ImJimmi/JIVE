#include <jive_layouts/jive_layouts.h>

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    GuiItemDecorator::GuiItemDecorator(std::unique_ptr<GuiItem> itemToDecorate)
        : GuiItem{ *itemToDecorate }
        , item{ std::move(itemToDecorate) }
    {
        if (auto* decorator = dynamic_cast<GuiItemDecorator*>(item.get()))
            decorator->owner = this;
    }

    //==================================================================================================================
    void GuiItemDecorator::addChild(std::unique_ptr<GuiItem> child)
    {
        item->addChild(std::move(child));
    }

    int GuiItemDecorator::getNumChildren() const
    {
        return item->getNumChildren();
    }

    GuiItem& GuiItemDecorator::getChild(int index) const
    {
        return item->getChild(index);
    }

    const GuiItem* GuiItemDecorator::getParent() const
    {
        return const_cast<GuiItemDecorator*>(this)->getParent();
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

    //==================================================================================================================
    void GuiItemDecorator::layOutChildren()
    {
        item->layOutChildren();
    }

    //==================================================================================================================
    GuiItem::Iterator GuiItemDecorator::begin()
    {
        return std::begin(*item);
    }

    const GuiItem::Iterator GuiItemDecorator::begin() const
    {
        return std::begin(*item);
    }

    GuiItem::Iterator GuiItemDecorator::end()
    {
        return std::end(*item);
    }

    const GuiItem::Iterator GuiItemDecorator::end() const
    {
        return std::end(*item);
    }
} // namespace jive
