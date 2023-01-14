#include "jive_BackgroundCanvas.h"

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    void BackgroundCanvas::paint(juce::Graphics& g)
    {
        if (fill.getGradient().has_value())
            g.setGradientFill(fill.getGradient()->toJuceGradient(getLocalBounds().toFloat()));
        else if (fill.getColour().has_value())
            g.setColour(*fill.getColour());
        else
            g.setColour(juce::Colour{});

        g.fillAll();
    }

    //==================================================================================================================
    Fill BackgroundCanvas::getFill() const
    {
        return fill;
    }

    void BackgroundCanvas::setFill(const Fill& newFill)
    {
        if (newFill != fill)
            repaint();

        fill = newFill;
    }
} // namespace jive
