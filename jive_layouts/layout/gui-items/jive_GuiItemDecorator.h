#pragma once

namespace jive
{
    class GuiItemDecorator : public GuiItem
    {
    public:
        explicit GuiItemDecorator(std::unique_ptr<GuiItem> itemToDecorate);

        void insertChild(std::unique_ptr<GuiItem> child, int index) override;
        void setChildren(std::vector<std::unique_ptr<GuiItem>>&& children) override;
        void removeChild(GuiItem& childToRemove) override;
        juce::Array<GuiItem*> getChildren() override;
        juce::Array<const GuiItem*> getChildren() const override;
        const GuiItem* getParent() const override;
        GuiItem* getParent() override;

        bool isContainer() const override;
        bool isContent() const override;

        GuiItemDecorator& getTopLevelDecorator();
        const GuiItemDecorator& getTopLevelDecorator() const;

        template <typename ItemType>
        ItemType* toType()
        {
            if (auto* itemWithType = dynamic_cast<ItemType*>(this))
                return itemWithType;
            else if (auto* decoratedDecorator = dynamic_cast<GuiItemDecorator*>(item.get()))
                return decoratedDecorator->toType<ItemType>();

            return nullptr;
        }

        template <typename ItemType>
        const ItemType* toType() const
        {
            return const_cast<GuiItemDecorator*>(this)->toType<ItemType>();
        }

        void layOutChildren() override;

        const std::unique_ptr<GuiItem> item;

    private:
        GuiItemDecorator* owner = nullptr;

        JUCE_LEAK_DETECTOR(GuiItemDecorator)
    };
} // namespace jive
