#pragma once

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    class Interpreter
    {
    public:
        //==============================================================================================================
        using ComponentFactory = std::function<std::unique_ptr<juce::Component>()>;

        //==============================================================================================================
        Interpreter();

        //==============================================================================================================
        void setFactory(const juce::Identifier& treeType, ComponentFactory factory);
        void resetFactories();

        //==============================================================================================================
        std::unique_ptr<GuiItem> interpret(juce::ValueTree tree) const;

    private:
        //==============================================================================================================
        std::unique_ptr<GuiItem> interpret(juce::ValueTree tree, GuiItem* const parent) const;

        std::unique_ptr<GuiItem> createGuiItem(juce::ValueTree tree, GuiItem* const parent) const;
        void appendChildItems(GuiItem& item, juce::ValueTree tree) const;

        std::unique_ptr<juce::Component> createComponent(juce::ValueTree tree) const;

        //==============================================================================================================
        juce::HashMap<juce::String, ComponentFactory> factories;

        //==============================================================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Interpreter)
    };
} // namespace jive
