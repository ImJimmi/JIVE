#pragma once

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    class BoxModel : private juce::ComponentListener
    {
    public:
        //==============================================================================================================
        BoxModel(std::shared_ptr<juce::Component> component,
                 juce::ValueTree stateSource);

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

        //==============================================================================================================
        juce::ValueTree state;

    private:
        //==============================================================================================================
        const std::shared_ptr<juce::Component> component;

        Length width;
        Length height;
        TypedValue<juce::BorderSize<float>> padding;
        TypedValue<juce::BorderSize<float>> border;
        TypedValue<juce::BorderSize<float>> margin;
        std::shared_ptr<TypedValue<float>> parentWidth;
        std::shared_ptr<TypedValue<float>> parentHeight;

        //==============================================================================================================
        JUCE_LEAK_DETECTOR(BoxModel)
    };
} // namespace jive
