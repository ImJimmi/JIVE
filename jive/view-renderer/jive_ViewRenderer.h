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
        using ComponentCreator = std::function<std::unique_ptr<juce::Component>()>;

        //==============================================================================================================
        ViewRenderer();

        //==============================================================================================================
        void setComponentCreator (const juce::String& customComponentType, ComponentCreator creator);
        void resetComponentCreators();

        //==============================================================================================================
        juce::Component& renderView(juce::ValueTree tree);

    private:
        //==============================================================================================================
        juce::Component& addComponent(juce::ValueTree tree);

        std::unique_ptr<juce::Component> createComponent(juce::ValueTree tree) const;
        void applyProperties(juce::ValueTree tree, juce::Component& component) const;
        void createAndAddChildren(juce::ValueTree tree, juce::Component& component);

        //==============================================================================================================
        juce::HashMap<juce::String, ComponentCreator> componentCreators;
        juce::OwnedArray<juce::Component> components;

        //==============================================================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ViewRenderer)
    };
} // namespace jive
