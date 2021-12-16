#include "jive_BoxModel.h"

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    BoxModel::BoxModel(const GuiItem& owningItem, juce::ValueTree valueTree)
        : item{ owningItem }
        , tree{ valueTree }
        , padding{ tree, "padding" }
        , border{ tree, "border-width" }
        , margin{ tree, "margin" }
    {
    }

    //==================================================================================================================
    float BoxModel::getWidth() const
    {
        return item.getWidth();
    }

    float BoxModel::getHeight() const
    {
        return item.getHeight();
    }

    juce::BorderSize<float> BoxModel::getPadding() const
    {
        return padding;
    }

    juce::BorderSize<float> BoxModel::getBorder() const
    {
        return border;
    }

    juce::BorderSize<float> BoxModel::getMargin() const
    {
        return margin;
    }

    juce::Rectangle<float> BoxModel::getBorderBounds() const
    {
        const auto width = item.hasAutoWidth() ? item.getComponent().getWidth() : calculateBorderBoundsWidth();
        const auto height = item.hasAutoHeight() ? item.getComponent().getHeight() : calculateBorderBoundsHeight();

        return { width, height };
    }

    juce::Rectangle<float> BoxModel::getPaddingBounds() const
    {
        return border.get().subtractedFrom(getBorderBounds());
    }

    juce::Rectangle<float> BoxModel::getContentBounds() const
    {
        return padding.get().subtractedFrom(getPaddingBounds());
    }

    //==================================================================================================================
    float BoxModel::calculateBorderBoundsWidth() const
    {
        if (item.getParent() == nullptr)
            return getWidth();

        return getWidth() + padding.get().getLeftAndRight() + border.get().getLeftAndRight();
    }

    float BoxModel::calculateBorderBoundsHeight() const
    {
        if (item.getParent() == nullptr)
            return getHeight();

        return getHeight() + padding.get().getTopAndBottom() + border.get().getTopAndBottom();
    }
} // namespace jive
