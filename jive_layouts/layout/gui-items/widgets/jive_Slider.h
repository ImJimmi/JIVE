#pragma once

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    class Slider
        : public GuiItemDecorator
        , private juce::Slider::Listener
    {
    public:
        //==============================================================================================================
        explicit Slider(std::unique_ptr<GuiItem> itemToDecorate);

        //==============================================================================================================
        bool isContainer() const override;

        //==============================================================================================================
        juce::Slider& getSlider();
        const juce::Slider& getSlider() const;

    protected:
        //==============================================================================================================
        Slider(std::unique_ptr<GuiItem> itemToDecorate, float defaultWidth, float defaultHeight);

        //==============================================================================================================
        void updateStyle();

    private:
        //==============================================================================================================
        void sliderValueChanged(juce::Slider* slider) override;

        //==============================================================================================================
        virtual juce::Slider::SliderStyle getStyleForOrientation(Orientation orientation);

        //==============================================================================================================
        void updateRange();

        //==============================================================================================================
        TypedValue<juce::String> value;
        TypedValue<juce::String> min;
        TypedValue<juce::String> max;
        TypedValue<juce::String> mid;
        TypedValue<juce::String> interval;
        TypedValue<AutoValue<Orientation>> orientation;
        Length width;
        Length height;
        TypedValue<double> sensitivity;
        TypedValue<bool> isInVelocityMode;
        TypedValue<double> velocitySensitivity;
        TypedValue<int> velocityThreshold;
        TypedValue<double> velocityOffset;
        TypedValue<bool> snapToMouse;
        TypedValue<float> explicitWidth;
        TypedValue<float> explicitHeight;

        //==============================================================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Slider)
    };
} // namespace jive
