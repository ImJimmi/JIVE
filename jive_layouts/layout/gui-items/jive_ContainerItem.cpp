#include <jive_layouts/jive_layouts.h>

namespace jive
{
    ContainerItem::ContainerItem(std::unique_ptr<GuiItem> itemToDecorate)
        : GuiItemDecorator{ std::move(itemToDecorate) }
        , idealWidth{ state, "ideal-width" }
        , idealHeight{ state, "ideal-height" }
        , boxModel{ toType<CommonGuiItem>()->boxModel }
    {
        boxModel.addListener(*this);
    }

    ContainerItem::~ContainerItem()
    {
        boxModel.removeListener(*this);
    }

    void ContainerItem::addChild(std::unique_ptr<GuiItem> child)
    {
        GuiItemDecorator::addChild(std::move(child));
        layoutChanged();
    }

    void ContainerItem::boxModelInvalidated(BoxModel& box)
    {
        const auto newIdealSize = calculateIdealSize(box.getBounds());
        const auto idealWidthChanged = newIdealSize.getWidth() != idealWidth.get();
        const auto idealHeightChanged = newIdealSize.getHeight() != idealHeight.get();

        idealWidth = newIdealSize.getWidth();
        idealHeight = newIdealSize.getHeight();

        const auto idealSizeChanged = idealWidthChanged || idealHeightChanged;

        if (!idealSizeChanged || isTopLevel())
            layOutChildren();
    }

    void ContainerItem::layoutChanged()
    {
        const auto newIdealSize = calculateIdealSize({
            static_cast<float>(std::numeric_limits<juce::uint16>::max()),
            static_cast<float>(std::numeric_limits<juce::uint16>::max()),
        });
        idealWidth = newIdealSize.getWidth();
        idealHeight = newIdealSize.getHeight();
    }
} // namespace jive
