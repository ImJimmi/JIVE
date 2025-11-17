#include "jive_BackgroundCanvas.h"

#include <jive_core/algorithms/jive_Bezier.h>

namespace jive
{
    BackgroundCanvas::BackgroundCanvas()
    {
        setInterceptsMouseClicks(false, false);
    }

    void BackgroundCanvas::paint(juce::Graphics& g)
    {
        const auto bounds = getLocalBounds().toFloat();

        g.reduceClipRegion(shape);

        const auto backgroundScale = 1.0f - ((borderWidth / 2.0f) / getWidth());
        g.setFillType(background.toJuceFillType(bounds));
        g.fillPath(shape,
                   juce::AffineTransform::scale(backgroundScale,
                                                backgroundScale,
                                                bounds.getCentreX(),
                                                bounds.getCentreY()));

        g.setFillType(borderFill.toJuceFillType(bounds));
        g.strokePath(shape,
                     juce::PathStrokeType{
                         borderWidth * 2.0f,
                     });
    }

    void BackgroundCanvas::resized()
    {
        updateShape();
    }

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
        if (!juce::approximatelyEqual(newWidth, borderWidth))
            updateShape();

        borderWidth = newWidth;
    }

    BorderRadii<float> BackgroundCanvas::getBorderRadii() const
    {
        return borderRadii;
    }

    void BackgroundCanvas::setBorderRadii(BorderRadii<float> newRadii)
    {
        if (newRadii != borderRadii)
            updateShape();

        borderRadii = newRadii;
    }

    template <typename Arithmetic>
    [[nodiscard]] static auto limited(BorderRadii<Arithmetic> radii, Arithmetic min, Arithmetic max)
    {
        static_assert(std::is_arithmetic<Arithmetic>());

        radii.topLeft = juce::jlimit(min, max, radii.topLeft);
        radii.topRight = juce::jlimit(min, max, radii.topRight);
        radii.bottomLeft = juce::jlimit(min, max, radii.bottomLeft);
        radii.bottomRight = juce::jlimit(min, max, radii.bottomRight);

        return radii;
    }

    void BackgroundCanvas::updateShape()
    {
        shape.clear();

        for (const auto& corner : CubicBezier::makeRoundedRectangle(getBorderRadii(),
                                                                    getLocalBounds().toFloat()))
        {
            if (shape.isEmpty())
                shape.startNewSubPath(corner.start);
            else
                shape.lineTo(corner.start);

            shape.cubicTo(corner.control1, corner.control2, corner.end);
        }

        shape.closeSubPath();

        repaint();
    }
} // namespace jive
