#pragma once

#include "jive_GuiItemDecorator.h"

#include <jive_layouts/utilities/jive_LayoutStrategy.h>

namespace jive
{
    class ContainerItem
        : public GuiItemDecorator
        , private BoxModel::Listener
    {
    public:
        class Child : public GuiItemDecorator
        {
        public:
            explicit Child(std::unique_ptr<GuiItem> itemToDecorate);
            ~Child() override;

        protected:
            void applyConstraints(std::variant<std::reference_wrapper<juce::FlexItem>,
                                               std::reference_wrapper<juce::GridItem>> flexOrGridItem,
                                  juce::Rectangle<float> parentContentBounds,
                                  Orientation orientation,
                                  LayoutStrategy strategy) const;

        private:
            class Pimpl;
            const std::unique_ptr<Pimpl> pimpl;
        };

        explicit ContainerItem(std::unique_ptr<GuiItem> itemToDecorate);
        ~ContainerItem() override;

        void insertChild(std::unique_ptr<GuiItem> child, int index) override;
        void setChildren(std::vector<std::unique_ptr<GuiItem>>&& newChildren) override;

    protected:
        void boxModelInvalidated(BoxModel& boxModel) override;

        virtual juce::Rectangle<float> calculateIdealSize(juce::Rectangle<float> constraints) const = 0;

        void layoutChanged();

    private:
        BoxModel& box;
        Property<float> idealWidth;
        Property<float> idealHeight;
    };
} // namespace jive
