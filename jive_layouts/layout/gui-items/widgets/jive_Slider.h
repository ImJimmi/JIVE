#pragma once

#include <jive_layouts/layout/gui-items/jive_GuiItemDecorator.h>

namespace jive
{
    class Slider
        : public GuiItemDecorator
        , private juce::Slider::Listener
    {
    public:
        explicit Slider(std::unique_ptr<GuiItem> itemToDecorate);

        bool isContainer() const override;

#if JIVE_IS_PLUGIN_PROJECT
        void attachToParameter(juce::RangedAudioParameter*, juce::UndoManager*) override;
#endif

        juce::Slider& getSlider();
        const juce::Slider& getSlider() const;

    protected:
        Slider(std::unique_ptr<GuiItem> itemToDecorate, float defaultWidth, float defaultHeight);

        void updateStyle();

    private:
        void sliderValueChanged(juce::Slider* slider) override;

        virtual juce::Slider::SliderStyle getStyleForOrientation(Orientation orientation);

        void updateRange();

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
        Property<bool> focusable;

        Event onChange;

#if JIVE_IS_PLUGIN_PROJECT
        std::unique_ptr<juce::SliderParameterAttachment> parameterAttachment;
#endif

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Slider)
    };
} // namespace jive
