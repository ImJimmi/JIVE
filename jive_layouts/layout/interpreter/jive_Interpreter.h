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
        [[nodiscard]] std::unique_ptr<GuiItem> interpret(const SourceFile& file,
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

        /** Called after a file referenced by a "source" property has been read
            into the tree, be that for the first time or because the file
            changed on disk.

            The trees an external source is loaded into are replaced wholesale,
            so any listeners attached to them need to be reassigned when this
            is called. The state tree is up to date by the time this is called.
        */
        std::function<void(const SourceFile&)> onExternalSourceLoaded = nullptr;

    private:
        // ValueTree callbacks
        void valueTreePropertyChanged(juce::ValueTree& tree,
                                      const juce::Identifier& property) final;
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
        [[nodiscard]] juce::ValueTree loadExternalSources(juce::ValueTree tree);
        [[nodiscard]] juce::ValueTree parseFileToValueTree(const SourceFile&) const;
        void onObservedFileChanged(const SourceFile& file);
        void observeFileForChanges(const SourceFile& file);
        void notifyExternalSourcesLoaded();

        ComponentFactory componentFactory;
        std::vector<std::pair<juce::Identifier, std::function<std::unique_ptr<GuiItemDecorator>(std::unique_ptr<GuiItem>)>>> customDecorators;

        juce::WeakReference<GuiItem> observedItem = nullptr;
        mutable juce::OwnedArray<FileObserver> fileObservers;
        // An empty SourceFile is the root that relative paths are found in.
        SourceDirectories::ReferenceCountedPointer sourceDirectories = new SourceDirectories{
            SourceFile{},
            juce::File::getCurrentWorkingDirectory(),
        };
        bool ignoreValueTreeChanges = false;
        std::vector<SourceFile> sourcesPendingNotification;
        int interpretationsInProgress = 0;

        JUCE_LEAK_DETECTOR(Interpreter)
    };
} // namespace jive
