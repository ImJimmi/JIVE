#include "jive_Canvas.h"

#include <jive_core/jive_core.h>

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
