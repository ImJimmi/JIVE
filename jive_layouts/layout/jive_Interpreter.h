#pragma once

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    class Interpreter
    {
    public:
        //==============================================================================================================
        const ComponentFactory& getComponentFactory() const;
        ComponentFactory& getComponentFactory();
        void setComponentFactory(const ComponentFactory& newFactory);

        //==============================================================================================================
        std::unique_ptr<GuiItem> interpret(juce::ValueTree tree) const;

    private:
        //==============================================================================================================
        std::unique_ptr<GuiItem> interpret(juce::ValueTree tree, GuiItem* const parent) const;

        std::unique_ptr<GuiItem> createUndecoratedItem(juce::ValueTree tree, GuiItem* const parent) const;
        void appendChild(GuiItem& item, juce::ValueTree childState) const;
        void appendChildItems(GuiItem& item) const;

        std::unique_ptr<juce::Component> createComponent(juce::ValueTree tree) const;

        //==============================================================================================================
        ComponentFactory componentFactory;

        //==============================================================================================================
        JUCE_LEAK_DETECTOR(Interpreter)
    };
} // namespace jive
