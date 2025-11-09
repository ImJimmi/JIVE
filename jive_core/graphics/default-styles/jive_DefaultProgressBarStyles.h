#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace jive::default_styles
{
    inline void paintLinearProgressBarTrack(juce::Graphics& g,
                                            const juce::ProgressBar& bar,
                                            juce::Array<double> progressValues,
                                            const Styles& styles)
    {
        progressValues.sort();

        const auto borderWidth = getBorderWidth(bar, styles);
        const auto barBounds = bar.getLocalBounds().toFloat();
        const auto trackBounds = getTrackBounds(bar, styles, progressValues[0], progressValues[1]);

        auto borderRadius = getBorderRadius(bar, styles);
        borderRadius.topLeft -= juce::jmax(borderWidth.getTop(), borderWidth.getLeft());
        borderRadius.topRight -= juce::jmax(borderWidth.getTop(), borderWidth.getRight());
        borderRadius.bottomRight -= juce::jmax(borderWidth.getBottom(), borderWidth.getRight());
        borderRadius.bottomLeft -= juce::jmax(borderWidth.getBottom(), borderWidth.getLeft());

        juce::Graphics::ScopedSaveState sss{ g };
        g.reduceClipRegion(getShape(getBorderRadius(bar, styles), barBounds, borderWidth));
        drawRoundedRectangle(g,
                             trackBounds,
                             borderRadius,
                             juce::BorderSize<float>{ 0.0f },
                             getTrackFill(bar, styles),
                             juce::Colours::transparentBlack);
    }

    inline void paintLinearProgressBar(juce::Graphics& g,
                                       const juce::ProgressBar& bar,
                                       double progress,
                                       const juce::String&,
                                       const Styles& styles)
    {
        paintComponent(g, bar, styles);

        if (bar.isEnabled() && progress >= 0.0f && progress <= 1.0f)
        {
            paintLinearProgressBarTrack(g, bar, { 0.0, progress }, styles);
        }
        else
        {
            const auto spinnerProgress = std::abs(static_cast<double>((juce::Time::getMillisecondCounter() % 2500)) - 1250.0) / 1250.0;
            paintLinearProgressBarTrack(g, bar, { spinnerProgress * 0.7, 0.3 + spinnerProgress * 0.7 }, styles);
        }

        const_cast<juce::ProgressBar*>(&bar)
            ->getProperties()
            .set("jive::shadow-path",
                 getShape(getBorderRadius(bar, styles),
                          bar.getLocalBounds().toFloat())
                     .toString());
    }

    inline void paintCircularProgressBarTrack(juce::Graphics& g,
                                              const juce::ProgressBar& bar,
                                              float trackThickness,
                                              juce::Array<double> progressValues,
                                              const Styles& styles)
    {
        progressValues.sort();

        const auto bounds = bar.getLocalBounds().toFloat().reduced(trackThickness / 2.0f);

        juce::Path trackPath;
        trackPath.addArc(bounds.getX(),
                         bounds.getY(),
                         bounds.getWidth(),
                         bounds.getHeight(),
                         static_cast<float>(juce::MathConstants<double>::twoPi * progressValues[0]),
                         static_cast<float>(juce::MathConstants<double>::twoPi * progressValues[1]),
                         true);

        g.setFillType(getTrackFill(bar, styles));
        g.strokePath(trackPath, juce::PathStrokeType{ trackThickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded });
    }

    inline void paintCircularProgressBar(juce::Graphics& g,
                                         const juce::ProgressBar& bar,
                                         double progress,
                                         const juce::String&,
                                         const Styles& styles)
    {
        static constexpr auto trackThickness = 5.0f;
        const auto bounds = bar.getLocalBounds().toFloat();

        juce::Path backgroundPath;
        backgroundPath.addEllipse(bounds.reduced(trackThickness / 2.0f));
        juce::PathStrokeType{ trackThickness }.createStrokedPath(backgroundPath, backgroundPath);

        g.setFillType(getBackgroundFill(bar, styles));
        g.fillPath(backgroundPath);

        if (bar.isEnabled() && progress >= 0.0f && progress <= 1.0f)
        {
            paintCircularProgressBarTrack(g, bar, trackThickness, { 0.0, progress }, styles);
        }
        else
        {
            const auto spinnerProgress = (juce::Time::getMillisecondCounter() % 2000) / 2000.0;
            paintCircularProgressBarTrack(g, bar, trackThickness, { spinnerProgress, spinnerProgress + 0.2 }, styles);
        }

        const_cast<juce::ProgressBar*>(&bar)
            ->getProperties()
            .set("jive::shadow-path", backgroundPath.toString());
    }

    inline void paintProgressBar(juce::Graphics& g,
                                 const juce::ProgressBar& bar,
                                 double progress,
                                 const juce::String& text,
                                 const Styles& styles)
    {
        if (bar.getResolvedStyle() == juce::ProgressBar::Style::linear)
            paintLinearProgressBar(g, bar, progress, text, styles);
        else
            paintCircularProgressBar(g, bar, progress, text, styles);

        g.setFont(getFont(bar, styles));
        g.setFillType(getForegroundFill(bar, styles));
        paintText(g,
                  bar,
                  bar.getLocalBounds().toFloat(),
                  styles,
                  text,
                  juce::Justification::centred);
    }

} // namespace jive::default_styles
