#pragma once

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    class BoxModel
    {
    public:
        //==============================================================================================================
        explicit BoxModel(juce::ValueTree stateSource);

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
        juce::ValueTree state;

    private:
        //==============================================================================================================
        Length width;
        Length height;
        TypedValue<juce::BorderSize<float>> padding;
        TypedValue<juce::BorderSize<float>> border;
        TypedValue<juce::BorderSize<float>> margin;
        std::shared_ptr<TypedValue<float>> parentWidth;
        std::shared_ptr<TypedValue<float>> parentHeight;
        TypedValue<float> componentWidth;
        TypedValue<float> componentHeight;
        std::unique_ptr<BoxModel> parentBoxModel;

        //==============================================================================================================
        JUCE_LEAK_DETECTOR(BoxModel)
    };
} // namespace jive
