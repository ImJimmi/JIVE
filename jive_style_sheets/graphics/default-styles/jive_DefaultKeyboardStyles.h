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

        drawRoundedRectangle(g,
                             area,
                             getBorderRadius(key, styles),
                             getBorderWidth(key, styles, defaultBorderWidth),
                             findBackgroundFill(key, styles)
                                 .value_or(getDefaultKeyColour(key, isDown, isOver))
                                 .toJuceFillType(area),
                             findBorderFill(key, styles)
                                 .value_or(lookAndFeel.findColour(juce::MidiKeyboardComponent::keySeparatorLineColourId))
                                 .toJuceFillType(area));

        if (label.isEmpty())
            return;

        const auto font = getFont(key, styles);
        g.setFont(font.withHeight(juce::jmin(font.getHeight(), area.getWidth() * 0.5f)));
        g.setFillType(findForegroundFill(key, styles)
                          .value_or(lookAndFeel.findColour(juce::MidiKeyboardComponent::textLabelColourId))
                          .toJuceFillType(area));
        g.drawText(label, area.withTrimmedBottom(4.0f), juce::Justification::centredBottom);
    }
} // namespace jive::default_styles
#endif
