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
        Property<juce::String> value;
        Property<juce::String> min;
        Property<juce::String> max;
        Property<juce::String> mid;
        Property<juce::String> interval;
        Property<Orientation> orientation;
        Length width;
        Length height;
        Property<double> sensitivity;
        Property<bool> isInVelocityMode;
        Property<double> velocitySensitivity;
        Property<int> velocityThreshold;
        Property<double> velocityOffset;
        Property<bool> snapToMouse;

        Event onChange;

        //==============================================================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Slider)
    };
} // namespace jive
