#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace jive
{
    class Drawable
    {
    public:
        Drawable() = default;
        Drawable(const Drawable& other);
        Drawable& operator=(const Drawable& other);
        Drawable(Drawable&& other);
        Drawable& operator=(Drawable&& other);

        explicit Drawable(juce::Image image);
        Drawable& operator=(juce::Image image);
        operator juce::Image() const;
        bool isImage() const;

        explicit Drawable(const juce::String& svgString);
        explicit Drawable(const juce::XmlElement& svgElement);
        Drawable& operator=(const juce::String& svgString);
        Drawable& operator=(const juce::XmlElement& svgElement);
        operator juce::String() const;
        bool isSVG() const;

        bool isEmpty() const;

    private:
        std::unique_ptr<juce::Drawable> drawable;
        juce::String svgSource;

        JUCE_LEAK_DETECTOR(Drawable)
    };
} // namespace jive

namespace juce
{
    template <>
    struct VariantConverter<jive::Drawable>
    {
        static jive::Drawable fromVar(const var& v);
        static var toVar(const jive::Drawable& drawable);
    };

    String& operator<<(String& str, const jive::Drawable& drawable);
} // namespace juce
