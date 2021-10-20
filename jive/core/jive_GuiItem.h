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
        GuiItem(std::unique_ptr<juce::Component> component, juce::ValueTree tree);

        //==============================================================================================================
        void addChild(std::unique_ptr<GuiItem> child);

        //==============================================================================================================
        juce::Component& getComponent();
        const juce::Component& getComponent() const;

        int getNumChildren() const;
        GuiItem& getChild(int index);

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

        void updateLayout();

        //==============================================================================================================
        const std::unique_ptr<juce::Component> component;
        juce::ValueTree tree;
        juce::OwnedArray<GuiItem> children;

        juce::CachedValue<int> width;
        juce::CachedValue<int> height;
        juce::CachedValue<Display> display;
        juce::CachedValue<juce::FlexBox::Direction> flexDirection;
        juce::CachedValue<juce::FlexBox::Wrap> flexWrap;
        juce::CachedValue<juce::FlexBox::JustifyContent> flexJustifyContent;

        //==============================================================================================================
        JUCE_LEAK_DETECTOR(GuiItem)
    };
} // namespace jive
