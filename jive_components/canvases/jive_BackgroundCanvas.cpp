#include "jive_BackgroundCanvas.h"

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    void BackgroundCanvas::paint(juce::Graphics& g)
    {
        g.setFillType(fill);
        g.fillAll();
    }

    //==================================================================================================================
    juce::FillType BackgroundCanvas::getFill() const
    {
        return fill;
    }

    void BackgroundCanvas::setFill(const juce::FillType& newFill)
    {
        fill = newFill;
    }
} // namespace jive
