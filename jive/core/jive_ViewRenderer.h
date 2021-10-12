#pragma once

#include "jive_GuiItem.h"

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
        GuiItem& renderView(juce::ValueTree tree);

    private:
        //==============================================================================================================
        GuiItem& addGuiItem(juce::ValueTree tree);

        std::unique_ptr<juce::Component> createComponent(juce::ValueTree tree) const;
        void applyProperties(juce::ValueTree tree, GuiItem& guiItem) const;
        void createAndAddChildren(juce::ValueTree tree, GuiItem& guiItem);

        //==============================================================================================================
        juce::HashMap<juce::String, ComponentCreator> componentCreators;
        juce::OwnedArray<GuiItem> guiItems;

        //==============================================================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ViewRenderer)
    };
} // namespace jive
