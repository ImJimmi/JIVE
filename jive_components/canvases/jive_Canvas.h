#pragma once

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