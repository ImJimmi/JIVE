#pragma once

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    class GuiItem
        : protected juce::ValueTree::Listener
        , protected juce::ComponentListener
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
        GuiItem(std::unique_ptr<juce::Component> component, juce::ValueTree tree, GuiItem* parent = nullptr);
        GuiItem(const GuiItem& other);
        ~GuiItem();

        //==============================================================================================================
        virtual void updateLayout();
        virtual void updateSize();
        virtual void updatePosition();
        virtual void informContentChanged();

        //==============================================================================================================
        juce::ValueTree getState();

        const juce::Component& getComponent() const;
        juce::Component& getComponent();
        const juce::Viewport& getViewport() const;
        juce::Viewport& getViewport();

        virtual void addChild(std::unique_ptr<GuiItem> child);
        virtual int getNumChildren() const;
        virtual GuiItem& getChild(int index) const;
        const GuiItem* getParent() const;
        GuiItem* getParent();
        bool isTopLevel() const;

        virtual bool isContainer() const;
        virtual bool isContent() const;

        BoxModel getBoxModel() const;
        Display getDisplay() const;
        bool hasAutoWidth() const;
        bool hasAutoHeight() const;

        //==============================================================================================================
        virtual Iterator begin();
        virtual const Iterator begin() const;
        virtual Iterator end();
        virtual const Iterator end() const;

    protected:
        //==============================================================================================================
        void componentMovedOrResized(juce::Component& component, bool wasMoved, bool wasResized) override;
        void componentVisibilityChanged(juce::Component& component) override;
        void componentNameChanged(juce::Component& component) override;
        void componentEnablementChanged(juce::Component& component) override;

        //==============================================================================================================
        virtual void contentChanged();

        //==============================================================================================================
        juce::ValueTree tree;

        const std::shared_ptr<juce::Component> component;
        const std::shared_ptr<juce::Viewport> viewport;
        GuiItem* const parent;

    private:
        //==============================================================================================================
        friend class GuiItemDecorator;

        //==============================================================================================================
        GuiItem(std::shared_ptr<juce::Component> component,
                std::shared_ptr<juce::Viewport> viewport,
                juce::ValueTree tree,
                GuiItem* parent);

        //==============================================================================================================
        void updateViewportSize();
        void updateComponentSize();

        //==============================================================================================================
        juce::OwnedArray<GuiItem> children;

        BoxModel boxModel;

        TypedValue<juce::String> name;
        TypedValue<juce::String> title;
        TypedValue<juce::Identifier> id;
        TypedValue<juce::String> description;
        TypedValue<juce::String> tooltip;
        TypedValue<bool> enabled;
        TypedValue<bool> visible;
        TypedValue<bool> alwaysOnTop;
        TypedValue<bool> bufferedToImage;
        TypedValue<bool> opaque;
        TypedValue<bool> focusable;
        TypedValue<bool> clickingGrabsFocus;
        TypedValue<bool> focusOutline;
        TypedValue<int> focusOrder;
        TypedValue<float> opacity;
        TypedValue<juce::MouseCursor::StandardCursorType> cursor;
        Length width;
        Length height;
        TypedValue<Display> display;
        TypedValue<Overflow> overflow;

        //==============================================================================================================
        JUCE_LEAK_DETECTOR(GuiItem)
    };
} // namespace jive
