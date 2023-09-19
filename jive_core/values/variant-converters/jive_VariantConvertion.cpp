#include <jive_core/jive_core.h>

#if JIVE_UNIT_TESTS
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
};
#endif
