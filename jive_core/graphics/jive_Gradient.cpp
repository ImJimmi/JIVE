#include "jive_Gradient.h"

#include <jive_core/algorithms/jive_Visitor.h>
#include <jive_core/logging/jive_StringStreams.h>
#include <jive_core/values/jive_Identifiers.h>
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

    juce::Point<float> RadialPosition::toPoint(juce::Rectangle<float> bounds) const
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
                    Length::fromString(keywordOrValue),
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
                    Length::fromString(keywordOrValue),
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

    [[nodiscard]] static float toAngle(SideOrCorner sideOrCorner, juce::Rectangle<float> bounds)
    {
        // https://drafts.csswg.org/css-images-3/#linear-gradient-syntax
        const auto cornerAngle = std::atan2(bounds.getHeight(), bounds.getWidth());

        switch (sideOrCorner)
        {
        case SideOrCorner::topLeft:
            return -cornerAngle;
        case SideOrCorner::top:
            return 0.0f;
        case SideOrCorner::topRight:
            return cornerAngle;
        case SideOrCorner::right:
            return juce::MathConstants<float>::halfPi;
        case SideOrCorner::bottomRight:
            return juce::MathConstants<float>::pi - cornerAngle;
        case SideOrCorner::bottom:
            return juce::MathConstants<float>::pi;
        case SideOrCorner::bottomLeft:
            return juce::MathConstants<float>::pi + cornerAngle;
        case SideOrCorner::left:
            return -juce::MathConstants<float>::halfPi;
        }

        jassertfalse;
        return 0.0f;
    }

    juce::ColourGradient Gradient::toJuceColourGradient(juce::Rectangle<float> bounds) const
    {
        juce::ColourGradient result;

        if (variant == Variant::linear)
        {
            result.isRadial = false;

            const auto angle = std::visit(Visitor{
                                              [](float radians) {
                                                  return radians;
                                              },
                                              [&bounds](SideOrCorner sideOrCorner) {
                                                  return toAngle(sideOrCorner, bounds);
                                              },
                                          },
                                          linearAngle);
            const auto halfLength = (std::abs(bounds.getWidth() * std::sin(angle))
                                     + std::abs(bounds.getHeight() * std::cos(angle)))
                                  / 2.0f;

            result.point1 = bounds.getCentre().getPointOnCircumference(halfLength, angle + juce::MathConstants<float>::pi);
            result.point2 = bounds.getCentre().getPointOnCircumference(halfLength, angle);
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

    [[nodiscard]] static auto splitPreservingParentheses(const juce::String& str, const juce::String& breakChar)
    {
        juce::StringArray parts;

        bool withinParentheses = false;
        auto startIndex = 0;

        for (auto i = 0; i < str.length(); ++i)
        {
            if (i == str.length() - 1)
            {
                parts.add(str.substring(startIndex).trim());
            }
            else if (str[i] == '(')
            {
                withinParentheses = true;
            }
            else if (str[i] == ')')
            {
                withinParentheses = false;
            }
            else if (!withinParentheses && str.substring(i, i + 1) == breakChar)
            {
                parts.add(str.substring(startIndex, i).trim());
                startIndex = i + 1;
            }
        }

        return parts;
    }

    [[nodiscard]] static auto parseColourStops(const juce::StringArray& stopStrings)
    {
        std::vector<Gradient::ColourStop> stops;

        for (const auto& stopString : stopStrings)
        {
            Gradient::ColourStop stop;

            const auto parts = splitPreservingParentheses(stopString.trim(), " ");
            stop.colour = parseColour(parts[0]);

            if (parts.size() > 1)
                stop.length = Length::fromString(parts[1]);

            stops.emplace_back(stop);
        }

        jassert(std::size(stops) > 1);

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

        auto args = splitPreservingParentheses(gradientString
                                                   .fromFirstOccurrenceOf("(", false, true)
                                                   .upToLastOccurrenceOf(")", false, true),
                                               ",");

        if (args[0].contains("deg")
            || args[0].contains("rad")
            || args[0].contains("grad")
            || args[0].contains("turn"))
        {
            gradient.linearAngle = parseAngle(args[0].trim());
            args.remove(0);
        }
        else if (args[0].startsWith("to"))
        {
            auto nextArgs = splitPreservingParentheses(args[0].fromFirstOccurrenceOf(" ", false, false), " ");
            nextArgs.sort(true);
            args.remove(0);

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

        auto args = splitPreservingParentheses(gradientString
                                                   .fromFirstOccurrenceOf("(", false, true)
                                                   .upToLastOccurrenceOf(")", false, true),
                                               ",");

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
                           object->setProperty(jive::ids::x(), "center");
                       },
                       [&object](jive::RadialPosition::Left left) {
                           object->setProperty(jive::ids::x(), "left " + juce::String{ left.inset.toString() });
                       },
                       [&object](jive::RadialPosition::Right right) {
                           object->setProperty(jive::ids::x(), "right " + juce::String{ right.inset.toString() });
                       },
                   },
                   position.xAxis);
        std::visit(jive::Visitor{
                       [&object](jive::RadialPosition::Centre) {
                           object->setProperty(jive::ids::y(), "center");
                       },
                       [&object](jive::RadialPosition::Top top) {
                           object->setProperty(jive::ids::y(), "top " + juce::String{ top.inset.toString() });
                       },
                       [&object](jive::RadialPosition::Bottom bottom) {
                           object->setProperty(jive::ids::y(), "bottom " + juce::String{ bottom.inset.toString() });
                       },
                   },
                   position.yAxis);

        return juce::var{ object.release() };
    }

    jive::RadialPosition VariantConverter<jive::RadialPosition>::fromVar(const var& object)
    {
        jive::RadialPosition position;

        const auto x = object[jive::ids::x()].toString();
        const auto y = object[jive::ids::y()].toString();

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

                                  jassertfalse;
                                  return juce::var{ "top-left" };
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

            stopObject->setProperty(jive::ids::colour(), VariantConverter<Colour>::toVar(stop.colour));

            if (stop.length.has_value())
                stopObject->setProperty(jive::ids::length(), VariantConverter<jive::Length>::toVar(*stop.length));
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
                stop.colour = VariantConverter<Colour>::fromVar(object[jive::ids::colour()]);
                stop.length = object.hasProperty(jive::ids::length())
                                ? std::make_optional(VariantConverter<jive::Length>::fromVar(object[jive::ids::length()]))
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

        object->setProperty(jive::ids::variant(), jive::toVar(gradient.variant));
        object->setProperty(jive::ids::stops(), jive::toVar(gradient.stops));
        object->setProperty(jive::ids::linearAngle(), jive::toVar(gradient.linearAngle));
        object->setProperty(jive::ids::radialSize(), gradient.radialSize);
        object->setProperty(jive::ids::radialPosition(), jive::toVar(gradient.radialPosition));

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

        gradient.variant = jive::fromVar<jive::Gradient::Variant>(value.hasProperty(jive::ids::variant()) ? value[jive::ids::variant()] : value[jive::ids::gradient()]);
        gradient.stops = jive::fromVar<std::vector<jive::Gradient::ColourStop>>(value[jive::ids::stops()]);
        gradient.linearAngle = jive::fromVar<std::variant<float, jive::SideOrCorner>>(value[jive::ids::linearAngle()]);
        gradient.radialSize = static_cast<float>(value[jive::ids::radialSize()]);
        gradient.radialPosition = jive::fromVar<jive::RadialPosition>(value[jive::ids::radialPosition()]);

        return gradient;
    }
} // namespace juce

