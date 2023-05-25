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

    void GuiItemDecorator::addChild(std::unique_ptr<GuiItem> child)
    {
        item->addChild(std::move(child));
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
