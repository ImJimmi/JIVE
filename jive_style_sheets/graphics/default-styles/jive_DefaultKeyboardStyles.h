#pragma once

#include "jive_DefaultComponentStyles.h"

#include <jive_style_sheets/graphics/jive_Fill.h>
#include <jive_style_sheets/graphics/jive_FontUtilities.h>

#if JUCE_MODULE_AVAILABLE_juce_audio_utils
namespace jive::default_styles
{
    // Keys without styles look like JUCE's own.
    [[nodiscard]] inline juce::Colour getDefaultKeyColour(const KeyboardComponent::Key& key, bool isDown, bool isOver)
    {
        const auto& lookAndFeel = key.getLookAndFeel();
        const auto base = lookAndFeel.findColour(key.isBlack()
                                                     ? juce::MidiKeyboardComponent::blackNoteColourId
                                                     : juce::MidiKeyboardComponent::whiteNoteColourId);

        if (isDown)
            return base.overlaidWith(lookAndFeel.findColour(juce::MidiKeyboardComponent::keyDownOverlayColourId));
        if (isOver)
            return base.overlaidWith(lookAndFeel.findColour(juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId));

        return base;
    }

    inline void paintKeyboardKey(juce::Graphics& g,
                                 const KeyboardComponent::Key& key,
                                 juce::Rectangle<float> area,
                                 const juce::String& label,
                                 bool isDown,
                                 bool isOver,
                                 const Styles& styles)
    {
        const auto& lookAndFeel = key.getLookAndFeel();
        const auto defaultBorderWidth = key.isBlack()
                                          ? juce::BorderSize<float>{}
                                          : juce::BorderSize<float>{ 0.0f, 0.0f, 0.0f, 1.0f };

        const auto background = findBackgroundFill(key, styles);
        const auto keyColour = getDefaultKeyColour(key, isDown, isOver);

        drawRoundedRectangle(g,
                             area,
                             getBorderRadius(key, styles),
                             getBorderWidth(key, styles, defaultBorderWidth),
                             background.value_or(keyColour).toJuceFillType(area),
                             findBorderFill(key, styles)
                                 .value_or(lookAndFeel.findColour(juce::MidiKeyboardComponent::keySeparatorLineColourId))
                                 .toJuceFillType(area));

        // Like JUCE, an unstyled black key has a lighter top, leaving a dark
        // lip along its sides and bottom.
        if (key.isBlack() && !background.has_value() && !isDown)
        {
            g.setColour(keyColour.brighter());
            g.fillRect(area.reduced(area.getWidth() / 8.0f, 0.0f).removeFromTop(area.getHeight() * 7.0f / 8.0f));
        }

        if (label.isEmpty())
            return;

        const auto font = getFont(key, styles);
        g.setFont(font
                      .withHeight(juce::jmin(font.getHeight(), 12.0f, area.getWidth() * 0.9f))
                      .withHorizontalScale(0.8f));
        g.setFillType(findForegroundFill(key, styles)
                          .value_or(lookAndFeel.findColour(juce::MidiKeyboardComponent::textLabelColourId))
                          .toJuceFillType(area));
        g.drawText(label, area.withTrimmedLeft(1.0f).withTrimmedBottom(2.0f), juce::Justification::centredBottom);
    }
} // namespace jive::default_styles
#endif
