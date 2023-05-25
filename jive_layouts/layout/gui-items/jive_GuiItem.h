#pragma once

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    class GuiItem
    {
    public:
        //==============================================================================================================
        GuiItem(std::unique_ptr<juce::Component> component,
                const juce::ValueTree& stateSource,
#if JIVE_GUI_ITEMS_HAVE_STYLE_SHEETS
                StyleSheet::ReferenceCountedPointer styleSheet,
#endif
                GuiItem* parent = nullptr);

        GuiItem(const GuiItem& other);
        virtual ~GuiItem() = default;

        //==============================================================================================================
        const std::shared_ptr<const juce::Component> getComponent() const;
        const std::shared_ptr<juce::Component> getComponent();

        virtual void addChild(std::unique_ptr<GuiItem> child);
        virtual juce::Array<GuiItem*> getChildren();
        virtual juce::Array<const GuiItem*> getChildren() const;
        virtual const GuiItem* getParent() const;
        virtual GuiItem* getParent();
        bool isTopLevel() const;

        virtual bool isContainer() const;
        virtual bool isContent() const;

        //==============================================================================================================
        virtual void layOutChildren() {}

        //==============================================================================================================
        juce::ValueTree state;

    protected:
        //==============================================================================================================
        const std::shared_ptr<juce::Component> component;
        GuiItem* const parent;

    private:
        //==============================================================================================================
        friend class GuiItemDecorator;

        //==============================================================================================================
        GuiItem(std::shared_ptr<juce::Component> component,
                GuiItem* parent,
#if JIVE_GUI_ITEMS_HAVE_STYLE_SHEETS
                StyleSheet::ReferenceCountedPointer sheet,
#endif
                const juce::ValueTree& stateSource);

        //==============================================================================================================
        juce::OwnedArray<GuiItem> children;

#if JIVE_GUI_ITEMS_HAVE_STYLE_SHEETS
        const StyleSheet::ReferenceCountedPointer styleSheet;
#endif

        //==============================================================================================================
        JUCE_LEAK_DETECTOR(GuiItem)
    };
} // namespace jive
