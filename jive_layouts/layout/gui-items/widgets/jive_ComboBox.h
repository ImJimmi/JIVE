#pragma once

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    class ComboBox
        : public GuiItemDecorator
        , private juce::ComboBox::Listener
    {
    public:
        //==============================================================================================================
        class Option
        {
        public:
            Option(juce::ValueTree sourceTree, int itemIndex, juce::ComboBox& box);

            void setSelected(bool shouldBeSelected);

        private:
            juce::ValueTree tree;
            juce::ComboBox& comboBox;
            const int index;
            const int id;
            TypedValue<juce::String> text;
            TypedValue<bool> enabled;
            TypedValue<bool> selected;
            bool wasSelected{ false };

            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Option)
        };

        class Header
        {
        public:
            Header(juce::ValueTree sourceTree, ComboBox& comboBox);

        private:
            ComboBox& comboBox;
            TypedValue<juce::String> text;

            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Header)
        };

        //==============================================================================================================
        explicit ComboBox(std::unique_ptr<GuiItem> itemToDecorate);

        //==============================================================================================================
        bool isContainer() const override;

        //==============================================================================================================
        juce::ComboBox& getComboBox();
        const juce::ComboBox& getComboBox() const;

        //==============================================================================================================
        void updateItems();

    protected:
        //==============================================================================================================
        void valueTreeChildAdded(juce::ValueTree& parent, juce::ValueTree& child) override;
        void valueTreeChildRemoved(juce::ValueTree& parent, juce::ValueTree& child, int index) override;

    private:
        //==============================================================================================================
        void comboBoxChanged(juce::ComboBox* box) final;

        //==============================================================================================================
        TypedValue<bool> editable;
        TypedValue<juce::String> tooltip;
        TypedValue<int> selected;

        juce::OwnedArray<Option> options;
        juce::OwnedArray<Header> headers;

        std::function<void()> onComboBoxChanged = nullptr;

        //==============================================================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ComboBox)
    };
} // namespace jive
