#pragma once

#include <jive_style_sheets/graphics/jive_Styles.h>

#include <juce_gui_basics/juce_gui_basics.h>

#if JUCE_MODULE_AVAILABLE_juce_audio_utils
    #include <juce_audio_utils/juce_audio_utils.h>

namespace jive
{
    class LookAndFeel;

    struct KeyboardStateHolder
    {
        juce::MidiKeyboardState ownedState;
    };

    // An on-screen keyboard that fits its range of notes to its width. Its
    // keys are styled through stand-ins of type "WhiteKey" and "BlackKey".
    class KeyboardComponent
        : private KeyboardStateHolder
        , public juce::MidiKeyboardComponent
    {
    public:
        // JUCE draws a keyboard's keys itself, rather than as components.
        // These stand-ins cover them so they can be styled like any other
        // component, and are hovered and pressed on their keys' behalf.
        class Key : public juce::Component
        {
        public:
            explicit Key(int note);

            [[nodiscard]] int getNote() const noexcept;
            [[nodiscard]] bool isBlack() const noexcept;

            void setInteraction(bool isOver, bool isDown);

        private:
            const int note;
        };

        KeyboardComponent();
        explicit KeyboardComponent(juce::MidiKeyboardState& state);

        [[nodiscard]] juce::MidiKeyboardState& getState() noexcept;

        void setNoteRange(int lowestNote, int highestNote);

        // Returns the stand-in for the given note's key, if it's shown.
        [[nodiscard]] Key* findKey(int note) const;

        void paint(juce::Graphics& g) override;
        void resized() override;

    private:
        void drawWhiteNote(int note, juce::Graphics& g, juce::Rectangle<float> area, bool isDown, bool isOver, juce::Colour, juce::Colour) override;
        void drawBlackNote(int note, juce::Graphics& g, juce::Rectangle<float> area, bool isDown, bool isOver, juce::Colour) override;

        void initialise();
        void fitKeysToWidth();
        void createKeys();
        void positionKeys();
        void paintKey(juce::Graphics& g, int note, juce::Rectangle<float> area, bool isDown, bool isOver);
        void paintShadowIfUnstyled(juce::Graphics& g, const LookAndFeel& jiveLookAndFeel) const;

        juce::MidiKeyboardState& state;
        juce::OwnedArray<Key> keys;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(KeyboardComponent)
    };
} // namespace jive
#endif