#if JIVE_UNIT_TESTS
    #include <jive_core/testing/jive_UnitTest.h>

static void expectEndPoints(jive::UnitTest& test,
                            const juce::String& direction,
                            juce::Point<float> start,
                            juce::Point<float> end)
{
    const auto gradient = jive::Gradient::fromString("linear-gradient(" + direction + ", red, blue)")
                              ->toJuceColourGradient({ 200.0f, 100.0f });

    test.expect(gradient.point1.getDistanceFrom(start) < 0.01f,
                direction + " starts at " + gradient.point1.toString());
    test.expect(gradient.point2.getDistanceFrom(end) < 0.01f,
                direction + " ends at " + gradient.point2.toString());
}

static jive::UnitTest linearGradientAnglesTest{
    "jive",
    "jive::Gradient",
    "Linear gradient angles point clockwise from the top towards the last colour",
    [](auto& test) {
        expectEndPoints(test, "0deg", { 100.0f, 100.0f }, { 100.0f, 0.0f });
        expectEndPoints(test, "90deg", { 0.0f, 50.0f }, { 200.0f, 50.0f });
        expectEndPoints(test, "180deg", { 100.0f, 0.0f }, { 100.0f, 100.0f });
        expectEndPoints(test, "270deg", { 200.0f, 50.0f }, { 0.0f, 50.0f });
        expectEndPoints(test, "-90deg", { 200.0f, 50.0f }, { 0.0f, 50.0f });
        expectEndPoints(test, "450deg", { 0.0f, 50.0f }, { 200.0f, 50.0f });
        expectEndPoints(test, "0.5turn", { 100.0f, 0.0f }, { 100.0f, 100.0f });
    },
};

static jive::UnitTest linearGradientLengthTest{
    "jive",
    "jive::Gradient",
    "Linear gradients reach the first and last colours at the corners nearest their ends",
    [](auto& test) {
        expectEndPoints(test, "45deg", { 25.0f, 125.0f }, { 175.0f, -25.0f });
        expectEndPoints(test, "-45deg", { 175.0f, 125.0f }, { 25.0f, -25.0f });
    },
};

static jive::UnitTest linearGradientKeywordsTest{
    "jive",
    "jive::Gradient",
    "Linear gradient keywords point at the given side, or into the given corner",
    [](auto& test) {
        expectEndPoints(test, "to top", { 100.0f, 100.0f }, { 100.0f, 0.0f });
        expectEndPoints(test, "to right", { 0.0f, 50.0f }, { 200.0f, 50.0f });
        expectEndPoints(test, "to bottom", { 100.0f, 0.0f }, { 100.0f, 100.0f });
        expectEndPoints(test, "to left", { 200.0f, 50.0f }, { 0.0f, 50.0f });
        expectEndPoints(test, "to top right", { 60.0f, 130.0f }, { 140.0f, -30.0f });
        expectEndPoints(test, "to bottom right", { 60.0f, -30.0f }, { 140.0f, 130.0f });
        expectEndPoints(test, "to bottom left", { 140.0f, -30.0f }, { 60.0f, 130.0f });
        expectEndPoints(test, "to top left", { 140.0f, 130.0f }, { 60.0f, -30.0f });
    },
};
#endif
