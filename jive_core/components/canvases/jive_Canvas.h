#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace jive
{
    class Canvas : public juce::Component
    {
    public:
        explicit Canvas(std::function<void(juce::Graphics&)> paintFunction = nullptr);

        void paint(juce::Graphics& g) final;

        std::function<void(juce::Graphics&)> onPaint;

    private:
        JUCE_LEAK_DETECTOR(Canvas)
    };
} // namespace jive
