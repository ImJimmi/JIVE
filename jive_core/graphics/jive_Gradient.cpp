#include "jive_Gradient.h"

#include <jive_core/algorithms/jive_Visitor.h>
#include <jive_core/logging/jive_StringStreams.h>
#include <jive_core/values/variant-converters/jive_VariantConvertion.h>

namespace jive
{
    [[nodiscard]] static bool operator==(const RadialPosition::Top& lhs, const RadialPosition::Top& rhs)
    {
        return lhs.inset == rhs.inset;
    }

    [[nodiscard]] static bool operator==(const RadialPosition::Right& lhs, const RadialPosition::Right& rhs)
    {
        return lhs.inset == rhs.inset;
    }

    [[nodiscard]] static bool operator==(const RadialPosition::Bottom& lhs, const RadialPosition::Bottom& rhs)
    {
        return lhs.inset == rhs.inset;
    }

    [[nodiscard]] static bool operator==(const RadialPosition::Left& lhs, const RadialPosition::Left& rhs)
    {
        return lhs.inset == rhs.inset;
    }

    [[nodiscard]] static bool operator==(const RadialPosition::Centre&, const RadialPosition::Centre&)
    {
        return true;
    }

    bool RadialPosition::operator==(const RadialPosition& other) const
    {
        return xAxis == other.xAxis && yAxis == other.yAxis;
    }

    juce::Point<float> RadialPosition::toPoint(const juce::Rectangle<float>& bounds) const
    {
        return {
            std::visit(Visitor{
                           [&bounds](const Centre&) {
                               return bounds.getCentreX();
                           },
                           [&bounds](const Left& left) {
                               return bounds.getX() + left.inset.toPixels(bounds.getWidth());
                           },
                           [&bounds](const Right& right) {
                               return bounds.getRight() - right.inset.toPixels(bounds.getWidth());
                           },
                       },
                       xAxis),
            std::visit(Visitor{
                           [&bounds](const Centre&) {
                               return bounds.getCentreX();
                           },
                           [&bounds](const Top& top) {
                               return bounds.getY() + top.inset.toPixels(bounds.getHeight());
                           },
                           [&bounds](const Bottom& bottom) {
                               return bounds.getBottom() - bottom.inset.toPixels(bounds.getHeight());
                           },
                       },
                       yAxis),
        };
    }

    std::optional<RadialPosition> RadialPosition::fromArgs(juce::StringArray& args)
    {
        static const juce::StringArray xAxisKeywords{ "left", "centre", "center", "right" };
        static const juce::StringArray yAxisKeywords{ "top", "centre", "center", "bottom" };

        static constexpr auto applyToXAxis = [](const auto& keywordOrValue, auto& position) {
            if (keywordOrValue == "center" || keywordOrValue == "centre")
                position.xAxis = RadialPosition::Centre{};
            else if (keywordOrValue == "left")
                position.xAxis = RadialPosition::Left{};
            else if (keywordOrValue == "right")
                position.xAxis = RadialPosition::Right{};
            else
                position.xAxis = RadialPosition::Left{
                    Length::fromString(keywordOrValue)
                };
        };
        static constexpr auto applyToYAxis = [](const auto& keywordOrValue, auto& position) {
            if (keywordOrValue == "center" || keywordOrValue == "centre")
                position.yAxis = RadialPosition::Centre{};
            else if (keywordOrValue == "top")
                position.yAxis = RadialPosition::Top{};
            else if (keywordOrValue == "bottom")
                position.yAxis = RadialPosition::Bottom{};
            else
                position.yAxis = RadialPosition::Top{
                    Length::fromString(keywordOrValue)
                };
        };

        RadialPosition result;

        if (args.size() == 1)
        {
            if (args[0] == "center" || args[0] == "centre")
            {
                applyToXAxis(args[0], result);
                applyToYAxis(args[0], result);
            }
            else if (yAxisKeywords.contains(args[0]))
            {
                applyToYAxis(args[0], result);
            }
            else
            {
                applyToYAxis(args[0], result);
            }
        }
        else if (args.size() == 2)
        {
            applyToXAxis(args[0], result);
            applyToYAxis(args[1], result);
        }
        else if (args.size() == 4)
        {
            if (args.contains("centre")
                || args.contains("center")
                || xAxisKeywords.contains(args[2])
                || yAxisKeywords.contains(args[0]))
            {
                return std::nullopt;
            }

            if (args[0] == "left")
                result.xAxis = RadialPosition::Left{ Length::fromString(args[1]) };
            else
                result.xAxis = RadialPosition::Right{ Length::fromString(args[1]) };

            if (args[2] == "top")
                result.yAxis = RadialPosition::Top{ Length::fromString(args[3]) };
            else
                result.yAxis = RadialPosition::Bottom{ Length::fromString(args[3]) };
        }

        return result;
    }

