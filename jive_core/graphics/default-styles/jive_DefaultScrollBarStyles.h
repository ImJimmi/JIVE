#pragma once

#include <jive_core/geometry/jive_BorderRadii.h>

#include <juce_gui_basics/juce_gui_basics.h>

namespace jive::default_styles
{
    inline void paintScrollBar(juce::Graphics& g,
                               const juce::ScrollBar& scrollBar,
                               const Styles& styles,
                               int thumbStart,
                               int thumbSize)
    {
        paintComponent(g, scrollBar, styles);

        const auto thumbBounds = scrollBar.isVertical()
                                   ? juce::Rectangle{ 0, thumbStart, scrollBar.getWidth(), thumbSize }
                                   : juce::Rectangle{ thumbStart, 0, thumbSize, scrollBar.getHeight() };

        g.setFillType(getThumbFill(scrollBar, styles));
        default_styles::drawRoundedRectangle(g,
                                             thumbBounds
                                                 .toFloat()
                                                 .reduced(scrollBar.isVertical() ? 1.0f : 0.0f,
                                                          scrollBar.isVertical() ? 0.0f : 1.0f),
                                             getBorderRadius(scrollBar,
                                                             styles,
                                                             BorderRadii{
                                                                 static_cast<float>(juce::jmin(scrollBar.getWidth(),
                                                                                               scrollBar.getHeight()))
                                                                     / 4.0f,
                                                             }),
                                             {},
                                             getThumbFill(scrollBar, styles),
                                             juce::Colours::transparentBlack);
    }

    inline void paintScrollBarButton(juce::Graphics& g,
                                     const juce::ScrollBar& scrollBar,
                                     const Styles& styles,
                                     const juce::Rectangle<float>& bounds,
                                     int direction)
    {
        default_styles::drawRoundedRectangle(g,
                                             bounds,
                                             getBorderRadius(scrollBar, styles),
                                             getBorderWidth(scrollBar, styles),
                                             juce::FillType{ juce::Colours::transparentBlack },
                                             getBorderFill(scrollBar, styles));

        juce::Path path;

        const auto radius = juce::jmax(4.0f, juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f - 2.0f);
        const auto centre = bounds.getCentre();
        path.startNewSubPath(centre.getPointOnCircumference(radius, 0.0f));
        path.lineTo(centre.getPointOnCircumference(radius, 120.0f * juce::MathConstants<float>::pi / 180.0f));
        path.lineTo(centre.getPointOnCircumference(radius, 240.0f * juce::MathConstants<float>::pi / 180.0f));
        path.closeSubPath();
        path.applyTransform(juce::AffineTransform::rotation(juce::MathConstants<float>::halfPi * static_cast<float>(direction),
                                                            centre.getX(),
                                                            centre.getY()));

        g.setFillType(getThumbFill(scrollBar, styles));
        g.fillPath(path);
    }
} // namespace jive::default_styles
