#pragma once

#include <jive_style_sheets/jive_style_sheets.h>
#include <juce_audio_utils/juce_audio_utils.h>

namespace boabass
{
    // JUCE's on-screen keyboard, restyled from the style sheet and sized so
    // its range always fills its width. Its keys use:
    //  - background: white keys
    //  - fill: black keys
    //  - track: keys being played
    //  - accent: the key under the mouse
    //  - border: the lines between white keys
    //  - foreground: the octave labels
    class Keyboard : public juce::MidiKeyboardComponent
    {
    public:
        static constexpr auto lowestNote = 24;
        static constexpr auto highestNote = 72;

        explicit Keyboard(juce::MidiKeyboardState& state)
            : juce::MidiKeyboardComponent{ state, horizontalKeyboard }
        {
            setAvailableRange(lowestNote, highestNote);
            setScrollButtonsVisible(false);
            setOctaveForMiddleC(4);
            setColour(whiteNoteColourId, juce::Colours::transparentBlack);
            setColour(shadowColourId, juce::Colours::transparentBlack);
        }

        void paint(juce::Graphics& g) final
        {
            auto* lookAndFeel = dynamic_cast<jive::LookAndFeel*>(&getLookAndFeel());

            if (lookAndFeel == nullptr)
                return;

            styles = lookAndFeel->findMostApplicableStyles(*this);
            juce::MidiKeyboardComponent::paint(g);
            lookAndFeel->updateShadow<Keyboard>(*this, styles);
        }

        void resized() final
        {
            static const auto numWhiteKeys = [] {
                auto count = 0;

                for (auto note = lowestNote; note <= highestNote; note++)
                    count += juce::MidiMessage::isMidiNoteBlack(note) ? 0 : 1;

                return count;
            }();

            if (getWidth() > 0)
                setKeyWidth(static_cast<float>(getWidth()) / static_cast<float>(numWhiteKeys));

            setBlackNoteLengthProportion(0.6f);
            juce::MidiKeyboardComponent::resized();
        }

    private:
        void drawWhiteNote(int note, juce::Graphics& g, juce::Rectangle<float> area, bool isDown, bool isOver, juce::Colour, juce::Colour) final
        {
            const auto key = area.reduced(0.5f, 0.0f);

            g.setFillType(findFill(isDown ? "track" : "background", key, juce::Colours::white));
            g.fillRoundedRectangle(key.withTrimmedTop(-4.0f), 4.0f);

            if (isOver && !isDown)
            {
                g.setFillType(findFill("accent", key, juce::Colours::transparentBlack));
                g.fillRoundedRectangle(key.withTrimmedTop(-4.0f), 4.0f);
            }

            g.setFillType(findFill("border", key, juce::Colours::transparentBlack));
            g.fillRect(area.withX(area.getRight() - 0.5f).withWidth(1.0f));

            if (note % 12 == 0)
            {
                g.setFillType(findFill("foreground", key, juce::Colours::black));
                g.setFont(juce::FontOptions{ juce::jmin(11.0f, area.getWidth() * 0.5f) });
                g.drawText(getWhiteNoteText(note), key.withTrimmedBottom(4.0f), juce::Justification::centredBottom);
            }
        }

        void drawBlackNote(int, juce::Graphics& g, juce::Rectangle<float> area, bool isDown, bool isOver, juce::Colour) final
        {
            const auto key = area.reduced(1.0f, 0.0f);

            g.setFillType(findFill(isDown ? "track" : "fill", key, juce::Colours::black));
            g.fillRoundedRectangle(key.withTrimmedTop(-3.0f), 3.0f);

            if (isOver && !isDown)
            {
                g.setFillType(findFill("accent", key, juce::Colours::transparentBlack));
                g.fillRoundedRectangle(key.withTrimmedTop(-3.0f), 3.0f);
            }
        }

        [[nodiscard]] juce::FillType findFill(const juce::String& style, juce::Rectangle<float> area, juce::Colour fallback) const
        {
            return styles.find<jive::Fill>(style).value_or(fallback).toJuceFillType(area);
        }

        jive::Styles styles;
    };
} // namespace boabass
