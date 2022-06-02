#pragma once

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    class Button
        : public GuiItemDecorator
        , public TextWidget
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
        float getWidth() const override;
        float getHeight() const override;

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

        //==============================================================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Button)
    };
} // namespace jive