    bool Gradient::ColourStop::operator==(const ColourStop& other) const
    {
        return colour == other.colour && length == other.length;
    }

    juce::ColourGradient Gradient::toJuceColourGradient(const juce::Rectangle<float>& bounds) const
    {
        const juce::Line top{ bounds.getTopLeft(), bounds.getTopRight() };
        const juce::Line right{ bounds.getTopRight(), bounds.getBottomRight() };
        const juce::Line bottom{ bounds.getBottomLeft(), bounds.getBottomRight() };
        const juce::Line left{ bounds.getTopLeft(), bounds.getBottomLeft() };
        const auto angleToTopLeft = bounds.getCentre().getAngleToPoint(bounds.getTopLeft());
        const auto angleToTopRight = bounds.getCentre().getAngleToPoint(bounds.getTopRight());
        const auto angleToBottomRight = bounds.getCentre().getAngleToPoint(bounds.getBottomRight());
        const auto angleToBottomLeft = bounds.getCentre().getAngleToPoint(bounds.getBottomLeft());

        juce::ColourGradient result;

        if (variant == Variant::linear)
        {
            result.isRadial = false;

            if (std::holds_alternative<float>(linearAngle))
            {
                const auto angle = std::get<float>(linearAngle);
                const juce::Line line{
                    bounds.getCentre().getPointOnCircumference(1'000'000.0f, angle),
                    bounds.getCentre().getPointOnCircumference(1'000'000.0f, angle + juce::MathConstants<float>::pi),
                };

                if (angle > angleToTopLeft && angle <= angleToTopRight)
                {
                    result.point1 = line.getIntersection(top);
                    result.point2 = line.getIntersection(bottom);
                }
                else if (angle > angleToTopRight && angle <= angleToBottomRight)
                {
                    result.point1 = line.getIntersection(right);
                    result.point2 = line.getIntersection(left);
                }
                else if (angle > angleToBottomLeft && angle <= angleToBottomRight)
                {
                    result.point1 = line.getIntersection(bottom);
                    result.point2 = line.getIntersection(top);
                }
                else
                {
                    result.point1 = line.getIntersection(left);
                    result.point2 = line.getIntersection(right);
                }
            }
            else
            {
                switch (std::get<SideOrCorner>(linearAngle))
                {
                case SideOrCorner::topLeft:
                    result.point1 = bounds.getBottomRight();
                    result.point2 = bounds.getTopLeft();
                    break;
                case SideOrCorner::top:
                    result.point1 = bottom.getPointAlongLineProportionally(0.5f);
                    result.point2 = top.getPointAlongLineProportionally(0.5f);
                    break;
                case SideOrCorner::topRight:
                    result.point1 = bounds.getBottomLeft();
                    result.point2 = bounds.getTopRight();
                    break;
                case SideOrCorner::right:
                    result.point1 = left.getPointAlongLineProportionally(0.5f);
                    result.point2 = right.getPointAlongLineProportionally(0.5f);
                    break;
                case SideOrCorner::bottomRight:
                    result.point1 = bounds.getTopLeft();
                    result.point2 = bounds.getBottomRight();
                    break;
                case SideOrCorner::bottom:
                    result.point1 = top.getPointAlongLineProportionally(0.5f);
                    result.point2 = bottom.getPointAlongLineProportionally(0.5f);
                    break;
                case SideOrCorner::bottomLeft:
                    result.point1 = bounds.getTopRight();
                    result.point2 = bounds.getBottomLeft();
                    break;
                case SideOrCorner::left:
                    result.point1 = right.getPointAlongLineProportionally(0.5f);
                    result.point2 = left.getPointAlongLineProportionally(0.5f);
                    break;
                }
            }
        }
        else
        {
            result.isRadial = true;
            result.point1 = radialPosition.toPoint(bounds);
            result.point2 = bounds.getBottomRight();
        }

        std::vector<const ColourStop*> autoStops;
        std::vector<const ColourStop*> definedStops;

        for (const auto& stop : stops)
        {
            if (stop.length.has_value())
                definedStops.emplace_back(&stop);
            else
                autoStops.emplace_back(&stop);
        }

        for (const auto&& [index, stop] : juce::enumerate(autoStops))
        {
            const auto proportion = static_cast<double>(index) / (static_cast<int>(std::size(autoStops)) - 1);
            result.addColour(proportion, stop->colour);
        }

        for (const auto* stop : definedStops)
        {
            const auto proportion = stop->length->unit == Length::Unit::percentage
                                      ? (stop->length->value / 100.0f)
                                      : stop->length->value / juce::Line{ result.point1, result.point2 }.getLength();
            result.addColour(static_cast<double>(proportion), stop->colour);
        }

        return result;
    }

