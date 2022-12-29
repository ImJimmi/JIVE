#pragma once

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    class Button : public GuiItemDecorator
    {
    public:
        //==============================================================================================================
        enum class TriggerEvent
        {
            mouseUp,
            mouseDown
        };

        //==============================================================================================================
        explicit Button(std::unique_ptr<GuiItem> itemToDecorate);

        //==============================================================================================================
        bool isContainer() const override;

        //==============================================================================================================
        juce::Button& getButton();
        const juce::Button& getButton() const;

    private:
        //==============================================================================================================
        void updateTriggerEvent();

        //==============================================================================================================
        TypedValue<bool> toggleable;
        TypedValue<bool> toggled;
        TypedValue<bool> toggleOnClick;
        TypedValue<int> radioGroup;
        TypedValue<TriggerEvent> triggerEvent;
        TypedValue<juce::String> tooltip;
        TypedValue<juce::FlexBox::Direction> flexDirection;
        TypedValue<juce::FlexBox::JustifyContent> justifyContent;
        TypedValue<juce::BorderSize<float>> padding;
        TypedValue<float> minWidth;
        TypedValue<float> minHeight;

        //==============================================================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Button)
    };
} // namespace jive

//======================================================================================================================
namespace juce
{
    //==================================================================================================================
    template <>
    class VariantConverter<jive::Button::TriggerEvent>
    {
    public:
        //==============================================================================================================
        static jive::Button::TriggerEvent fromVar(const var& v);
        static var toVar(const jive::Button::TriggerEvent& event);

    private:
        //==============================================================================================================
        static const Array<var> options;
    };
} // namespace juce
