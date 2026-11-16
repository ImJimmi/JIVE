#pragma once

#include <jive_core/compile-time-helpers/jive_JuceVersion.h>
#include <juce_gui_basics/juce_gui_basics.h>

namespace jive
{
    class Drawable
    {
    public:
        Drawable() = default;
        Drawable(Drawable&& other) = default;
        Drawable& operator=(Drawable&& other) = default;
        ~Drawable() = default;

        Drawable(const Drawable& other);
        Drawable& operator=(const Drawable& other);

        explicit Drawable(const juce::Image& image);
        Drawable& operator=(const juce::Image& image);
        operator juce::Image() const;
        bool isImage() const;

#if JUCE_VERSION < JIVE_JUCE_VERSION(9, 0, 0)
        explicit Drawable(const juce::XmlElement& svgElement);
        Drawable& operator=(const juce::XmlElement& svgElement);
#endif

        explicit Drawable(const juce::String& svgString);
        Drawable& operator=(const juce::String& svgString);
        operator juce::String() const;
        bool isSVG() const;

        bool isEmpty() const;

    private:
        std::unique_ptr<juce::Drawable> drawable;
        juce::String svgSource;

        JUCE_LEAK_DETECTOR(Drawable)
    };
} // namespace jive