    bool Gradient::operator==(const Gradient& other) const
    {
        if (variant != other.variant || stops != other.stops)
            return false;

        if (variant == Variant::linear)
            return linearAngle == other.linearAngle;

        return juce::approximatelyEqual(radialSize, other.radialSize)
            && radialPosition == other.radialPosition;
    }

    [[nodiscard]] static auto parseColourStops(const juce::StringArray& stopStrings)
    {
        std::vector<Gradient::ColourStop> stops;

        for (const auto& stopString : stopStrings)
        {
            Gradient::ColourStop stop;

            const auto parts = juce::StringArray::fromTokens(stopString.trim(), " ", "()");
            stop.colour = parseColour(parts[0]);

            if (parts.size() > 1)
                stop.length = Length::fromString(parts[1]);

            stops.emplace_back(stop);
        }

        return stops;
    }

    [[nodiscard]] static float parseAngle(const juce::String& angleString)
    {
        // https://developer.mozilla.org/en-US/docs/Web/CSS/angle
        const auto value = angleString.getFloatValue();

        if (angleString.endsWith("deg"))
            return (value / 180.0f) * juce::MathConstants<float>::pi;

        if (angleString.endsWith("grad"))
            return (value / 200.0f) * juce::MathConstants<float>::pi;

        if (angleString.endsWith("turn"))
            return value * juce::MathConstants<float>::twoPi;

        jassert(angleString.endsWith("rad"));
        return value;
    }

    [[nodiscard]] static auto parseLinearGradient(const juce::String& gradientString)
    {
        Gradient gradient;
        gradient.variant = Gradient::Variant::linear;

        auto args = juce::StringArray::fromTokens(gradientString
                                                      .fromFirstOccurrenceOf("(", false, true)
                                                      .upToLastOccurrenceOf(")", false, true),
                                                  ",",
                                                  "()");

        if (args[0].contains("deg")
            || args[0].contains("rad")
            || args[0].contains("grad")
            || args[0].contains("turn"))
        {
            gradient.linearAngle = parseAngle(args[0].trim());
            args.remove(0);
        }
        else if (args[0] == "to")
        {
            args.remove(0);

            juce::StringArray sides{ "top", "right", "bottom", "left" };
            juce::StringArray nextArgs;

            for (auto _ = 0; _ < 2; _++)
            {
                if (sides.contains(args[0]))
                {
                    nextArgs.add(args[0]);
                    args.remove(0);
                }
                else
                {
                    break;
                }
            }

            nextArgs.sort(true);

            if (nextArgs == juce::StringArray{ "left", "top" })
                gradient.linearAngle = SideOrCorner::topLeft;
            else if (nextArgs == juce::StringArray{ "top" })
                gradient.linearAngle = SideOrCorner::top;
            else if (nextArgs == juce::StringArray{ "right", "top" })
                gradient.linearAngle = SideOrCorner::topRight;
            else if (nextArgs == juce::StringArray{ "right" })
                gradient.linearAngle = SideOrCorner::right;
            else if (nextArgs == juce::StringArray{ "bottom", "right" })
                gradient.linearAngle = SideOrCorner::bottomRight;
            else if (nextArgs == juce::StringArray{ "bottom" })
                gradient.linearAngle = SideOrCorner::bottom;
            else if (nextArgs == juce::StringArray{ "bottom", "left" })
                gradient.linearAngle = SideOrCorner::bottomLeft;
            else if (nextArgs == juce::StringArray{ "left" })
                gradient.linearAngle = SideOrCorner::left;
            else
                jassertfalse;
        }

        gradient.stops = parseColourStops(args);

        return gradient;
    }

