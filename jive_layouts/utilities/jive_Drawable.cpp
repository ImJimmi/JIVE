#include "jive_Drawable.h"

#include <jive_core/jive_core.h>

namespace jive
{
    Drawable::Drawable(const Drawable& other)
    {
        *this = other;
    }

    Drawable& Drawable::operator=(const Drawable& other)
    {
        if (other.drawable != nullptr)
            drawable = other.drawable->createCopy();

        svgSource = other.svgSource;
        return *this;
    }

    Drawable::Drawable(const juce::Image& image)
    {
        *this = image;
    }

    Drawable& Drawable::operator=(const juce::Image& image)
    {
        svgSource.clear();
        drawable = std::make_unique<juce::DrawableImage>(image);
        return *this;
    }

    Drawable::operator juce::Image() const
    {
        return dynamic_cast<juce::DrawableImage&>(*drawable).getImage();
    }

    bool Drawable::isImage() const
    {
        return dynamic_cast<juce::DrawableImage*>(drawable.get()) != nullptr;
    }

#if JUCE_VERSION < JIVE_JUCE_VERSION(9, 0, 0)
    Drawable::Drawable(const juce::XmlElement& svgElement)
    {
        *this = svgElement;
    }

    Drawable& Drawable::operator=(const juce::XmlElement& svgElement)
    {
        if (svgSource.isEmpty())
            svgSource = svgElement.toString();

        drawable = juce::Drawable::createFromSVG(svgElement);
        return *this;
    }
#endif

    Drawable::Drawable(const juce::String& svgString)
    {
        *this = svgString;
    }

    Drawable& Drawable::operator=(const juce::String& svgString)
    {
        svgSource = svgString;

#if JIVE_JUCE_9
        drawable = juce::Drawable::createFromSVGString(svgString);
#else
        *this = *juce::parseXML(svgString);
#endif

        return *this;
    }

    Drawable::operator juce::String() const
    {
        return svgSource;
    }

    bool Drawable::isSVG() const
    {
        return svgSource.isNotEmpty();
    }

    bool Drawable::isEmpty() const
    {
        return drawable == nullptr;
    }
} // namespace jive
