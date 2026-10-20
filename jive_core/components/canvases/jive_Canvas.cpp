#include "jive_Canvas.h"

#include <jive_core/jive_core.h>

#include <utility>

namespace jive
{
    Canvas::Canvas(std::function<void(juce::Graphics&)> paintFunction)
        : onPaint{ std::move(paintFunction) }
    {
    }

    void Canvas::paint(juce::Graphics& g)
    {
        if (onPaint != nullptr)
            onPaint(g);
    }
} // namespace jive