    [[nodiscard]] static auto parseRadialGradient(const juce::String& gradientString)
    {
        Gradient gradient;
        gradient.variant = Gradient::Variant::radial;

        auto args = juce::StringArray::fromTokens(gradientString
                                                      .fromFirstOccurrenceOf("(", false, true)
                                                      .upToLastOccurrenceOf(")", false, true),
                                                  ",",
                                                  "()");

        if (args[0].containsOnly("0123456789.px%"))
        {
            gradient.radialSize = args[0].getFloatValue();
            args.remove(0);
        }

        if (args[0].startsWith("at"))
        {
            auto parts = juce::StringArray::fromTokens(args[0], " ", "");
            parts.remove(0);
            gradient.radialPosition = *RadialPosition::fromArgs(parts);

            args.remove(0);
        }

        gradient.stops = parseColourStops(args);

        return gradient;
    }

    std::optional<Gradient> Gradient::fromString(const juce::String& gradientString)
    {
        if (gradientString.startsWith("linear-gradient"))
            return parseLinearGradient(gradientString);
        if (gradientString.startsWith("radial-gradient"))
            return parseRadialGradient(gradientString);

        return std::nullopt;
    }
} // namespace jive

namespace juce
{
    var VariantConverter<jive::RadialPosition>::toVar(const jive::RadialPosition& position)
    {
        auto object = std::make_unique<DynamicObject>();

        std::visit(jive::Visitor{
                       [&object](jive::RadialPosition::Centre) {
                           object->setProperty("x", "center");
                       },
                       [&object](jive::RadialPosition::Left left) {
                           object->setProperty("x", "left " + juce::String{ left.inset.toString() });
                       },
                       [&object](jive::RadialPosition::Right right) {
                           object->setProperty("x", "right " + juce::String{ right.inset.toString() });
                       },
                   },
                   position.xAxis);
        std::visit(jive::Visitor{
                       [&object](jive::RadialPosition::Centre) {
                           object->setProperty("y", "center");
                       },
                       [&object](jive::RadialPosition::Top top) {
                           object->setProperty("y", "top " + juce::String{ top.inset.toString() });
                       },
                       [&object](jive::RadialPosition::Bottom bottom) {
                           object->setProperty("y", "bottom " + juce::String{ bottom.inset.toString() });
                       },
                   },
                   position.yAxis);

        return juce::var{ object.release() };
    }

    jive::RadialPosition VariantConverter<jive::RadialPosition>::fromVar(const var& object)
    {
        jive::RadialPosition position;

        const auto x = object["x"].toString();
        const auto y = object["y"].toString();

        if (x == "center")
            position.xAxis = jive::RadialPosition::Centre{};
        else if (x.startsWith("left"))
            position.xAxis = jive::RadialPosition::Left{
                jive::Length::fromString(x.fromLastOccurrenceOf(" ", false, false)),
            };
        else if (x.startsWith("right"))
            position.xAxis = jive::RadialPosition::Right{
                jive::Length::fromString(x.fromLastOccurrenceOf(" ", false, false)),
            };

        if (y == "center")
            position.yAxis = jive::RadialPosition::Centre{};
        else if (y.startsWith("top"))
            position.yAxis = jive::RadialPosition::Top{
                jive::Length::fromString(y.fromLastOccurrenceOf(" ", false, false)),
            };
        else if (y.startsWith("bottom"))
            position.yAxis = jive::RadialPosition::Bottom{
                jive::Length::fromString(y.fromLastOccurrenceOf(" ", false, false)),
            };

        return position;
    }

    var VariantConverter<std::variant<float, jive::SideOrCorner>>::toVar(const std::variant<float, jive::SideOrCorner>& linearAngle)
    {
        return std::visit(jive::Visitor{
                              [](float f) {
                                  return juce::var{ f };
                              },
                              [](jive::SideOrCorner sideOrCorner) {
                                  switch (sideOrCorner)
                                  {
                                  case jive::SideOrCorner::topLeft:
                                      return juce::var{ "top-left" };
                                  case jive::SideOrCorner::top:
                                      return juce::var{ "top" };
                                  case jive::SideOrCorner::topRight:
                                      return juce::var{ "top-right" };
                                  case jive::SideOrCorner::right:
                                      return juce::var{ "right" };
                                  case jive::SideOrCorner::bottomRight:
                                      return juce::var{ "bottom-right" };
                                  case jive::SideOrCorner::bottom:
                                      return juce::var{ "bottom" };
                                  case jive::SideOrCorner::bottomLeft:
                                      return juce::var{ "bottom-left" };
                                  case jive::SideOrCorner::left:
                                      return juce::var{ "left" };
                                  }
                              },
                          },
                          linearAngle);
    }

