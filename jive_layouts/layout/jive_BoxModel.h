#pragma once

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    class BoxModel
    {
    public:
        //==============================================================================================================
        explicit BoxModel(GuiItem& owningItem);

        //==============================================================================================================
        float getWidth() const;
        void setWidth(float width);
        float getHeight() const;
        void setHeight(float height);

        juce::BorderSize<float> getPadding() const;
        juce::BorderSize<float> getBorder() const;
        juce::BorderSize<float> getMargin() const;

        juce::Rectangle<float> getBorderBounds() const;
        juce::Rectangle<float> getPaddingBounds() const;
        juce::Rectangle<float> getContentBounds() const;

        //==============================================================================================================
        juce::Rectangle<float> calculateContentBounds(const juce::Component& component);

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
        Length width;
        Length height;
        TypedValue<float> explicitWidth;
        TypedValue<float> explicitHeight;
        std::shared_ptr<TypedValue<float>> parentWidth;
        std::shared_ptr<TypedValue<float>> parentHeight;
    };
} // namespace jive
