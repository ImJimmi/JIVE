#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    class GuiItem
        : private juce::ValueTree::Listener
        , private juce::ComponentListener
    {
    public:
        //==============================================================================================================
        enum class Display
        {
            flex,
            grid
        };

        //==============================================================================================================
        GuiItem(std::unique_ptr<juce::Component> component, juce::ValueTree tree, GuiItem* parent = nullptr);

        //==============================================================================================================
        void addChild(std::unique_ptr<GuiItem> child);

        //==============================================================================================================
        juce::Component& getComponent();
        const juce::Component& getComponent() const;

        int getNumChildren() const;
        GuiItem& getChild(int index);
        const GuiItem* getParent();

        Display getDisplay() const;

        //==============================================================================================================
        operator juce::FlexBox();
        operator juce::FlexItem();

    private:
        //==============================================================================================================
        void valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& id) final;

        void componentMovedOrResized(juce::Component& component, bool wasMoved, bool wasResized) final;

        //==============================================================================================================
        void componentIdChanged();
        void flexJustifyContentChanged();
        void flexAlignContentChanged();
        void flexItemOrderChanged();
        void flexItemGrowChanged();

        void updateLayout();

        //==============================================================================================================
        const std::unique_ptr<juce::Component> component;
        juce::ValueTree tree;
        juce::OwnedArray<GuiItem> children;
        GuiItem* const parent;

        juce::CachedValue<int> width;
        juce::CachedValue<int> height;
        juce::CachedValue<Display> display;
        juce::CachedValue<juce::FlexBox::Direction> flexDirection;
        juce::CachedValue<juce::FlexBox::Wrap> flexWrap;
        juce::CachedValue<juce::FlexBox::JustifyContent> flexJustifyContent;
        juce::CachedValue<juce::FlexBox::AlignContent> flexAlignContent;
        juce::CachedValue<int> flexItemOrder;
        juce::CachedValue<float> flexItemGrow;

        //==============================================================================================================
        JUCE_LEAK_DETECTOR(GuiItem)
    };
} // namespace jive
