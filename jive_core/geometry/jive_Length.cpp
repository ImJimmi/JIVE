#include "jive_Length.h"

#include <jive_core/values/variant-converters/jive_VariantConvertion.h>

namespace jive
{
    bool Length::operator==(const Length& other) const
    {
        return juce::approximatelyEqual(value, other.value) && unit == other.unit;
    }

    juce::String Length::toString() const
    {
        if (unit == Unit::absolute)
            return juce::String{ value } + "px";
        if (unit == Unit::percentage)
            return juce::String{ value } + "%";

        jassertfalse;
        return "";
    }

    Length Length::fromString(const juce::String& lengthString)
    {
        if (lengthString.endsWith("%"))
        {
            return Length{
                lengthString.getFloatValue(),
                Unit::percentage,
            };
        }

        return Length{
            lengthString.getFloatValue(),
            Unit::absolute,
        };
    }
} // namespace jive

namespace juce
{
    var VariantConverter<jive::Length>::toVar(const jive::Length& length)
    {
        return length.toString();
    }

    jive::Length VariantConverter<jive::Length>::fromVar(const var& value)
    {
        return value.toString();
    }
} // namespace juce

#if JIVE_UNIT_TESTS
    #include <jive_layouts/jive_layouts.h>

class LengthUnitTest : public juce::UnitTest
{
public:
    LengthUnitTest()
        : juce::UnitTest{ "jive::Length", "jive" }
    {
    }

    void runTest() final
    {
        testPixels();
        testPercent();
    }

private:
    std::unique_ptr<jive::GuiItem> createGuiItem(juce::ValueTree tree)
    {
        jive::Interpreter interpreter;

        return interpreter.interpret(tree);
    }

    void testPixels()
    {
        beginTest("pixels");

        juce::ValueTree tree{ "Component" };
        jive::Property<jive::Length> width{ tree, "width" };
        expect(width.isAuto());

        width = "10px";
        expectEquals(width.get().toPixels(0.0f), 10.f);

        width = "312.65px";
        expectEquals(width.get().toPixels(0.0f), 312.65f);
    }

    void testPercent()
    {
        beginTest("percent");

        {
            juce::ValueTree tree{
                "Component",
                {
                    { "width", 40.f },
                    { "height", 20.f },
                },
                {
                    juce::ValueTree{ "Component" },
                }
            };
            jive::Property<jive::Length> width{ tree.getChild(0), "width" };
            width = "50%";
            expectEquals(width.get().toPixels(40.0f), 20.f);

            jive::Property<jive::Length> height{ tree.getChild(0), "height" };
            height = "20%";
            expectWithinAbsoluteError(height.get().toPixels(20.0f), 4.f, 0.000001f);
        }
    }
};

static LengthUnitTest lengthUnitTest;
#endif
