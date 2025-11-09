#include "jive_Geometry.h"

#include "jive_BorderWidth.h"

#include <jive_core/algorithms/jive_Bezier.h>
#include <jive_core/graphics/jive_FontUtilities.h>
#include <jive_core/graphics/jive_Styles.h>

namespace jive
{
    juce::Path getShape(BorderRadii<float> borderRadius,
                        juce::Rectangle<float> bounds,
                        juce::BorderSize<float> borderWidth)
    {
        juce::Path shape;

        for (const auto& corner : CubicBezier::makeRoundedRectangle(borderRadius, bounds, borderWidth))
        {
            if (shape.isEmpty())
                shape.startNewSubPath(corner.start);
            else
                shape.lineTo(corner.start);

            shape.cubicTo(corner.control1, corner.control2, corner.end);
        }

        shape.closeSubPath();
        return shape;
    }

    juce::Rectangle<float> getTitleBounds(const juce::GroupComponent& group,
                                          const Styles& styles)
    {
        const auto font = getFont(group, styles);
        const auto borderRadius = getBorderRadius(group, styles);

        const auto textInset = std::ceil(juce::jmax(font.getHeight(),
                                                    borderRadius.topLeft + 10.0f,
                                                    borderRadius.topRight + 10.0f));

        auto bounds = juce::Rectangle{
            juce::GlyphArrangement::getStringWidthInt(font, group.getText()),
            static_cast<int>(std::ceil(font.getHeight())),
        }
                          .toFloat();

        if (group.getTextLabelPosition().testFlags(juce::Justification::left))
            bounds.setX(textInset);
        else if (group.getTextLabelPosition().testFlags(juce::Justification::right))
            bounds.setX(static_cast<float>(group.getWidth()) - bounds.getWidth() - textInset);
        else
            bounds.setX(group.getLocalBounds().toFloat().getCentreX() - bounds.proportionOfWidth(0.5f));

        return bounds;
    }

    juce::Rectangle<float> getGroupBounds(const juce::GroupComponent& group,
                                          const Styles& styles)
    {
        return group
            .getLocalBounds()
            .toFloat()
            .withTop(std::round(getTitleBounds(group, styles).getCentreY()
                                - getBorderWidth(group, styles).getTop() * 0.5f));
    }

    juce::Rectangle<float> getTrackBounds(const juce::ProgressBar& bar,
                                          const Styles& styles,
                                          double progressStart,
                                          double progressEnd)
    {
        const auto borderWidth = getBorderWidth(bar, styles);
        const auto barBounds = bar.getLocalBounds().toFloat();
        const auto fillableBounds = borderWidth.subtractedFrom(barBounds);
        const auto start = fillableBounds.getAspectRatio() > 1.0f
                             ? fillableBounds.getX() + fillableBounds.proportionOfWidth(progressStart)
                             : fillableBounds.getY() + fillableBounds.proportionOfHeight(1.0 - progressStart);
        const auto end = fillableBounds.getAspectRatio() > 1.0f
                           ? fillableBounds.getX() + fillableBounds.proportionOfWidth(progressEnd)
                           : fillableBounds.getY() + fillableBounds.proportionOfHeight(1.0 - progressEnd);
        return fillableBounds.getAspectRatio() > 1.0f
                 ? fillableBounds.withLeft(start).withRight(end)
                 : fillableBounds.withTop(start).withBottom(end);
    }
} // namespace jive
