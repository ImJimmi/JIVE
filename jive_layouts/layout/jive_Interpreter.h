#pragma once

namespace jive
{
    class Interpreter : private juce::ValueTree::Listener
    {
    public:
        Interpreter() = default;

        const ComponentFactory& getComponentFactory() const;
        ComponentFactory& getComponentFactory();
        void setComponentFactory(const ComponentFactory& newFactory);

        void setAlias(juce::Identifier aliasType, const juce::ValueTree& treeToReplaceWith);

        template <typename Decorator>
        void addDecorator(const juce::Identifier& itemType);

        [[nodiscard]] std::unique_ptr<GuiItem> interpret(const juce::ValueTree& tree,
                                                         juce::AudioProcessor* pluginProcessor = nullptr) const;
        [[nodiscard]] std::unique_ptr<GuiItem> interpret(const juce::XmlElement& xml,
                                                         juce::AudioProcessor* pluginProcessor = nullptr) const;
        [[nodiscard]] std::unique_ptr<GuiItem> interpret(const juce::String& xmlString,
                                                         juce::AudioProcessor* pluginProcessor = nullptr) const;
        [[nodiscard]] std::unique_ptr<GuiItem> interpret(const void* xmlStringData,
                                                         int xmlStringDataSize,
                                                         juce::AudioProcessor* pluginProcessor = nullptr) const;

        void listenTo(GuiItem& item);

    private:
        void valueTreeChildAdded(juce::ValueTree& parentTree,
                                 juce::ValueTree& childWhichHasBeenAdded) final;

        std::unique_ptr<GuiItem> interpret(const juce::ValueTree& tree,
                                           GuiItem* const parent,
                                           juce::AudioProcessor* pluginProcessor) const;

        void expandAlias(juce::ValueTree& tree) const;

        std::unique_ptr<GuiItem> createUndecoratedItem(const juce::ValueTree& tree, GuiItem* const parent) const;
        void insertChild(GuiItem& item, int index, const juce::ValueTree& childState) const;
        void setChildItems(GuiItem& item) const;

        std::unique_ptr<juce::Component> createComponent(const juce::ValueTree& tree) const;

        ComponentFactory componentFactory;
        std::vector<std::pair<juce::Identifier, std::function<std::unique_ptr<GuiItemDecorator>(std::unique_ptr<GuiItem>)>>> customDecorators;
        std::unordered_map<juce::Identifier, juce::ValueTree> aliases;

        juce::WeakReference<GuiItem> observedItem = nullptr;

        JUCE_LEAK_DETECTOR(Interpreter)
    };
} // namespace jive
