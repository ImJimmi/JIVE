#pragma once

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    class BoxModel
    {
    public:
        //==============================================================================================================
        BoxModel(const GuiItem& owningItem, juce::ValueTree valueTree);

        //==============================================================================================================
        float getWidth() const;
        float getHeight() const;

        juce::BorderSize<float> getPadding() const;
        juce::BorderSize<float> getBorder() const;
        juce::BorderSize<float> getMargin() const;

        juce::Rectangle<float> getBorderBounds() const;
        juce::Rectangle<float> getPaddingBounds() const;
        juce::Rectangle<float> getContentBounds() const;

    private:
        //==============================================================================================================
        float calculateBorderBoundsWidth() const;
        float calculateBorderBoundsHeight() const;

        //==============================================================================================================
        const GuiItem& item;
        juce::ValueTree tree;

        TypedValue<juce::BorderSize<float>> padding;
        TypedValue<juce::BorderSize<float>> border;
        TypedValue<juce::BorderSize<float>> margin;
    };
} // namespace jive
