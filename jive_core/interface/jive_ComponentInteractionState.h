#pragma once

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    class ComponentInteractionState
        : private juce::MouseListener
        , private juce::FocusChangeListener
    {
    public:
        //==============================================================================================================
        enum class Mouse
        {
            dissociate,
            hover,
            active,
        };

        enum class Keyboard
        {
            dissociate,
            focus,
        };

        //==============================================================================================================
        ComponentInteractionState(const juce::Component&, juce::ValueTree);
        ~ComponentInteractionState();

    private:
        //==============================================================================================================
        void mouseEnter(const juce::MouseEvent& event) final;
        void mouseExit(const juce::MouseEvent& event) final;
        void mouseDown(const juce::MouseEvent& event) final;
        void mouseUp(const juce::MouseEvent& event) final;
        void globalFocusChanged(juce::Component* focusedComponent) final;

        //==============================================================================================================
        Mouse getCurrentMouseState() const;
        Keyboard getCurrentKeyboardState() const;

        //==============================================================================================================
        const juce::Component& component;
        Property<Mouse> mouse;
        Property<Keyboard> keyboard;
    };
} // namespace jive

//======================================================================================================================
namespace juce
{
    //==================================================================================================================
    template <>
    class VariantConverter<jive::ComponentInteractionState::Mouse>
    {
    public:
        //==============================================================================================================
        static var toVar(const jive::ComponentInteractionState::Mouse& mouse);
        static jive::ComponentInteractionState::Mouse fromVar(const var&);

    private:
        //==============================================================================================================
        static const std::unordered_map<jive::ComponentInteractionState::Mouse, juce::String> map;
    };
    //==================================================================================================================
    template <>
    class VariantConverter<jive::ComponentInteractionState::Keyboard>
    {
    public:
        //==============================================================================================================
        static var toVar(const jive::ComponentInteractionState::Keyboard& keyboard);
        static jive::ComponentInteractionState::Keyboard fromVar(const var&);

    private:
        //==============================================================================================================
        static const std::unordered_map<jive::ComponentInteractionState::Keyboard, juce::String> map;
    };
} // namespace juce