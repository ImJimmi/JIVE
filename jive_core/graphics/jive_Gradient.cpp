#include "jive_Gradient.h"

namespace jive
{
    bool Gradient::ColourStop::operator==(const ColourStop& other) const
    {
        return juce::approximatelyEqual(proportion, other.proportion)
            && colour == other.colour;
    }

    bool Gradient::ColourStop::operator!=(const ColourStop& other) const
    {
        return !(*this == other);
    }

    juce::ColourGradient Gradient::toJuceGradient(const juce::Rectangle<float>& bounds) const
    {
        juce::ColourGradient gradient;

        gradient.isRadial = variant == Variant::radial;

        for (const auto& stop : stops)
            gradient.addColour(stop.proportion, stop.colour);

        if (orientation.hasValue())
        {
            jassert(!startEndPoints.hasValue());

            gradient.point1 = bounds.getTopLeft();

            switch (*orientation)
            {
            case Orientation::horizontal:
                gradient.point2 = bounds.getTopRight();
                break;
            case Orientation::vertical:
                gradient.point2 = bounds.getBottomLeft();
                break;
            }
        }

        if (startEndPoints.hasValue())
        {
            jassert(!orientation.hasValue());

            const auto bottomRightRelative = bounds.getBottomRight() - bounds.getTopLeft();
            gradient.point1 = bounds.getTopLeft() + startEndPoints->getStart() * bottomRightRelative;
            gradient.point2 = bounds.getTopLeft() + startEndPoints->getEnd() * bottomRightRelative;
        }

        return gradient;
    }

    bool Gradient::operator==(const Gradient& other) const
    {
        return stops == other.stops
            && variant == other.variant
            && orientation == other.orientation
            && startEndPoints == other.startEndPoints;
    }

    bool Gradient::operator!=(const Gradient& other) const
    {
        return !(*this == other);
    }
} // namespace jive

namespace juce
{
    var VariantConverter<Array<jive::Gradient::ColourStop>>::toVar(const Array<jive::Gradient::ColourStop>& stops)
    {
        auto object = std::make_unique<DynamicObject>();

        for (const auto& stop : stops)
        {
            object->setProperty(String{ stop.proportion },
                                VariantConverter<Colour>::toVar(stop.colour));
        }

        return var{ object.release() };
    }

    Array<jive::Gradient::ColourStop> VariantConverter<Array<jive::Gradient::ColourStop>>::fromVar(const var& value)
    {
        auto* object = value.getDynamicObject();
        jassert(object != nullptr);

        Array<jive::Gradient::ColourStop> stops;

        for (const auto& namedValue : object->getProperties())
        {
            stops.add({
                namedValue.name.toString().getDoubleValue(),
                VariantConverter<Colour>::fromVar(namedValue.value),
            });
        }

        return stops;
    }

    var VariantConverter<jive::Gradient::Variant>::toVar(const jive::Gradient::Variant& variant)
    {
        switch (variant)
        {
        case jive::Gradient::Variant::linear:
            return "linear";
        case jive::Gradient::Variant::radial:
            return "radial";
        }

        jassertfalse;
        return var{};
    }

    jive::Gradient::Variant VariantConverter<jive::Gradient::Variant>::fromVar(const var& value)
    {
        jassert(value.isString());

        if (value.toString() == "linear")
            return jive::Gradient::Variant::linear;
        if (value.toString() == "radial")
            return jive::Gradient::Variant::radial;

        jassertfalse;
        return {};
    }

    var VariantConverter<jive::Gradient>::toVar(const jive::Gradient& gradient)
    {
        auto object = std::make_unique<DynamicObject>();

        object->setProperty("stops",
                            VariantConverter<Array<jive::Gradient::ColourStop>>::toVar(gradient.stops));
        object->setProperty("gradient",
                            VariantConverter<jive::Gradient::Variant>::toVar(gradient.variant));

        if (gradient.orientation.hasValue())
        {
            jassert(!gradient.startEndPoints.hasValue());

            object->setProperty("orientation",
                                VariantConverter<jive::Orientation>::toVar(*gradient.orientation));
        }
        else if (gradient.startEndPoints.hasValue())
        {
            jassert(!gradient.orientation.hasValue());

            object->setProperty("start",
                                VariantConverter<Point<float>>::toVar(gradient.startEndPoints->getStart()));
            object->setProperty("end",
                                VariantConverter<Point<float>>::toVar(gradient.startEndPoints->getEnd()));
        }

        return var{ object.release() };
    }

    jive::Gradient VariantConverter<jive::Gradient>::fromVar(const var& value)
    {
        jassert(value.isObject());

        jive::Gradient gradient;

        gradient.stops = VariantConverter<Array<jive::Gradient::ColourStop>>::fromVar(value["stops"]);
        gradient.variant = VariantConverter<jive::Gradient::Variant>::fromVar(value["gradient"]);

        if (value.hasProperty("orientation"))
        {
            gradient.orientation = VariantConverter<jive::Orientation>::fromVar(value["orientation"]);
        }
        else if (value.hasProperty("start") || value.hasProperty("end"))
        {
            gradient.startEndPoints = juce::Line<float>{
                VariantConverter<Point<float>>::fromVar(value["start"]),
                VariantConverter<Point<float>>::fromVar(value["end"]),
            };
        }
        else
        {
            gradient.orientation = jive::Orientation::vertical;
        }

        return gradient;
    }
} // namespace juce
