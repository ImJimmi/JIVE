#include "jive_VariantConvertion.h"

#if JIVE_UNIT_TESTS
    #include <jive_core/logging/jive_StringStreams.h>
    #include <jive_core/values/jive_Number.h>
    #include <jive_core/values/jive_Property.h>
    #include <juce_gui_basics/juce_gui_basics.h>

class VarConversionUnitTest : public juce::UnitTest
{
public:
    VarConversionUnitTest()
        : juce::UnitTest{ "jive::toVar/fromVar", "jive" }
    {
    }

    void runTest() final
    {
        testToVar();
        testFromVar();
        testOptionalParsing();
        testMultiValueParsing();
        testValidation();
    }

private:
    void testToVar()
    {
        beginTest("toVar()");

        expectEquals(jive::toVar<juce::String>("Hello, World!"), juce::var{ "Hello, World!" });
        expectEquals(jive::toVar(123), juce::var{ 123 });
        expectEquals(jive::toVar(juce::FlexBox::AlignItems::center), juce::VariantConverter<juce::FlexBox::AlignItems>::toVar(juce::FlexBox::AlignItems::center));
    }

    void testFromVar()
    {
        beginTest("fromVar()");

        expectEquals(jive::fromVar<long long>(juce::var{ 808 }), 808LL);
        expectEquals(jive::fromVar<juce::Rectangle<int>>("2 4 6 8"), juce::Rectangle{ 2, 4, 6, 8 });
    }

    void testOptionalParsing()
    {
        beginTest("optionals");

        expectEquals(jive::toVar(std::optional<int>{}), juce::var{});
        expectEquals(jive::toVar(std::make_optional(123)), juce::var{ 123 });
    }

    void testMultiValueParsing()
    {
        beginTest("multiple values - toVar");
        {
            expectEquals(jive::toVar(1, "foo", 66.6),
                         juce::var{
                             juce::Array{
                                 juce::var{ 1 },
                                 juce::var{ "foo" },
                                 juce::var{ 66.6 },
                             },
                         });
        }

        beginTest("multiple values - fromVar");
        {
            const auto [actualString, actualInt] = jive::fromVar<juce::String, int>(juce::var{
                juce::Array{
                    juce::var{ "bar" },
                    juce::var{ 7531 },
                },
            });
            const juce::String expectedString = "bar";
            const auto expectedInt = 7531;
            expectEquals(actualString, expectedString);
            expectEquals(actualInt, expectedInt);
        }
    }

    void testValidation()
    {
        beginTest("validation");

        expect(jive::isValidVar<juce::FlexBox::Direction>("row"));
        expect(!jive::isValidVar<juce::FlexBox::Direction>("sideways"));
        expect(jive::isValidVar<juce::FlexBox::Direction>({}));
        expect(jive::isValidVar<juce::BorderSize<float>>("1 2px 3"));
        expect(!jive::isValidVar<juce::BorderSize<float>>("1 2 3 4 5"));
        expect(!jive::isValidVar<juce::BorderSize<float>>("wide"));
        expect(jive::isValidVar<juce::Colour>("#ABC"));
        expect(jive::isValidVar<juce::Colour>("hotpink"));
        expect(!jive::isValidVar<juce::Colour>("#ABCDE"));
        expect(jive::isValidVar<juce::Array<juce::Grid::TrackInfo>>("1fr auto 20px"));
        expect(!jive::isValidVar<juce::Array<juce::Grid::TrackInfo>>("1fr big"));
        expect(jive::isValidVar<juce::GridItem::StartAndEndProperty>("1 / span 2"));
        expect(!jive::isValidVar<juce::GridItem::StartAndEndProperty>("1"));
        expect(jive::isValidVar<bool>(true));
        expect(jive::isValidVar<bool>("false"));
        expect(jive::isValidVar<bool>("1"));
        expect(!jive::isValidVar<bool>("ture"));

        using Number = jive::Number<float, struct NumberTag>;
        expect(jive::isValidVar<Number>(12.5));
        expect(jive::isValidVar<Number>("-12.5"));
        expect(jive::isValidVar<Number>({}));
        expect(!jive::isValidVar<Number>("big"));
        expect(!jive::isValidVar<Number>("12px"));
        expectEquals(jive::fromVar<Number>("12.5").value, 12.5f);

        beginTest("validation - invalid property values are removed");
        {
            juce::ValueTree state{ "State", { { "direction", "sideways" } } };
            jive::Property<juce::FlexBox::Direction> direction{ state, "direction" };
            expect(!state.hasProperty("direction"));

            state.setProperty("direction", "row", nullptr);
            expect(direction.get() == juce::FlexBox::Direction::row);

            state.setProperty("direction", "backwards", nullptr);
            expect(!state.hasProperty("direction"));
        }

        beginTest("validation - invalid numbers are removed");
        {
            juce::ValueTree state{ "State", { { "number", "big" } } };
            jive::Property<jive::Number<float, struct NumberTag>> number{ state, "number" };
            expect(!state.hasProperty("number"));

            state.setProperty("number", "1.5", nullptr);
            expectEquals(number.get().value, 1.5f);
        }
    }
};

static VarConversionUnitTest varConversionUnitTest;
#endif
