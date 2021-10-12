#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    class ViewRenderer
    {
    public:
        //==============================================================================================================
        ViewRenderer() = default;

        //==============================================================================================================
        juce::Component* createView(juce::ValueTree tree);

    private:
        //==============================================================================================================
        juce::Component* createAndAddComponent(juce::ValueTree tree);

        void applyProperties(juce::ValueTree tree, juce::Component& component);
        void createAndAddChildren(juce::ValueTree tree, juce::Component& component);

        //==============================================================================================================
        juce::OwnedArray<juce::Component> components;

        //==============================================================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ViewRenderer)
    };
} // namespace jive
