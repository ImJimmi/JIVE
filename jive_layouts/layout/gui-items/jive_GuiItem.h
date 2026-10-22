#pragma once

#include <jive_style_sheets/jive_style_sheets.h>

#if JIVE_IS_PLUGIN_PROJECT
    #include <juce_audio_processors/juce_audio_processors.h>
#endif

namespace jive
{
    class GuiItem
    {
    public:
        GuiItem(std::unique_ptr<juce::Component> component,
                const juce::ValueTree& sourceState,
                GuiItem* parent = nullptr);
        GuiItem(std::shared_ptr<juce::Component> component,
                const juce::ValueTree& sourceState,
                GuiItem* parent = nullptr);

        GuiItem(const GuiItem& other);
        virtual ~GuiItem();

        [[nodiscard]] const std::shared_ptr<const juce::Component> getComponent() const;
        [[nodiscard]] std::shared_ptr<juce::Component> getComponent();

        virtual void insertChild(std::unique_ptr<GuiItem> child, int index);
        virtual void setChildren(std::vector<std::unique_ptr<GuiItem>>&& children);
        virtual void removeChild(GuiItem& childToRemove);
        [[nodiscard]] virtual juce::Array<const GuiItem*> getChildren() const;
        [[nodiscard]] virtual juce::Array<GuiItem*> getChildren();
        [[nodiscard]] virtual const GuiItem* getParent() const;
        [[nodiscard]] virtual GuiItem* getParent();

        [[nodiscard]] bool isTopLevel() const;
        [[nodiscard]] virtual bool isContainer() const;
        [[nodiscard]] virtual bool isContent() const;

        // Items that don't stretch by default keep their ideal size unless
        // their parent's align-items/justify-items, or their own
        // align-self/justify-self, ask for them to be stretched.
        [[nodiscard]] virtual bool stretchesByDefault() const;

        void callLayoutChildrenWithRecursionLock();
        [[nodiscard]] bool isLayingOutChildren() const;

        void setStyleSheet(StyleSheet::ReferenceCountedPointer sheet)
        {
            styleSheet = sheet;
        }

        // Returns the strings of this item or of its closest ancestor with a
        // "lang" file, or nullptr if there are none.
        [[nodiscard]] const juce::LocalisedStrings* findLocalisedStrings() const;

        // Falls back to JUCE's current mappings when there are no strings.
        [[nodiscard]] juce::String translate(const juce::String& text) const;

#if JIVE_IS_PLUGIN_PROJECT
        virtual void attachToParameter(juce::RangedAudioParameter*, juce::UndoManager* = nullptr);
#endif

        juce::ValueTree state;

    protected:
        virtual void layOutChildren() {}
        virtual void childrenChanged() {}
        virtual void localisedStringsChanged() {}
        [[nodiscard]] virtual const juce::LocalisedStrings* getLocalisedStrings() const;

        std::shared_ptr<juce::Component> component;

    private:
        friend class GuiItemDecorator;

        void insertChild(std::unique_ptr<GuiItem> child, int index, bool invokeCallback);
        void notifyLocalisedStringsChanged();

        StyleSheet::ReferenceCountedPointer styleSheet;
        std::optional<juce::LocalisedStrings> localisedStrings;

        GuiItem* const parent;
        juce::OwnedArray<GuiItem> children;

        bool layoutRecursionLock = false;

        JUCE_DECLARE_WEAK_REFERENCEABLE(GuiItem)
        JUCE_LEAK_DETECTOR(GuiItem)
    };

    [[nodiscard]] BoxModel& boxModel(GuiItem& decoratedItem);
    [[nodiscard]] const BoxModel& boxModel(const GuiItem& decoratedItem);

    [[nodiscard]] GuiItem* findItemWithID(GuiItem& root, const juce::Identifier& id);
} // namespace jive
