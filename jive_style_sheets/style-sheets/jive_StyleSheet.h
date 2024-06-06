#pragma once

#include "jive_StyleCache.h"

#include <jive_components/jive_components.h>

namespace jive
{
    class StyleSheet
        : public juce::ReferenceCountedObject
        , private juce::ComponentListener
        , private juce::ValueTree::Listener
        , private Object::Listener
    {
    public:
        struct Selectors;
        using ReferenceCountedPointer = juce::ReferenceCountedObjectPtr<StyleSheet>;

        StyleSheet(juce::Component& component, juce::ValueTree state);
        ~StyleSheet();

        [[nodiscard]] Fill getBackground() const;
        [[nodiscard]] Fill getForeground() const;
        [[nodiscard]] Fill getBorderFill() const;
        [[nodiscard]] BorderRadii<float> getBorderRadii() const;
        [[nodiscard]] juce::Font getFont() const;

    private:
        void componentMovedOrResized(juce::Component& componentThatWasMovedOrResized, bool wasMoved, bool wasResized) final;
        void componentParentHierarchyChanged(juce::Component& childComponent) final;
        void valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier&) final;
        void propertyChanged(Object& object, const juce::Identifier& name) final;

        [[nodiscard]] juce::var findStyleProperty(const juce::Identifier& propertyName) const;
        [[nodiscard]] juce::var findHierarchicalStyleProperty(const juce::Identifier& propertyName) const;
        [[nodiscard]] StyleSheet* findClosestAncestorStyleSheet();
        [[nodiscard]] juce::Array<StyleSheet*> collectChildSheets();

        void applyStyles();

        juce::Component::SafePointer<juce::Component> component;
        juce::ValueTree state;
        juce::ValueTree stateRoot;
        ComponentInteractionState interactionState;

        BackgroundCanvas backgroundCanvas;

        Property<Object::ReferenceCountedPointer> style;
        Property<float> borderWidth;

        const std::unique_ptr<Selectors> selectors;

        friend class StyleCache;
        StyleCache cache;

        JUCE_LEAK_DETECTOR(StyleSheet)
    };
} // namespace jive
