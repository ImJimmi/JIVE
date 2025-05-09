#pragma once

#include <jive_layouts/layout/gui-items/jive_GuiItemDecorator.h>

namespace jive
{
    class ComboBox
        : public GuiItemDecorator
        , private juce::ComboBox::Listener
        , private juce::ValueTree::Listener
    {
    public:
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
            Property<juce::String> text;
            Property<bool> enabled;
            Property<bool> selected;
            bool wasSelected{ false };

            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Option)
        };

        class Header
        {
        public:
            Header(juce::ValueTree sourceTree, ComboBox& comboBox);

        private:
            ComboBox& comboBox;
            Property<juce::String> text;

            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Header)
        };

        explicit ComboBox(std::unique_ptr<GuiItem> itemToDecorate);

        bool isContainer() const override;

#if JIVE_IS_PLUGIN_PROJECT
        void attachToParameter(juce::RangedAudioParameter*, juce::UndoManager*) override;
#endif

        juce::ComboBox& getComboBox();
        const juce::ComboBox& getComboBox() const;

        void updateItems();

    protected:
        void valueTreeChildAdded(juce::ValueTree& parent, juce::ValueTree& child) override;
        void valueTreeChildRemoved(juce::ValueTree& parent, juce::ValueTree& child, int index) override;

    private:
        void comboBoxChanged(juce::ComboBox* box) final;

        Property<bool> editable;
        Property<juce::String> tooltip;
        Property<int> selected;
        Length width;
        Length height;
        Property<bool> focusable;

        Event onChange;

        juce::OwnedArray<Option> options;
        juce::OwnedArray<Header> headers;

#if JIVE_IS_PLUGIN_PROJECT
        std::unique_ptr<juce::ComboBoxParameterAttachment> parameterAttachment;
#endif

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ComboBox)
    };
} // namespace jive
