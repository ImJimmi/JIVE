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
        GuiItem(std::unique_ptr<juce::Component> component, juce::ValueTree stateSource, GuiItem* parent = nullptr);
        GuiItem(const GuiItem& other);
        ~GuiItem();

        //==============================================================================================================
        const std::shared_ptr<const juce::Component> getComponent() const;
        const std::shared_ptr<juce::Component> getComponent();

        virtual void addChild(std::unique_ptr<GuiItem> child);
        virtual int getNumChildren() const;
        virtual GuiItem& getChild(int index) const;
        const GuiItem* getParent() const;
        GuiItem* getParent();
        bool isTopLevel() const;

        virtual bool isContainer() const;
        virtual bool isContent() const;
        virtual float calculateAutoWidth() const;
        virtual float calculateAutoHeight() const;

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
        GuiItem(std::shared_ptr<juce::Component> component, GuiItem* parent, juce::ValueTree stateSource);

        //==============================================================================================================
        juce::OwnedArray<GuiItem> children;

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
        TypedValue<Display> display;
        Length width;
        Length height;
        TypedValue<float> componentWidth;
        TypedValue<float> componentHeight;

        //==============================================================================================================
        JUCE_LEAK_DETECTOR(GuiItem)
    };
} // namespace jive
