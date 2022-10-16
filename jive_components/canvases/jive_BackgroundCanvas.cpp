#include "jive_BackgroundCanvas.h"

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    void BackgroundCanvas::paint(juce::Graphics& g)
    {
        if (fill.getGradient().hasValue())
            g.setGradientFill(fill.getGradient()->toJuceGradient(getLocalBounds().toFloat()));
        else if (fill.getColour().hasValue())
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
        fill = newFill;
        repaint();
    }
} // namespace jive
