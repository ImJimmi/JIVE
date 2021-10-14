#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    class GuiItem : private juce::ValueTree::Listener
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

        juce::String getID() const;
        Display getDisplay() const;

    private:
        //==============================================================================================================
        void valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& id) final;

        //==============================================================================================================
        void componentIdChanged();

        //==============================================================================================================
        const std::unique_ptr<juce::Component> component;
        juce::ValueTree tree;
        juce::OwnedArray<GuiItem> children;

        juce::CachedValue<Display> display;

        //==============================================================================================================
        JUCE_LEAK_DETECTOR(GuiItem)
    };
} // namespace jive
