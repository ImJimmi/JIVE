#pragma once

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    class ComboBox : public GuiItemDecorator
    {
    public:
        //==============================================================================================================
        explicit ComboBox(std::unique_ptr<GuiItem> itemToDecorate);

        //==============================================================================================================
        bool isContainer() const override;

        //==============================================================================================================
        juce::ComboBox& getComboBox();

    protected:
        //==============================================================================================================
        void valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& id) override;
        void valueTreeChildAdded(juce::ValueTree& parent, juce::ValueTree& child) override;
        void valueTreeChildRemoved(juce::ValueTree& parent, juce::ValueTree& child, int childIndex) override;

    private:
        //==============================================================================================================
        void resetComboBoxOptions();

        //==============================================================================================================
        TypedValue<juce::String> text;
        TypedValue<juce::String> nothingSelectedText;
        TypedValue<juce::String> noOptionsText;
        TypedValue<bool> editable;
        TypedValue<juce::Justification> justification;

        //==============================================================================================================
        JUCE_LEAK_DETECTOR(GuiItemDecorator)
    };
} // namespace jive
