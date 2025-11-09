#include "jive_ContainerItem.h"

#include "jive_CommonGuiItem.h"

namespace jive
{
    class ContainerItem::Child::Pimpl
    {
    public:
        explicit Pimpl(jive::GuiItemDecorator& item)
            : state{ item.state }
            , order{ state, "order" }
            , width{ state, "width" }
            , height{ state, "height" }
            , idealWidth{ state, "ideal-width" }
            , idealHeight{ state, "ideal-height" }
            , boxModel{ item.toType<CommonGuiItem>()->boxModel }
        {
        }

        template <typename FlexOrGridItem>
        void applyConstraints(FlexOrGridItem& item,
                              juce::Rectangle<float> parentContentBounds,
                              Orientation orientation,
                              LayoutStrategy strategy) const
        {
            const auto minBounds = boxModel.getMinimumBounds();
            item.minWidth = minBounds.getWidth();
            item.minHeight = minBounds.getHeight();

            const auto maxBounds = boxModel.getMaximumBounds();
            item.maxWidth = maxBounds.getWidth();
            item.maxHeight = maxBounds.getHeight();

            item.order = order.get();
            item.margin = getMargin<FlexOrGridItem>();

            if (strategy == LayoutStrategy::dummy)
                item.associatedComponent = nullptr;

            switch (orientation)
            {
            case Orientation::vertical:
                applyConstraintsForVerticalOrientation(item,
                                                       parentContentBounds,
                                                       strategy);
                break;
            case Orientation::horizontal:
                applyConstraintsForHorizontalOrientation(item,
                                                         parentContentBounds,
                                                         strategy);
                break;
            }
        }

    private:
        template <typename FlexOrGridItem>
        void applyConstraintsForVerticalOrientation(FlexOrGridItem& item,
                                                    juce::Rectangle<float> parentContentBounds,
                                                    LayoutStrategy strategy) const
        {
            if (!width.isAuto())
            {
                item.width = width.get().toPixels(strategy == LayoutStrategy::real ? parentContentBounds.getWidth() : 0.0f);
            }
            else if (idealWidth.exists())
            {
                if (idealWidth.get() < parentContentBounds.getWidth() || strategy == LayoutStrategy::dummy)
                    item.minWidth = juce::jmax(item.minWidth, idealWidth.get());
                else
                    item.width = parentContentBounds.getWidth();
            }

            if (!height.isAuto())
            {
                item.height = height.get().toPixels(strategy == LayoutStrategy::real ? parentContentBounds.getHeight() : 0.0f);
            }
            else if (idealHeight.exists())
            {
                const auto property = state["ideal-height"];

                if (const auto calculateHeight = property.getNativeFunction();
                    calculateHeight != nullptr)
                {
                    juce::var args[] = {
                        strategy == LayoutStrategy::dummy
                            ? juce::jmin(idealWidth.get(), parentContentBounds.getWidth())
                            : juce::jmax(item.width, item.minWidth),
                    };
                    item.minHeight = juce::jmax(item.minHeight,
                                                static_cast<float>(calculateHeight({ property, args, 1 })));
                }
                else
                {
                    item.minHeight = juce::jmax(item.minHeight,
                                                static_cast<float>(property));
                }
            }
        }

        template <typename FlexOrGridItem>
        void applyConstraintsForHorizontalOrientation(FlexOrGridItem& item,
                                                      juce::Rectangle<float> parentContentBounds,
                                                      LayoutStrategy strategy) const
        {
            if (!width.isAuto())
                item.width = width.get().toPixels(strategy == LayoutStrategy::real ? parentContentBounds.getWidth() : 0.0f);
            else if (idealWidth.exists())
                item.width = idealWidth.get();

            if (!height.isAuto())
            {
                item.height = height.get().toPixels(strategy == LayoutStrategy::real ? parentContentBounds.getHeight() : 0.0f);
            }
            else if (idealHeight.exists())
            {
                const auto property = state["ideal-height"];

                if (const auto calculateHeight = property.getNativeFunction();
                    calculateHeight != nullptr)
                {
                    juce::var args[] = {
                        strategy == LayoutStrategy::dummy
                            ? juce::jmin(idealWidth.get(), parentContentBounds.getWidth())
                            : juce::jmax(item.width, item.minWidth),
                    };
                    const auto idealHeightAbsolute = static_cast<float>(calculateHeight({ property, args, 1 }));

                    if (idealHeightAbsolute < parentContentBounds.getHeight() || strategy == LayoutStrategy::dummy)
                        item.minHeight = juce::jmax(item.minHeight, idealHeightAbsolute);
                    else
                        item.height = parentContentBounds.getHeight();
                }
                else
                {
                    item.minHeight = juce::jmax(item.minHeight,
                                                static_cast<float>(property));
                }
            }
        }

        template <typename FlexOrGridItem>
        [[nodiscard]] auto getMargin() const
        {
            const auto margin = boxModel.getMargin();
            using Margin = typename FlexOrGridItem::Margin;

            return Margin{
                margin.getTop(),
                margin.getRight(),
                margin.getBottom(),
                margin.getLeft(),
            };
        }

        const juce::ValueTree state;
        const Property<int> order;
        const Property<Length> width;
        const Property<Length> height;
        const Property<float> idealWidth;
        const Property<float> idealHeight;
        const BoxModel& boxModel;
    };

    ContainerItem::Child::Child(std::unique_ptr<GuiItem> itemToDecorate)
        : GuiItemDecorator{ std::move(itemToDecorate) }
        , pimpl{ std::make_unique<Pimpl>(*this) }
        , idealWidth{ state, "ideal-width" }
        , idealHeight{ state, "ideal-height" }
    {
        getComponent()->addComponentListener(this);
    }

    ContainerItem::Child::~Child()
    {
        getComponent()->removeComponentListener(this);
    }

    void ContainerItem::Child::applyConstraints(std::variant<std::reference_wrapper<juce::FlexItem>,
                                                             std::reference_wrapper<juce::GridItem>> flexOrGridItem,
                                                juce::Rectangle<float> parentContentBounds,
                                                Orientation orientation,
                                                LayoutStrategy strategy) const
    {
        const auto visit = [this,
                            parentContentBounds,
                            orientation,
                            strategy](auto&& gridOrFlexItem) {
            pimpl->applyConstraints(gridOrFlexItem.get(),
                                    parentContentBounds,
                                    orientation,
                                    strategy);
        };
        std::visit(visit, flexOrGridItem);
    }

    void ContainerItem::Child::componentMovedOrResized(juce::Component&, bool, bool resized)
    {
        if (!resized)
            return;

        if (auto* container = getTopLevelDecorator().toType<ContainerItem>())
        {
            if (getComponent()->getWidth() < idealWidth.get() || getComponent()->getHeight() < idealHeight.get())
                container->updateIdealSizeWithinConstraints();
        }
    }
} // namespace jive
