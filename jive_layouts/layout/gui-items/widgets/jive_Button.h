#pragma once

#include <jive_layouts/layout/gui-items/jive_GuiItemDecorator.h>

namespace jive
{
    class Button
        : public GuiItemDecorator
        , private juce::Button::Listener
        , private juce::ComponentListener
        , private juce::ValueTree::Listener
    {
    public:
        enum class TriggerEvent
        {
            mouseUp,
            mouseDown
        };

        explicit Button(std::unique_ptr<GuiItem> itemToDecorate);
        ~Button() override;

#if JIVE_IS_PLUGIN_PROJECT
        void attachToParameter(juce::RangedAudioParameter*, juce::UndoManager*) override;
#endif

        juce::Button& getButton();
        const juce::Button& getButton() const;

        void layOutChildren() override;

    protected:
        void valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property) override;

    private:
        void buttonClicked(juce::Button* button) final;
        void componentParentHierarchyChanged(juce::Component& component) final;

        // Returns the underlying component as a check-box, or nullptr if this
        // button isn't one.
        [[nodiscard]] const juce::ToggleButton* getCheckbox() const;

        void updateTriggerEvent();
        void updateCheckboxTextInset();

        PassiveProperty<bool> toggleable;
        PassiveProperty<bool> toggled;
        PassiveProperty<bool> toggleOnClick;
        PassiveProperty<int> radioGroup;
        PassiveProperty<TriggerEvent> triggerEvent;
        PassiveProperty<juce::String> tooltip;
        PassiveProperty<juce::String, Accumulation::accumulate> text;
        PassiveProperty<float> minWidth;
        PassiveProperty<float> minHeight;
        PassiveProperty<bool> focusable;
        PassiveProperty<juce::FlexBox::JustifyContent> flexJustifyContent;
        PassiveProperty<juce::FlexBox::AlignItems> flexAlignItems;
        PassiveProperty<juce::BorderSize<float>> padding;

        bool hasAutomaticPadding{ false };

        Event onClick;

#if JIVE_IS_PLUGIN_PROJECT
        std::unique_ptr<juce::ButtonParameterAttachment> parameterAttachment;
#endif

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Button)
    };
} // namespace jive

namespace juce
{
    template <>
    class VariantConverter<jive::Button::TriggerEvent>
    {
    public:
        static jive::Button::TriggerEvent fromVar(const var& v);
        static var toVar(const jive::Button::TriggerEvent& event);

    private:
        static const Array<var> options;
    };
} // namespace juce
