#pragma once

#include <jive_core/jive_core.h>

namespace jive_examples::widgets
{
    // Draws a slider as a row of lights, lit from zero to its value. Only the
    // shape is decided here: the colours are the slider's thumb and background
    // styles, so themes and style sheets still apply.
    inline void paintSegmentedSlider(const juce::Component& component,
                                     const jive::Styles& styles,
                                     juce::Graphics& g)
    {
        static constexpr auto segmentPitch = 10.0f;
        static constexpr auto gap = 3.0f;

        const auto& slider = dynamic_cast<const juce::Slider&>(component);
        const auto range = slider.getNormalisableRange();
        const auto [low, high] = std::minmax({
            range.convertTo0to1(juce::jlimit(range.start, range.end, 0.0)),
            range.convertTo0to1(slider.getValue()),
        });
        const auto bounds = slider
                                .getLookAndFeel()
                                .getSliderLayout(const_cast<juce::Slider&>(slider))
                                .sliderBounds.toFloat();
        const auto length = slider.isHorizontal() ? bounds.getWidth() : bounds.getHeight();
        const auto numSegments = juce::jmax(1, static_cast<int>(length / segmentPitch));
        const auto segmentLength = length / static_cast<float>(numSegments);

        for (auto i = 0; i < numSegments; i++)
        {
            const auto offset = segmentLength * static_cast<float>(i);
            const auto segment = slider.isHorizontal()
                                   ? bounds.withX(bounds.getX() + offset).withWidth(segmentLength).reduced(gap / 2.0f, 0.0f)
                                   : bounds.withY(bounds.getBottom() - offset - segmentLength).withHeight(segmentLength).reduced(0.0f, gap / 2.0f);
            const auto centre = (i + 0.5) / numSegments;

            g.setFillType(low <= centre && centre <= high
                              ? jive::getThumbFill(slider, styles, segment)
                              : jive::getBackgroundFill(slider, styles));
            g.fillRoundedRectangle(segment, 2.0f);
        }
    }
} // namespace jive_examples::widgets
