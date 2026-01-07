#pragma once

#include <jive_layouts/layout/gui-items/jive_GuiItemDecorator.h>
#include <jive_layouts/utilities/jive_ComponentFactory.h>

namespace juce
{
    class AudioProcessor;
}

namespace jive
{
    class TreeEditor;

    class Interpreter : private juce::ValueTree::Listener
    {
    public:
        Interpreter() = default;

        [[nodiscard]] std::unique_ptr<GuiItem> interpret(const juce::ValueTree& tree,
                                                         juce::AudioProcessor* pluginProcessor = nullptr);
        [[nodiscard]] std::unique_ptr<GuiItem> interpret(const juce::XmlElement& xml,
                                                         juce::AudioProcessor* pluginProcessor = nullptr);
        [[nodiscard]] std::unique_ptr<GuiItem> interpret(const juce::String& xmlString,
                                                         juce::AudioProcessor* pluginProcessor = nullptr);
        [[nodiscard]] std::unique_ptr<GuiItem> interpret(const void* xmlStringData,
                                                         int xmlStringDataSize,
                                                         juce::AudioProcessor* pluginProcessor = nullptr);
        [[nodiscard]] std::unique_ptr<GuiItem> interpret(const juce::File& file,
                                                         juce::AudioProcessor* pluginProcessor = nullptr);

        void listenTo(GuiItem& item);
        void addSourceDirectory(const juce::File& directory);
        void removeSourceDirectory(const juce::File& directory);

        [[nodiscard]] const ComponentFactory& getComponentFactory() const;
        [[nodiscard]] ComponentFactory& getComponentFactory();
        void setComponentFactory(const ComponentFactory& newFactory);

        template <typename Decorator>
        void addDecorator(const juce::Identifier& itemType)
        {
            customDecorators.emplace_back(itemType, [](std::unique_ptr<GuiItem> item) {
                return std::make_unique<Decorator>(std::move(item));
            });
        }

    private:
        // ValueTree callbacks
        void valueTreeChildAdded(juce::ValueTree& parentTree,
                                 juce::ValueTree& childWhichHasBeenAdded) final;
        void valueTreeChildRemoved(juce::ValueTree& parentTree,
                                   juce::ValueTree& childWhichHasBeenRemoved,
                                   int indexFromWhichChildWasRemoved) final;
        void valueTreeRedirected(juce::ValueTree& treeWhichHasBeenChanged) final;

        // Item creation
        [[nodiscard]] std::unique_ptr<GuiItem> interpretInternal(juce::ValueTree tree,
                                                                 GuiItem* const parent);
        [[nodiscard]] std::unique_ptr<GuiItem> createUndecoratedItem(const juce::ValueTree& tree,
                                                                     GuiItem* const parent);
        [[nodiscard]] std::shared_ptr<juce::Component> createComponent(const juce::ValueTree& tree, const GuiItem* parent) const;
        void setChildItems(GuiItem& item);
        void setupItemsRecursive(GuiItem& item) const;

        // File sources
        void loadExternalSources(juce::ValueTree tree);
        [[nodiscard]] juce::ValueTree parseFileToValueTree(const juce::File&) const;
        void onObservedFileChanged(const juce::File& file);

        ComponentFactory componentFactory;
        std::vector<std::pair<juce::Identifier, std::function<std::unique_ptr<GuiItemDecorator>(std::unique_ptr<GuiItem>)>>> customDecorators;

        juce::WeakReference<GuiItem> observedItem = nullptr;
        mutable juce::OwnedArray<FileObserver> fileObservers;
        SourceDirectories::ReferenceCountedPointer sourceDirectories = new SourceDirectories{
            juce::File::getCurrentWorkingDirectory(),
        };
        bool ignoreValueTreeChanges = false;

        JUCE_LEAK_DETECTOR(Interpreter)
    };
} // namespace jive
