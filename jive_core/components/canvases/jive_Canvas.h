#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace jive
{
    class Canvas : public juce::Component
    {
    public:
        Canvas() = default;
        explicit Canvas(std::function<void(juce::Graphics&)> paintFunction);

        void paint(juce::Graphics& g) final;

        std::function<void(juce::Graphics&)> onPaint;
    };
} // namespace jive
