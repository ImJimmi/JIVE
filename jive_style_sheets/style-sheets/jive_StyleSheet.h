#pragma once

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    class StyleSheet
        : private juce::ComponentListener
        , private juce::MouseListener
        , private juce::ValueTree::Listener
        , private Object::Listener
    {
    public:
        //==============================================================================================================
        StyleSheet(juce::Component& component, juce::ValueTree state);
        ~StyleSheet();

        //==============================================================================================================
        Fill getBackground() const;
        Fill getForeground() const;
        Fill getBorderFill() const;
        BorderRadii<float> getBorderRadii() const;
        juce::Font getFont() const;

    private:
        //==============================================================================================================
        void componentMovedOrResized(juce::Component& componentThatWasMovedOrResized,
                                     bool wasMoved,
                                     bool wasResized) final;
        void mouseEnter(const juce::MouseEvent&) final;
        void mouseExit(const juce::MouseEvent&) final;
        void mouseDown(const juce::MouseEvent&) final;
        void mouseUp(const juce::MouseEvent&) final;
        void valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier&) final;
        void propertyChanged(Object& object, const juce::Identifier& name) final;

        //==============================================================================================================
        juce::StringArray findApplicableSelectorsInOrderOfSpecificity() const;
        juce::var findStyleProperty(const juce::Identifier& propertyName) const;
        juce::var findHierarchicalStyleProperty(const juce::Identifier& propertyName) const;

        //==============================================================================================================
        void applyStyles();

        //==============================================================================================================
        juce::Component::SafePointer<juce::Component> component;
        juce::ValueTree state;
        juce::ValueTree stateRoot;

        BackgroundCanvas backgroundCanvas;

        Property<Object::ReferenceCountedPointer> style;
        Property<float> borderWidth;

        //==============================================================================================================
        JUCE_LEAK_DETECTOR(StyleSheet)
    };
} // namespace jive
