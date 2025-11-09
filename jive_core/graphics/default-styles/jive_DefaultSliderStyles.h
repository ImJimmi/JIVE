#pragma once

namespace jive::default_styles
{
    inline void paintBarSlider(juce::Graphics& g,
                               const juce::Slider& slider,
                               const Styles& styles)
    {
        const auto bounds = slider
                                .getLookAndFeel()
                                .getSliderLayout(*const_cast<juce::Slider*>(&slider))
                                .sliderBounds.toFloat();
        drawRoundedRectangle(g,
                             bounds,
                             getBorderRadius(slider, styles),
                             getBorderWidth(slider, styles),
                             getBackgroundFill(slider, styles),
                             juce::Colours::transparentBlack);

        {
            juce::Graphics::ScopedSaveState sss{ g };

            const auto progress = static_cast<float>(slider.getNormalisableRange().convertTo0to1(slider.getValue()));
            g.reduceClipRegion((slider.isHorizontal()
                                    ? bounds.withRight(bounds.proportionOfWidth(1.0f - progress))
                                    : bounds.withTop(bounds.proportionOfHeight(1.0f - progress)))
                                   .toNearestInt());
            drawRoundedRectangle(g,
                                 bounds,
                                 getBorderRadius(slider, styles),
                                 getBorderWidth(slider, styles),
                                 getTrackFill(slider, styles),
                                 juce::Colours::transparentBlack);
        }

        drawRoundedRectangle(g,
                             bounds,
                             getBorderRadius(slider, styles),
                             getBorderWidth(slider, styles),
                             juce::Colours::transparentBlack,
                             getBorderFill(slider, styles));
    }

    inline void paintLinearSlider(juce::Graphics& g,
                                  const juce::Slider& slider,
                                  const Styles& styles)
    {
        const auto bounds = slider
                                .getLookAndFeel()
                                .getSliderLayout(*const_cast<juce::Slider*>(&slider))
                                .sliderBounds.toFloat();
        const auto style = slider.getSliderStyle();
        const auto progress = static_cast<float>(slider.getNormalisableRange().convertTo0to1(slider.getValue()));
        const auto minProgress = static_cast<float>(slider.getNormalisableRange().convertTo0to1(slider.getMinValue()));
        const auto maxProgress = static_cast<float>(slider.getNormalisableRange().convertTo0to1(slider.getMaxValue()));
        const auto isTwoVal = style == juce::Slider::SliderStyle::TwoValueVertical
                           || style == juce::Slider::SliderStyle::TwoValueHorizontal;
        const auto isThreeVal = style == juce::Slider::SliderStyle::ThreeValueVertical
                             || style == juce::Slider::SliderStyle::ThreeValueHorizontal;
        const auto thumbWidth = static_cast<float>(slider
                                                       .getLookAndFeel()
                                                       .getSliderThumbRadius(*const_cast<juce::Slider*>(&slider)));
        const auto trackWidth = thumbWidth * 0.6f;
        const juce::Point startPoint{
            slider.isHorizontal() ? bounds.getX() : bounds.getCentreX(),
            slider.isHorizontal() ? bounds.getCentreY() : bounds.getBottom(),
        };
        const juce::Point endPoint{
            slider.isHorizontal() ? bounds.getRight() : startPoint.x,
            slider.isHorizontal() ? startPoint.y : bounds.getY(),
        };

        juce::Path backgroundTrack;
        backgroundTrack.startNewSubPath(startPoint);
        backgroundTrack.lineTo(endPoint);
        g.setFillType(getBackgroundFill(slider, styles));
        juce::PathStrokeType{
            trackWidth,
            juce::PathStrokeType::curved,
            juce::PathStrokeType::rounded,
        }
            .createStrokedPath(backgroundTrack, backgroundTrack);
        g.fillPath(backgroundTrack);

        juce::Point<float> minPoint;
        juce::Point<float> maxPoint;
        juce::Point<float> thumbPoint;

        thumbPoint = {
            slider.isHorizontal()
                ? bounds.getX() + bounds.proportionOfWidth(progress)
                : bounds.getCentreX(),
            slider.isHorizontal()
                ? bounds.getCentreY()
                : bounds.getY() + bounds.proportionOfHeight(progress)
        };

        if (isTwoVal || isThreeVal)
        {
            minPoint = {
                slider.isHorizontal()
                    ? bounds.getX() + bounds.proportionOfWidth(minProgress)
                    : bounds.getCentreX(),
                slider.isHorizontal()
                    ? bounds.getCentreY()
                    : bounds.getY() + bounds.proportionOfHeight(minProgress)
            };
            maxPoint = {
                slider.isHorizontal()
                    ? bounds.getX() + bounds.proportionOfWidth(maxProgress)
                    : bounds.getCentreX(),
                slider.isHorizontal()
                    ? bounds.getCentreY()
                    : bounds.getY() + bounds.proportionOfHeight(maxProgress)
            };
        }
        else
        {
            minPoint = startPoint;
            maxPoint = endPoint;
        }

        juce::Path valueTrack;
        valueTrack.startNewSubPath(minPoint);
        valueTrack.lineTo(isTwoVal ? maxPoint : thumbPoint);
        g.setFillType(getTrackFill(slider, styles));
        g.strokePath(valueTrack, juce::PathStrokeType{
                                     trackWidth,
                                     juce::PathStrokeType::curved,
                                     juce::PathStrokeType::rounded,
                                 });

        if (isTwoVal || isThreeVal)
        {
            const auto minThumb = juce::Rectangle{ thumbWidth * 0.8f, thumbWidth * 0.8f }
                                      .withCentre(minPoint)
                                      .toFloat();
            g.setFillType(getThumbFill(slider, styles, minThumb));
            g.fillEllipse(minThumb);
            backgroundTrack.addEllipse(minThumb);

            const auto maxThumb = juce::Rectangle{ thumbWidth * 0.8f, thumbWidth * 0.8f }
                                      .withCentre(maxPoint)
                                      .toFloat();
            g.setFillType(getThumbFill(slider, styles, maxThumb));
            g.fillEllipse(maxThumb);
            backgroundTrack.addEllipse(maxThumb);
        }
        if (!isTwoVal)
        {
            const auto thumb = juce::Rectangle{ thumbWidth, thumbWidth }
                                   .withCentre(thumbPoint)
                                   .toFloat();
            g.setFillType(getThumbFill(slider, styles, thumb));
            g.fillEllipse(thumb);
            backgroundTrack.addEllipse(thumb);
        }

        const_cast<juce::Slider*>(&slider)->getProperties().set("jive::shadow-path", backgroundTrack.toString());
    }

