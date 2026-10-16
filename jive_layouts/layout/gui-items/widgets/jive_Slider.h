#pragma once

#include <jive_layouts/layout/gui-items/jive_GuiItemDecorator.h>

namespace jive
{
    class Slider
        : public GuiItemDecorator
        , private juce::Slider::Listener
        , private juce::ValueTree::Listener
    {
    public:
        explicit Slider(std::unique_ptr<GuiItem> itemToDecorate);
        ~Slider() override;

        bool isContainer() const override;

#if JIVE_IS_PLUGIN_PROJECT
        void attachToParameter(juce::RangedAudioParameter*, juce::UndoManager*) override;
#endif

        juce::Slider& getSlider();
        const juce::Slider& getSlider() const;

    protected:
        Slider(std::unique_ptr<GuiItem> itemToDecorate, float defaultWidth, float defaultHeight);

        void updateStyle();
        void valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property) override;

    private:
        void sliderValueChanged(juce::Slider* slider) override;

        virtual juce::Slider::SliderStyle getStyleForOrientation(Orientation orientation);

        void updateRange();

        PassiveProperty<juce::String> value;
        PassiveProperty<juce::String> min;
        PassiveProperty<juce::String> max;
        PassiveProperty<juce::String> mid;
        PassiveProperty<juce::String> interval;
        PassiveProperty<Orientation> orientation;
        PassiveProperty<Length> width;
        PassiveProperty<Length> height;
        PassiveProperty<double> sensitivity;
        PassiveProperty<bool> isInVelocityMode;
        PassiveProperty<double> velocitySensitivity;
        PassiveProperty<int> velocityThreshold;
        PassiveProperty<double> velocityOffset;
        PassiveProperty<bool> snapToMouse;
        PassiveProperty<bool> focusable;

        Event onChange;

#if JIVE_IS_PLUGIN_PROJECT
        std::unique_ptr<juce::SliderParameterAttachment> parameterAttachment;
#endif

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Slider)
    };
} // namespace jive
