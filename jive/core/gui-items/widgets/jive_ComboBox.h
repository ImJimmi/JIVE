#pragma once

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    class ComboBox : public GuiItem
    {
    public:
        //==============================================================================================================
        explicit ComboBox(std::unique_ptr<GuiItem> itemToDecorate);

        //==============================================================================================================
        bool isContainer() const override;

        operator juce::FlexBox() override;
        operator juce::FlexItem() override;

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
        const std::unique_ptr<GuiItem> item;

        TypedValue<juce::String> text;
        TypedValue<juce::String> nothingSelectedText;
        TypedValue<juce::String> noOptionsText;
        TypedValue<bool> editable;
        TypedValue<juce::Justification> justification;
    };
} // namespace jive
