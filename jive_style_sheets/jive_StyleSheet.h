#pragma once

#include <jive_core/jive_core.h>
#include <jive_style_sheets/graphics/jive_LookAndFeel.h>

namespace jive
{
    class StyleSheet
        : public juce::ReferenceCountedObject
        , private Object::Listener
        , private juce::ValueTree::Listener
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
        void valueTreePropertyChanged(juce::ValueTree& tree,
                                      const juce::Identifier& property) final;

        void clear();
        void stylePropertyChanged();
        void watch(const juce::Array<SourceFile>& files);
        void loadFile(const SourceFile& file);
        void updateLineSpacing();
        void addStylesFrom(Object& object);
        void addTokensFrom(Object& object);
        [[nodiscard]] SourceFile findRuleFile(const Object& object, const juce::String& propertyName) const;
        void appendStyleProperties(Object& object, std::optional<Styles>& styles) const;
        void addInlineStyles(Object& object);
        void addSelectorBasedStyles(Object& object);

        juce::Component& component;
        LookAndFeel& lookAndFeel;
        juce::ValueTree state;

        struct RuleFiles : juce::ReferenceCountedObject
        {
            explicit RuleFiles(std::map<std::pair<const Object*, juce::String>, SourceFile> filesToUse)
                : files{ std::move(filesToUse) }
            {
            }

            const std::map<std::pair<const Object*, juce::String>, SourceFile> files;
        };

        using StyleProperty = Property<Object::ReferenceCountedPointer,
                                       Inheritance::doNotInherit,
                                       Accumulation::doNotAccumulate,
                                       false>;

        StyleProperty style;
        PassiveProperty<juce::BorderSize<float>> border;
        PassiveProperty<LineSpacing> lineSpacing;
        std::unordered_map<juce::String, juce::Uuid> uuids;
        std::unordered_map<juce::String, StyleProperty> observedObjects;

        std::vector<std::unique_ptr<FileObserver>> fileObservers;
        Object::ReferenceCountedPointer parsedFile;
        SourceFile styleFile;
        juce::ReferenceCountedObjectPtr<RuleFiles> ruleFiles;
        Property<SourceDirectories::ReferenceCountedPointer> sourceDirectories;

        std::unordered_map<juce::String, juce::var> tokens;
    };
} // namespace jive
