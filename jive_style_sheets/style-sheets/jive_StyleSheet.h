#pragma once

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    class StyleSheet
        : public juce::ReferenceCountedObject
        , private juce::ComponentListener
        , private juce::ValueTree::Listener
        , private Object::Listener
    {
    public:
        //==============================================================================================================
        struct Selectors;
        using ReferenceCountedPointer = juce::ReferenceCountedObjectPtr<StyleSheet>;

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
        void componentMovedOrResized(juce::Component& componentThatWasMovedOrResized, bool wasMoved, bool wasResized) final;
        void componentParentHierarchyChanged(juce::Component& childComponent) final;
        void valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier&) final;
        void propertyChanged(Object& object, const juce::Identifier& name) final;

        //==============================================================================================================
        juce::var findStyleProperty(const juce::Identifier& propertyName) const;
        juce::var findHierarchicalStyleProperty(const juce::Identifier& propertyName) const;
        juce::ReferenceCountedObjectPtr<StyleSheet> findClosestAncestorStyleSheet();
        juce::Array<ReferenceCountedPointer> collectChildSheets();

        //==============================================================================================================
        void applyStyles();

        //==============================================================================================================
        juce::Component::SafePointer<juce::Component> component;
        juce::ValueTree state;
        juce::ValueTree stateRoot;
        ComponentInteractionState interactionState;

        BackgroundCanvas backgroundCanvas;

        Property<Object::ReferenceCountedPointer> style;
        Property<float> borderWidth;

        const std::unique_ptr<Selectors> selectors;

        mutable bool fontInvalidated{ true };
        mutable juce::Font cachedFont;

        //==============================================================================================================
        JUCE_LEAK_DETECTOR(StyleSheet)
    };
} // namespace jive
