#pragma once

#include "jive_StyleSelectors.h"

#include <jive_components/jive_components.h>

namespace jive
{
    template <typename Value>
    using Styles = std::unordered_map<StyleIdentifier, Property<Value, Inheritance::doNotInherit, Accumulation::doNotAccumulate, false>>;

    class StyleSheet
        : public juce::ReferenceCountedObject
        , private juce::ComponentListener
    {
    public:
        using ReferenceCountedPointer = juce::ReferenceCountedObjectPtr<StyleSheet>;

        ~StyleSheet();

        [[nodiscard]] Fill getBackground() const;
        [[nodiscard]] Fill getForeground() const;
        [[nodiscard]] Fill getBorderFill() const;
        [[nodiscard]] BorderRadii<float> getBorderRadii() const;
        [[nodiscard]] juce::Font getFont() const;

        [[nodiscard]] static ReferenceCountedPointer create(juce::Component& component, juce::ValueTree state);

    private:
        StyleSheet(juce::Component& component, juce::ValueTree state);

        void componentParentHierarchyChanged(juce::Component&) final;
        void componentMovedOrResized(juce::Component&, bool, bool) final;

        [[nodiscard]] juce::String getFontFamily() const;
        [[nodiscard]] float getFontSize() const;
        [[nodiscard]] float getFontStretch() const;
        [[nodiscard]] juce::String getFontStyle() const;
        [[nodiscard]] juce::String getFontWeight() const;
        [[nodiscard]] float getLetterSpacing() const;
        [[nodiscard]] juce::String getTextDecoration() const;

        void updateClosestAncestor();
        void updateStyles(jive::Object& state, StyleIdentifier);
        void updateStyles();
        void applyStyles();

        BackgroundCanvas backgroundCanvas;

        juce::Component::SafePointer<juce::Component> component;
        juce::ValueTree state;
        Property<Object::ReferenceCountedPointer,
                 Inheritance::doNotInherit,
                 Accumulation::doNotAccumulate,
                 false>
            style;
        ReferenceCountedPointer closestAncestor;
        StyleSelectors selectors;
        juce::Array<StyleSheet*> dependants;
#if !JIVE_UNIT_TESTS
        const ComponentInteractionState interactionState;
#endif

        Property<float> borderWidth;
        Styles<Fill> backgroundStyles;
        Styles<Fill> foregroundStyles;
        Styles<Fill> borderFillStyles;
        Styles<BorderRadii<float>> borderRadiiStyles;
        Styles<juce::String> fontFamilyStyles;
        Styles<float> fontSizeStyles;
        Styles<float> fontStretchStyles;
        Styles<juce::String> fontStyleStyles;
        Styles<juce::String> fontWeightStyles;
        Styles<float> letterSpacingStyles;
        Styles<juce::String> textDecorationStyles;

        JUCE_LEAK_DETECTOR(StyleSheet)
    };
} // namespace jive
