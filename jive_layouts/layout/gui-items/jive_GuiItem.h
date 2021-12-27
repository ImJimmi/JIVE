#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

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
        enum class Display
        {
            flex
        };

        //==============================================================================================================
        GuiItem(std::unique_ptr<juce::Component> component, juce::ValueTree tree, GuiItem* parent = nullptr);
        GuiItem(const GuiItem& other);
        ~GuiItem();

        //==============================================================================================================
        virtual void updateLayout();

        //==============================================================================================================
        const juce::Component& getComponent() const;
        juce::Component& getComponent();

        virtual void addChild(std::unique_ptr<GuiItem> child);
        virtual int getNumChildren() const;
        virtual GuiItem& getChild(int index);
        const GuiItem* getParent() const;
        GuiItem* getParent();

        virtual bool isContainer() const;

        BoxModel getBoxModel() const;

        juce::Identifier getID() const;
        virtual float getWidth() const;
        virtual float getHeight() const;

        Display getDisplay() const;

        bool hasAutoWidth() const;
        bool hasAutoHeight() const;

        //==============================================================================================================
        virtual operator juce::FlexBox();
        virtual operator juce::FlexItem();

    protected:
        //==============================================================================================================
        void valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& id) override;
        void componentMovedOrResized(juce::Component& component, bool wasMoved, bool wasResized) override;

        //==============================================================================================================
        juce::ValueTree tree;

        const std::shared_ptr<juce::Component> component;
        GuiItem* const parent;

    private:
        //==============================================================================================================
        friend class GuiItemDecorator;

        //==============================================================================================================
        GuiItem(juce::ValueTree tree, std::shared_ptr<juce::Component> component, GuiItem* parent);

        //==============================================================================================================
        void updateComponentSize();

        //==============================================================================================================
        juce::OwnedArray<GuiItem> children;

        BoxModel boxModel{ *this, tree };

        TypedValue<juce::Identifier> id;
        juce::CachedValue<Display> display;
        TypedValue<float> width;
        TypedValue<float> height;

        //==============================================================================================================
        JUCE_LEAK_DETECTOR(GuiItem)
    };
} // namespace jive
