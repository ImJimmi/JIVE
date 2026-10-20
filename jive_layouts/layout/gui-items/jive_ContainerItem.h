#pragma once

#include "jive_GuiItemDecorator.h"

#include <jive_layouts/utilities/jive_LayoutStrategy.h>

namespace std
{
    template <>
    struct hash<std::pair<juce::Rectangle<float>, jive::LayoutStrategy>>
    {
        [[nodiscard]] std::size_t operator()(const std::pair<juce::Rectangle<float>, jive::LayoutStrategy>& pair) const noexcept
        {
            auto result = std::hash<int>{}(static_cast<int>(pair.second));

            for (const auto value : { pair.first.getX(), pair.first.getY(), pair.first.getWidth(), pair.first.getHeight() })
                result = (result * 31) + std::hash<float>{}(value);

            return result;
        }
    };
} // namespace std

namespace jive
{
    class ContainerItem
        : public GuiItemDecorator
        , private BoxModel::Listener
        , private juce::ValueTree::Listener
    {
    public:
        class Child
            : public GuiItemDecorator
            , private juce::ComponentListener
        {
        public:
            explicit Child(std::unique_ptr<GuiItem> itemToDecorate);
            ~Child() override;

            // Lets the item know its ideal size may have changed without its
            // box model changing, e.g. when text is laid out differently.
            virtual void idealSizeChanged() {}

        protected:
            using ItemCacheKey = std::pair<juce::Rectangle<float>, jive::LayoutStrategy>;
            static constexpr std::size_t maxNumCachedItems = 8;

            void applyConstraints(std::variant<std::reference_wrapper<juce::FlexItem>,
                                               std::reference_wrapper<juce::GridItem>> flexOrGridItem,
                                  juce::Rectangle<float> parentContentBounds,
                                  Orientation orientation,
                                  LayoutStrategy strategy) const;

        private:
            void componentMovedOrResized(juce::Component&, bool, bool) final;

            class Pimpl;
            const std::unique_ptr<Pimpl> pimpl;

            PassiveProperty<float> idealWidth;
            PassiveProperty<float> idealHeight;
        };

        explicit ContainerItem(std::unique_ptr<GuiItem> itemToDecorate);
        ~ContainerItem() override;

        void insertChild(std::unique_ptr<GuiItem> child, int index) override;
        void setChildren(std::vector<std::unique_ptr<GuiItem>>&& newChildren) override;

        void updateIdealSize(bool informParentOfChanges = true, bool measureChildren = true);

        // Lets the container know its ideal size may have changed without its
        // state changing, e.g. when a child's text is laid out differently.
        void invalidateIdealSize();

    protected:
        virtual juce::Rectangle<float> calculateIdealSize(juce::Rectangle<float> constraints) const = 0;

        void valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& id) override;

    private:
        void boxModelChanged(BoxModel&) final;
        void valueTreeChildAdded(juce::ValueTree& parent, juce::ValueTree& child) final;
        void valueTreeChildRemoved(juce::ValueTree& parent, juce::ValueTree& child, int index) final;
        void valueTreeChildOrderChanged(juce::ValueTree& parent, int oldIndex, int newIndex) final;

        void updateIdealSizeIfOutOfDate();
        void updateChildrensIdealSizesIfOutOfDate();

        [[nodiscard]] juce::Rectangle<float> getContentConstraints() const;

        BoxModel& box;
        PassiveProperty<float> idealWidth;
        PassiveProperty<float> idealHeight;
        juce::Rectangle<float> explicitConstraintsWhenIdealSizeWasMeasured;
        juce::Rectangle<float> constraintsWhenIdealSizeWasMeasured;
        bool idealSizeWasLimitedByConstraints = false;
        bool idealSizeIsOutOfDate = true;
        bool childrenMayBeOutOfDate = true;
    };
} // namespace jive
