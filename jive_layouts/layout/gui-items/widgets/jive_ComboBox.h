#pragma once

#include <jive_core/components/widgets/jive_ComboBoxComponent.h>
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
            Option(juce::ValueTree sourceTree, int itemIndex, juce::ComboBox& box, const GuiItem& ownerItem);

            void setSelected(bool shouldBeSelected);
            void updateText();
            bool handlePropertyChange(juce::ValueTree& changedTree, juce::Identifier property);

        private:
            void updateStandIn();

            juce::ValueTree tree;
            juce::ComboBox& comboBox;
            const GuiItem& owner;
            const int index;
            const int id;
            std::unique_ptr<ComboBoxComponent::Item> standIn;
            PassiveProperty<juce::String> text;
            PassiveProperty<bool> enabled;
            PassiveProperty<bool> selected;
            PassiveProperty<juce::String> componentID;
            PassiveProperty<juce::String> classes;
            bool wasSelected{ false };

            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Option)
        };

        explicit ComboBox(std::unique_ptr<GuiItem> itemToDecorate);
        ~ComboBox() override;

        bool isContainer() const override;

#if JIVE_IS_PLUGIN_PROJECT
        void attachToParameter(juce::RangedAudioParameter*, juce::UndoManager*) override;
#endif

        juce::ComboBox& getComboBox();
        const juce::ComboBox& getComboBox() const;

        void updateItems();

    protected:
        void localisedStringsChanged() override;
        void valueTreeChildAdded(juce::ValueTree& parent, juce::ValueTree& child) override;
        void valueTreeChildRemoved(juce::ValueTree& parent, juce::ValueTree& child, int index) override;

    private:
        void comboBoxChanged(juce::ComboBox* box) final;
        void valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property) final;

        void updateMinimumSize();
        void translateHeaders();

        PassiveProperty<bool> editable;
        PassiveProperty<juce::String> tooltip;
        PassiveProperty<int> selected;
        PassiveProperty<float> minWidth;
        PassiveProperty<float> minHeight;
        PassiveProperty<bool> focusable;

        Event onChange;

        juce::OwnedArray<Option> options;
        bool updatingItems = false;
        const bool hasAutomaticMinWidth;
        const bool hasAutomaticMinHeight;

#if JIVE_IS_PLUGIN_PROJECT
        std::unique_ptr<juce::ComboBoxParameterAttachment> parameterAttachment;
#endif

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ComboBox)
    };
} // namespace jive