    inline void paintRotarySlider(juce::Graphics& g,
                                  const juce::Slider& slider,
                                  const Styles& styles)
    {
        const auto bounds = slider
                                .getLookAndFeel()
                                .getSliderLayout(*const_cast<juce::Slider*>(&slider))
                                .sliderBounds.toFloat()
                                .reduced(10);
        const auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
        const auto rotaryParams = slider.getRotaryParameters();
        const auto sliderPos = static_cast<float>(slider.getNormalisableRange().convertTo0to1(slider.getValue()));
        const auto toAngle = rotaryParams.startAngleRadians + sliderPos * (rotaryParams.endAngleRadians - rotaryParams.startAngleRadians);
        const auto thumbWidth = static_cast<float>(slider
                                                       .getLookAndFeel()
                                                       .getSliderThumbRadius(*const_cast<juce::Slider*>(&slider)));
        const auto trackWidth = thumbWidth * 0.6f;
        const auto arcRadius = radius - trackWidth * 0.5f;

        juce::Path backgroundArc;
        backgroundArc.addCentredArc(bounds.getCentreX(),
                                    bounds.getCentreY(),
                                    arcRadius,
                                    arcRadius,
                                    0.0f,
                                    rotaryParams.startAngleRadians,
                                    rotaryParams.endAngleRadians,
                                    true);
        g.setFillType(getBackgroundFill(slider, styles));
        juce::PathStrokeType{
            trackWidth,
            juce::PathStrokeType::curved,
            juce::PathStrokeType::rounded,
        }
            .createStrokedPath(backgroundArc, backgroundArc);
        g.fillPath(backgroundArc);

        juce::Path valueArc;
        valueArc.addCentredArc(bounds.getCentreX(),
                               bounds.getCentreY(),
                               arcRadius,
                               arcRadius,
                               0.0f,
                               rotaryParams.startAngleRadians,
                               toAngle,
                               true);

        g.setFillType(getTrackFill(slider, styles));
        g.strokePath(valueArc,
                     juce::PathStrokeType{
                         trackWidth,
                         juce::PathStrokeType::curved,
                         juce::PathStrokeType::rounded,
                     });

        juce::Point<float> thumbPoint{
            bounds.getCentreX() + arcRadius * std::cos(toAngle - juce::MathConstants<float>::halfPi),
            bounds.getCentreY() + arcRadius * std::sin(toAngle - juce::MathConstants<float>::halfPi),
        };

        const auto thumb = juce::Rectangle{ thumbWidth, thumbWidth }.withCentre(thumbPoint);
        g.setFillType(getThumbFill(slider, styles, thumb));
        g.fillEllipse(thumb);

        backgroundArc.addEllipse(thumb);
        const_cast<juce::Slider*>(&slider)->getProperties().set("jive::shadow-path", backgroundArc.toString());
    }
} // namespace jive::default_styles
