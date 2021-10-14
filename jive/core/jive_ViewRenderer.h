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
        std::unique_ptr<GuiItem> renderView(juce::ValueTree tree);

    private:
        //==============================================================================================================
        std::unique_ptr<GuiItem> createGuiItem(juce::ValueTree tree) const;
        void appendChildItems(GuiItem& item, juce::ValueTree tree);

        std::unique_ptr<juce::Component> createComponent(juce::ValueTree tree) const;

        //==============================================================================================================
        juce::HashMap<juce::String, ComponentCreator> componentCreators;

        //==============================================================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ViewRenderer)
    };
} // namespace jive