    std::variant<float, jive::SideOrCorner> VariantConverter<std::variant<float, jive::SideOrCorner>>::fromVar(const var& object)
    {
        if (object.toString() == "top-left")
            return jive::SideOrCorner::topLeft;
        if (object.toString() == "top")
            return jive::SideOrCorner::top;
        if (object.toString() == "top-right")
            return jive::SideOrCorner::topRight;
        if (object.toString() == "right")
            return jive::SideOrCorner::right;
        if (object.toString() == "bottom-right")
            return jive::SideOrCorner::bottomRight;
        if (object.toString() == "bottom")
            return jive::SideOrCorner::bottom;
        if (object.toString() == "bottom-left")
            return jive::SideOrCorner::bottomLeft;
        if (object.toString() == "left")
            return jive::SideOrCorner::left;

        return static_cast<float>(object);
    }

    var VariantConverter<std::vector<jive::Gradient::ColourStop>>::toVar(const std::vector<jive::Gradient::ColourStop>& stops)
    {
        juce::Array<juce::var> result;

        for (const auto& stop : stops)
        {
            auto stopObject = std::make_unique<DynamicObject>();

            stopObject->setProperty("colour", VariantConverter<Colour>::toVar(stop.colour));

            if (stop.length.has_value())
                stopObject->setProperty("length", VariantConverter<jive::Length>::toVar(*stop.length));
        }

        return result;
    }

    std::vector<jive::Gradient::ColourStop> VariantConverter<std::vector<jive::Gradient::ColourStop>>::fromVar(const var& value)
    {
        std::vector<jive::Gradient::ColourStop> stops;

        if (value.isArray())
        {
            for (const auto& object : *value.getArray())
            {
                jive::Gradient::ColourStop stop;
                stop.colour = VariantConverter<Colour>::fromVar(object["colour"]);
                stop.length = object.hasProperty("length")
                                ? std::make_optional(VariantConverter<jive::Length>::fromVar(object["length"]))
                                : std::nullopt;
                stops.push_back(stop);
            }
        }
        else if (value.isObject())
        {
            for (const auto& [length, colour] : value.getDynamicObject()->getProperties())
            {
                jive::Gradient::ColourStop stop;
                stop.colour = VariantConverter<Colour>::fromVar(colour);
                stop.length = std::make_optional(VariantConverter<jive::Length>::fromVar(juce::String{ length.toString().getFloatValue() * 100.0f } + "%"));
                stops.push_back(stop);
            }
        }
        else
        {
            jassertfalse;
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

        object->setProperty("variant", jive::toVar(gradient.variant));
        object->setProperty("stops", jive::toVar(gradient.stops));
        object->setProperty("linear-angle", jive::toVar(gradient.linearAngle));
        object->setProperty("radial-size", gradient.radialSize);
        object->setProperty("radial-position", jive::toVar(gradient.radialPosition));

        return juce::var{ object.release() };
    }

    jive::Gradient VariantConverter<jive::Gradient>::fromVar(const var& value)
    {
        if (value.isString())
        {
            if (auto result = jive::Gradient::fromString(value.toString()); result.has_value())
                return *result;
        }

        jive::Gradient gradient;

        gradient.variant = jive::fromVar<jive::Gradient::Variant>(value.hasProperty("variant") ? value["variant"] : value["gradient"]);
        gradient.stops = jive::fromVar<std::vector<jive::Gradient::ColourStop>>(value["stops"]);
        gradient.linearAngle = jive::fromVar<std::variant<float, jive::SideOrCorner>>(value["linear-angle"]);
        gradient.radialSize = static_cast<float>(value["radial-size"]);
        gradient.radialPosition = jive::fromVar<jive::RadialPosition>(value["radial-position"]);

        return gradient;
    }
} // namespace juce
