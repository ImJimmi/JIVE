#pragma once

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    class GuiItem
        : protected juce::ValueTree::Listener
        , protected juce::ComponentListener
        , protected BoxModel::Listener
    {
    public:
        //==============================================================================================================
        struct Iterator
        {
            using difference_type = std::ptrdiff_t;
            using value_type = GuiItem*;
            using pointer = GuiItem**;
            using iterator_category = std::forward_iterator_tag;

            Iterator(const GuiItem& item, bool isEnd);

            Iterator& operator++();
            bool operator==(const Iterator& other) const;
            bool operator!=(const Iterator& other) const;
            GuiItem& operator*();
            const GuiItem& operator*() const;

        private:
            GuiItem* const* item;
        };

        //==============================================================================================================
        GuiItem(std::unique_ptr<juce::Component> component,
                juce::ValueTree stateSource,
#if JIVE_GUI_ITEMS_HAVE_STYLE_SHEETS
                StyleSheet::ReferenceCountedPointer styleSheet,
#endif
                GuiItem* parent = nullptr);

        GuiItem(const GuiItem& other);
        ~GuiItem();

        //==============================================================================================================
        const std::shared_ptr<const juce::Component> getComponent() const;
        const std::shared_ptr<juce::Component> getComponent();

        virtual void addChild(std::unique_ptr<GuiItem> child);
        virtual int getNumChildren() const;
        virtual GuiItem& getChild(int index) const;
        virtual const GuiItem* getParent() const;
        virtual GuiItem* getParent();
        bool isTopLevel() const;

        virtual bool isContainer() const;
        virtual bool isContent() const;

        //==============================================================================================================
        virtual void layOutChildren() {}

        //==============================================================================================================
        virtual Iterator begin();
        virtual const Iterator begin() const;
        virtual Iterator end();
        virtual const Iterator end() const;

        //==============================================================================================================
        juce::ValueTree state;
        BoxModel boxModel;

    protected:
        //==============================================================================================================
        void valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property) override;
        void componentMovedOrResized(juce::Component& component, bool wasMoved, bool wasResized) override;
        void componentVisibilityChanged(juce::Component& component) override;
        void componentNameChanged(juce::Component& component) override;
        void componentEnablementChanged(juce::Component& component) override;
        void componentChildrenChanged(juce::Component& component) override;
        void boxModelChanged(BoxModel& boxModel) override;

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
                juce::ValueTree stateSource);

        //==============================================================================================================
        juce::OwnedArray<GuiItem> children;

#if JIVE_GUI_ITEMS_HAVE_STYLE_SHEETS
        const StyleSheet::ReferenceCountedPointer styleSheet;
#endif

        Property<juce::String> name;
        Property<juce::String> title;
        Property<juce::Identifier> id;
        Property<juce::String> description;
        Property<juce::String> tooltip;
        Property<bool> enabled;
        Property<bool> visible;
        Property<bool> alwaysOnTop;
        Property<bool> bufferedToImage;
        Property<bool> opaque;
        Property<bool> focusable;
        Property<bool> clickingGrabsFocus;
        Property<bool> focusOutline;
        Property<int> focusOrder;
        Property<float> opacity;
        Property<juce::MouseCursor::StandardCursorType> cursor;
        Property<Display> display;
        Length width;
        Length height;
        Property<float> componentWidth;
        Property<float> componentHeight;

        //==============================================================================================================
        JUCE_LEAK_DETECTOR(GuiItem)
    };
} // namespace jive
