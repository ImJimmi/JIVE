#include <jive_core/jive_core.h>

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
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
        const auto bounds = getLocalBounds().toFloat();

        juce::Path path;
        path.addRectangle(bounds);
        g.reduceClipRegion(path);

        apply(background, g, bounds);
        g.fillAll();

        apply(borderFill, g, bounds);
        g.strokePath(path, juce::PathStrokeType{ borderWidth * 2.0f });
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

    float BackgroundCanvas::getBorderWidth() const
    {
        return borderWidth;
    }

    void BackgroundCanvas::setBorder(const Fill& newFill, float newWidth)
    {
        if (newFill != borderFill || newWidth != borderWidth)
            repaint();

        borderFill = newFill;
        borderWidth = newWidth;
    }
} // namespace jive
