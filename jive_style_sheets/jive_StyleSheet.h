#pragma once

#include <jive_core/jive_core.h>

namespace jive
{
    class StyleSheet
        : public juce::ReferenceCountedObject
        , private Object::Listener
    {
    public:
        ~StyleSheet();

        using ReferenceCountedPointer = juce::ReferenceCountedObjectPtr<StyleSheet>;

        [[nodiscard]] static ReferenceCountedPointer create(juce::Component&,
                                                            const juce::ValueTree& source);

    private:
        StyleSheet(juce::Component& component,
                   LookAndFeel& lookAndFeel,
                   const juce::ValueTree& source);

        void propertyChanged(Object& object,
                             const juce::Identifier& propertyName) final;

        void clear();
        void stylePropertyChanged();
        void addStylesFrom(Object& object);
        void addInlineStyles(Object& object,
                             const juce::String&);
        void addSelectorBasedStyles(Object& object,
                                    const juce::String& id);
        // void addStylesFrom(Object& object,
        //                    const StyleSelector& selector);

        // void findAndAddStyleProperties(Object& object,
        //                                const juce::String& id,
        //                                std::optional<Styles>& styles,
        //                                const juce::String& selector);
        // void findAndAddStatefulStyles(Object& object,
        //                               const juce::String& selector);
        // void findAndAddOtherStyles(Object& object);

        juce::Component& component;
        LookAndFeel& lookAndFeel;
        juce::ValueTree state;

        using StyleProperty = Property<Object::ReferenceCountedPointer,
                                       Inheritance::doNotInherit,
                                       Accumulation::doNotAccumulate,
                                       false>;

        StyleProperty style;
        Property<juce::BorderSize<float>> border;
        std::unordered_map<juce::String, juce::Uuid> uuids;
        std::unordered_map<juce::String, StyleProperty> observedObjects;
        std::unique_ptr<juce::VBlankAttachment> vBlank;

        std::unique_ptr<FileObserver> fileObserver;
        Property<SourceDirectories::ReferenceCountedPointer> sourceDirectories;
    };
} // namespace jive
