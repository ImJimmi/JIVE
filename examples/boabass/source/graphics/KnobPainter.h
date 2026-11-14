#pragma once

#include <jive_style_sheets/jive_style_sheets.h>

namespace boabass
{
    // Draws a knob as a ring with a lit arc and a capped pointer. The arc
    // starts from zero, so bipolar parameters light up either side of the
    // top. Colours come from the knob's styles:
    //  - background: the unlit ring
    //  - track: the lit arc
    //  - thumb: the cap
    //  - foreground: the pointer
    //  - border: the cap's rim
    inline void paintKnob(juce::Component& component, const jive::Styles& styles, juce::Graphics& g)
    {
        static constexpr auto startAngle = juce::MathConstants<float>::pi * -0.75f;
        static constexpr auto endAngle = juce::MathConstants<float>::pi * 0.75f;

        auto& slider = dynamic_cast<juce::Slider&>(component);
        const auto range = slider.getNormalisableRange();
        const auto toAngle = [&range](double value) {
            return startAngle + (endAngle - startAngle) * static_cast<float>(range.convertTo0to1(value));
        };

        const auto size = static_cast<float>(juce::jmin(slider.getWidth(), slider.getHeight()));
        const auto bounds = slider.getLocalBounds().toFloat().withSizeKeepingCentre(size, size);
        const auto centre = bounds.getCentre();
        const auto ringThickness = juce::jmax(2.5f, size * 0.07f);
        const auto ringRadius = size * 0.5f - ringThickness;
        const auto origin = toAngle(juce::jlimit(range.start, range.end, 0.0));
        const auto value = toAngle(slider.getValue());

        const auto makeArc = [&centre, ringRadius](float from, float to) {
            juce::Path arc;
            arc.addCentredArc(centre.x, centre.y, ringRadius, ringRadius, 0.0f, from, to, true);
            return arc;
        };
        const juce::PathStrokeType ringStroke{ ringThickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded };

        g.setFillType(jive::getBackgroundFill(slider, styles));
        g.strokePath(makeArc(startAngle, endAngle), ringStroke);

        if (!juce::approximatelyEqual(origin, value))
        {
            g.setFillType(jive::getTrackFill(slider, styles));
            g.strokePath(makeArc(juce::jmin(origin, value), juce::jmax(origin, value)), ringStroke);
        }

        const auto capRadius = ringRadius - ringThickness * 1.5f;
        const auto cap = juce::Rectangle{ capRadius * 2.0f, capRadius * 2.0f }.withCentre(centre);
        juce::Path capPath;
        capPath.addEllipse(cap);

        g.setFillType(jive::getThumbFill(slider, styles, cap));
        g.fillPath(capPath);
        g.setFillType(jive::getBorderFill(slider, styles));
        g.strokePath(capPath, juce::PathStrokeType{ 1.0f });

        const auto pointerStart = centre.getPointOnCircumference(capRadius * 0.35f, value);
        const auto pointerEnd = centre.getPointOnCircumference(capRadius * 0.85f, value);
        g.setFillType(jive::getForegroundFill(slider, styles, juce::Colours::white));
        g.drawLine({ pointerStart, pointerEnd }, juce::jmax(1.5f, size * 0.04f));

        slider.getProperties().set(jive::ids::shadowPath(), capPath.toString());
    }
} // namespace boabass
