#pragma once

#include <jive_layouts/layout/gui-items/jive_GuiItemDecorator.h>

#if JUCE_MODULE_AVAILABLE_juce_audio_utils
namespace jive
{
    class Keyboard
        : public GuiItemDecorator
        , private juce::ValueTree::Listener
    {
    public:
        explicit Keyboard(std::unique_ptr<GuiItem> itemToDecorate);
        ~Keyboard() override;

        bool isContainer() const override;

        KeyboardComponent& getKeyboard();
        const KeyboardComponent& getKeyboard() const;

    private:
        void valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property) final;
        void updateNoteRange();

        PassiveProperty<juce::String> startNote;
        PassiveProperty<juce::String> endNote;
        PassiveProperty<float> minWidth;
        PassiveProperty<float> minHeight;
        PassiveProperty<bool> focusable;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Keyboard)
    };
} // namespace jive
#endif
