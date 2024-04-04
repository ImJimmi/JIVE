#include "jive_VariantConvertion.h"

#if JIVE_UNIT_TESTS
    #include <jive_core/logging/jive_StringStreams.h>
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
        expectEquals(jive::fromVar<juce::Rectangle<int>>("2 4 5 6"), juce::Rectangle{ 2, 4, 6, 8 });
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
};
#endif
