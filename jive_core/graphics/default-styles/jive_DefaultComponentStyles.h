#pragma once

#include <jive_core/geometry/jive_BorderRadii.h>
#include <jive_core/geometry/jive_BorderWidth.h>
#include <jive_core/geometry/jive_Geometry.h>

namespace jive::default_styles
{
    namespace detail
    {
    } // namespace detail

    inline void drawRoundedRectangle(juce::Graphics& g,
                                     const juce::Rectangle<float>& bounds,
                                     const BorderRadii<float>& borderRadius,
                                     const juce::BorderSize<float> borderWidth,
                                     const juce::FillType& backgroundFill,
                                     const juce::FillType& borderFill)
    {
        if (backgroundFill.isInvisible() && borderFill.isInvisible())
            return;

        const auto fillShape = getShape(borderRadius,
                                        bounds,
                                        juce::BorderSize{
                                            borderWidth.getTop() * 0.5f,
                                            borderWidth.getLeft() * 0.5f,
                                            borderWidth.getBottom() * 0.5f,
                                            borderWidth.getRight() * 0.5f,
                                        });
        g.setFillType(backgroundFill);
        g.fillPath(fillShape);

        if (borderFill.isInvisible())
            return;

        if (borderWidth == juce::BorderSize<float>{})
            return;

        auto borderShape = getShape(borderRadius, bounds);
        borderShape.setUsingNonZeroWinding(false);
        borderShape.addPath(getShape(borderRadius, bounds, borderWidth));

        g.setFillType(borderFill);
        g.fillPath(borderShape);
    }

    template <typename Component>
    inline void paintComponent(juce::Graphics& g,
                               const Component& component,
                               const Styles& styles)
    {
        const auto bounds = component.getLocalBounds().toFloat();
        const auto borderRadius = getBorderRadius(component, styles);

        drawRoundedRectangle(g,
                             bounds,
                             borderRadius,
                             getBorderWidth(component, styles),
                             getBackgroundFill(component, styles),
                             getBorderFill(component, styles));

        const auto shape = getShape(borderRadius, bounds);
        const_cast<Component*>(&component)
            ->getProperties()
            .set("jive::shadow-path",
                 shape.toString());
    }
} // namespace jive::default_styles
