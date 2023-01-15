#include <jive_core/jive_core.h>

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    struct CubicBezier
    {
        juce::Point<float> start{ 0.0f, 0.0f };
        juce::Point<float> control1{ 0.0f, 0.0f };
        juce::Point<float> control2{ 1.0f, 1.0f };
        juce::Point<float> end{ 1.0f, 1.0f };
    };

    std::array<CubicBezier, 4> getCorners(BorderRadii<float> radii,
                                          juce::Rectangle<float> bounds)
    {
        static constexpr auto relativeControlPointOffsetForNearPerfectEllipse = 0.552f;

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

        return {
            topLeft,
            topRight,
            bottomRight,
            bottomLeft,
        };
    }

    juce::Path getShape(const BackgroundCanvas& background)
    {
        juce::Path path;

        for (const auto& corner : getCorners(background.getBorderRadii(),
                                             background.getLocalBounds().toFloat()))
        {
            if (path.isEmpty())
                path.startNewSubPath(corner.start);
            else
                path.lineTo(corner.start);

            path.cubicTo(corner.control1, corner.control2, corner.end);
        }

        path.closeSubPath();

        return path;
    }

    void apply(const Fill& fill, juce::Graphics& g, juce::Rectangle<float> bounds)
    {
        if (fill.getGradient().has_value())
            g.setGradientFill(fill.getGradient()->toJuceGradient(bounds));
        else if (fill.getColour().has_value())
            g.setColour(*fill.getColour());
        else
            g.setColour(juce::Colour{});
    }

    void BackgroundCanvas::paint(juce::Graphics& g)
    {
        const auto shape = getShape(*this);
        g.reduceClipRegion(shape);

        const auto bounds = getLocalBounds().toFloat();
        apply(background, g, bounds);
        g.fillAll();

        apply(borderFill, g, bounds);
        g.strokePath(shape, juce::PathStrokeType{ borderWidth * 2.0f });
    }

    //==================================================================================================================
    Fill BackgroundCanvas::getFill() const
    {
        return background;
    }

    void BackgroundCanvas::setFill(const Fill& newFill)
    {
        if (newFill != background)
            repaint();

        background = newFill;
    }

    Fill BackgroundCanvas::getBorderFill() const
    {
        return borderFill;
    }

    void BackgroundCanvas::setBorderFill(const Fill& newFill)
    {
        if (newFill != borderFill)
            repaint();

        borderFill = newFill;
    }

    float BackgroundCanvas::getBorderWidth() const
    {
        return borderWidth;
    }

    void BackgroundCanvas::setBorderWidth(float newWidth)
    {
        if (newWidth != borderWidth)
            repaint();

        borderWidth = newWidth;
    }

    BorderRadii<float> BackgroundCanvas::getBorderRadii() const
    {
        return borderRadii;
    }

    void BackgroundCanvas::setBorderRadii(BorderRadii<float> newRadii)
    {
        if (newRadii != borderRadii)
            repaint();

        borderRadii = newRadii;
    }
} // namespace jive
