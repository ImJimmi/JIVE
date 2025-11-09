#include "jive_Bezier.h"

namespace jive
{
    std::array<CubicBezier, 4> CubicBezier::makeRoundedRectangle(BorderRadii<float> radii,
                                                                 juce::Rectangle<float> bounds,
                                                                 juce::BorderSize<float> borderWidth)
    {
        static constexpr auto relativeControlPointOffsetForNearPerfectEllipse = 0.552f;

        borderWidth.subtractFrom(bounds);
        const auto maxRadius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;

        radii.topLeft = juce::jlimit(0.0f, maxRadius, radii.topLeft - juce::jmax(borderWidth.getTop(), borderWidth.getLeft()));
        radii.topRight = juce::jlimit(0.0f, maxRadius, radii.topRight - juce::jmax(borderWidth.getTop(), borderWidth.getRight()));
        radii.bottomRight = juce::jlimit(0.0f, maxRadius, radii.bottomRight - juce::jmax(borderWidth.getBottom(), borderWidth.getRight()));
        radii.bottomLeft = juce::jlimit(0.0f, maxRadius, radii.bottomLeft - juce::jmax(borderWidth.getBottom(), borderWidth.getLeft()));

        CubicBezier topLeft;
        topLeft.start = bounds.getTopLeft().translated(0.0f, radii.topLeft);
        topLeft.control1 = topLeft.start.translated(0.0f, -radii.topLeft * relativeControlPointOffsetForNearPerfectEllipse);
        topLeft.end = bounds.getTopLeft().translated(radii.topLeft, 0.0f);
        topLeft.control2 = topLeft.end.translated(-radii.topLeft * relativeControlPointOffsetForNearPerfectEllipse, 0.0f);

        CubicBezier topRight;
        topRight.start = bounds.getTopRight().translated(-radii.topRight, 0.0f);
        topRight.control1 = topRight.start.translated(radii.topRight * relativeControlPointOffsetForNearPerfectEllipse, 0.0f);
        topRight.end = bounds.getTopRight().translated(0.0f, radii.topRight);
        topRight.control2 = topRight.end.translated(0.0f, -radii.topRight * relativeControlPointOffsetForNearPerfectEllipse);

        CubicBezier bottomRight;
        bottomRight.start = bounds.getBottomRight().translated(0.0f, -radii.bottomRight);
        bottomRight.control1 = bottomRight.start.translated(0.0f, radii.bottomRight * relativeControlPointOffsetForNearPerfectEllipse);
        bottomRight.end = bounds.getBottomRight().translated(-radii.bottomRight, 0.0f);
        bottomRight.control2 = bottomRight.end.translated(radii.bottomRight * relativeControlPointOffsetForNearPerfectEllipse, 0.0f);

        CubicBezier bottomLeft;
        bottomLeft.start = bounds.getBottomLeft().translated(radii.bottomLeft, 0.0f);
        bottomLeft.control1 = bottomLeft.start.translated(-radii.bottomLeft * relativeControlPointOffsetForNearPerfectEllipse, 0.0f);
        bottomLeft.end = bounds.getBottomLeft().translated(0.0f, -radii.bottomLeft);
        bottomLeft.control2 = bottomLeft.end.translated(0.0f, radii.bottomLeft * relativeControlPointOffsetForNearPerfectEllipse);

        return std::array{
            topLeft,
            topRight,
            bottomRight,
            bottomLeft,
        };
    }
} // namespace jive
