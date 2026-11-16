#pragma once

#include <jive_layouts/layout/gui-items/jive_GuiItemDecorator.h>

namespace jive
{
    class TextBox
        : public GuiItemDecorator
        , private juce::TextEditor::Listener
        , private juce::ValueTree::Listener
    {
    public:
        explicit TextBox(std::unique_ptr<GuiItem> itemToDecorate);
        ~TextBox() override;

        bool isContainer() const override;

        juce::TextEditor& getTextEditor();
        const juce::TextEditor& getTextEditor() const;

    protected:
        void localisedStringsChanged() override;

    private:
        void textEditorTextChanged(juce::TextEditor& editor) final;
        void valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property) final;

        void updatePlaceholder();

        PassiveProperty<juce::String> text;
        PassiveProperty<juce::String> placeholder;
        PassiveProperty<bool> multiLine;
        PassiveProperty<bool> readOnly;
        PassiveProperty<juce::String> tooltip;
        PassiveProperty<float> minWidth;
        PassiveProperty<float> minHeight;
        PassiveProperty<bool> focusable;

        Event onChange;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TextBox)
    };
} // namespace jive
