#include "jive_Canvas.h"

namespace jive
{
    Canvas::Canvas(std::function<void(juce::Graphics&)> paintFunction)
        : onPaint{ paintFunction }
    {
    }

    void Canvas::paint(juce::Graphics& g)
    {
        if (onPaint != nullptr)
            onPaint(g);
    }
} // namespace jive
