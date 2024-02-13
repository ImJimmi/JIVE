#pragma once

namespace jive
{
    class GuiItem
    {
    public:
        GuiItem(std::unique_ptr<juce::Component> component,
                const juce::ValueTree& stateSource,
#if JIVE_GUI_ITEMS_HAVE_STYLE_SHEETS
                StyleSheet::ReferenceCountedPointer styleSheet,
#endif
                GuiItem* parent = nullptr);

        GuiItem(const GuiItem& other);
        virtual ~GuiItem();

        const std::shared_ptr<const juce::Component> getComponent() const;
        const std::shared_ptr<juce::Component> getComponent();

        virtual void insertChild(std::unique_ptr<GuiItem> child, int index);
        virtual void setChildren(std::vector<std::unique_ptr<GuiItem>>&& children);
        virtual void removeChild(GuiItem& childToRemove);
        virtual juce::Array<GuiItem*> getChildren();
        virtual juce::Array<const GuiItem*> getChildren() const;
        virtual const GuiItem* getParent() const;
        virtual GuiItem* getParent();
        bool isTopLevel() const;

        virtual bool isContainer() const;
        virtual bool isContent() const;

        virtual void layOutChildren() {}

        juce::ValueTree state;

    protected:
        virtual void childrenChanged() {}

    private:
        friend class GuiItemDecorator;

        class Remover : private juce::ValueTree::Listener
        {
        public:
            explicit Remover(GuiItem& guiItem);
            ~Remover();

        private:
            void valueTreeChildRemoved(juce::ValueTree&, juce::ValueTree&, int) final;

            GuiItem& item;
            GuiItem* parent;
        };

        GuiItem(std::shared_ptr<juce::Component> component,
                GuiItem* parent,
#if JIVE_GUI_ITEMS_HAVE_STYLE_SHEETS
                StyleSheet::ReferenceCountedPointer sheet,
#endif
                const juce::ValueTree& stateSource);

        void insertChild(std::unique_ptr<GuiItem> child, int index, bool invokeCallback);

#if JIVE_GUI_ITEMS_HAVE_STYLE_SHEETS
        const StyleSheet::ReferenceCountedPointer styleSheet;
#endif

        const std::shared_ptr<juce::Component> component;
        GuiItem* const parent;
        juce::OwnedArray<GuiItem> children;
        std::unique_ptr<Remover> remover;

        JUCE_DECLARE_WEAK_REFERENCEABLE(GuiItem)
        JUCE_LEAK_DETECTOR(GuiItem)
    };

    [[nodiscard]] BoxModel& boxModel(GuiItem& decoratedItem);
    [[nodiscard]] const BoxModel& boxModel(const GuiItem& decoratedItem);
} // namespace jive
