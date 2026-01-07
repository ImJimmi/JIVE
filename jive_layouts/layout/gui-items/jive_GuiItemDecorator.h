#pragma once

#include "jive_GuiItem.h"

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

#if JIVE_IS_PLUGIN_PROJECT
        void attachToParameter(juce::RangedAudioParameter*, juce::UndoManager*) override;
#endif

        void layOutChildren() override;

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

        // Calling this will leave this item in an invalid state. Make sure you
        // delete this decorator immediately after taking ownership of the
        // underlying item returned by this method.
        [[nodiscard]] std::unique_ptr<GuiItem> stripAllDecorators();

    protected:
        std::unique_ptr<GuiItem> item;

    private:
        GuiItemDecorator* owner = nullptr;

        JUCE_LEAK_DETECTOR(GuiItemDecorator)
    };

    [[nodiscard]] std::unique_ptr<GuiItem> decorate(std::unique_ptr<GuiItem> item,
                                                    const std::vector<std::pair<juce::Identifier, std::function<std::unique_ptr<GuiItemDecorator>(std::unique_ptr<GuiItem>)>>>& customDecorators);
} // namespace jive
