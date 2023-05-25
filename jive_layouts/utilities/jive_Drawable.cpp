#include <jive_layouts/jive_layouts.h>

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

    Drawable::Drawable(Drawable&& other)
    {
        *this = std::move(other);
    }

    Drawable& Drawable::operator=(Drawable&& other)
    {
        drawable = std::move(other.drawable);
        svgSource = std::move(other.svgSource);
        return *this;
    }

    Drawable::Drawable(juce::Image image)
    {
        *this = image;
    }

    Drawable& Drawable::operator=(juce::Image image)
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

    Drawable::Drawable(const juce::String& svgString)
    {
        *this = svgString;
    }

    Drawable::Drawable(const juce::XmlElement& svgElement)
    {
        *this = svgElement;
    }

    Drawable& Drawable::operator=(const juce::String& svgString)
    {
        svgSource = svgString;
        *this = *juce::parseXML(svgString);
        return *this;
    }

    Drawable& Drawable::operator=(const juce::XmlElement& svgElement)
    {
        if (svgSource.isEmpty())
            svgSource = svgElement.toString();

        drawable = juce::Drawable::createFromSVG(svgElement);
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

namespace juce
{
    jive::Drawable VariantConverter<jive::Drawable>::fromVar(const var& v)
    {
        if (v.isObject())
            return jive::Drawable{ VariantConverter<Image>::fromVar(v) };

        if (v.isString())
            return jive::Drawable{ v.toString() };

        return {};
    }

    var VariantConverter<jive::Drawable>::toVar(const jive::Drawable& drawable)
    {
        if (drawable.isImage())
            return VariantConverter<Image>::toVar(static_cast<Image>(drawable));

        if (drawable.isSVG())
            return static_cast<String>(drawable);

        return {};
    }

    String& operator<<(String& str, const jive::Drawable& /* drawable */)
    {
        return str << "Drawable{}";
    }
} // namespace juce
