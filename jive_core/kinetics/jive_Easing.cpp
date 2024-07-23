#include "jive_Easing.h"

namespace jive::easing
{
    std::optional<Easing> fromString(const juce::String& name)
    {
        static const std::unordered_map<juce::String, Easing> namedEasings{
            { "linear", linear },
            { "ease", inOut },
            { "ease-in", in },
            { "ease-out", out },
            { "ease-in-out", inOut },
        };

        if (const auto result = namedEasings.find(name);
            result != std::end(namedEasings))
            return result->second;

        if (name.startsWith("cubic-bezier(") && name.endsWith(")"))
        {
            static constexpr auto includeSubStringsInResults = false;
            static constexpr auto ignoreCase = true;
            const auto args = juce::StringArray::fromTokens(name
                                                                .fromFirstOccurrenceOf("(", includeSubStringsInResults, ignoreCase)
                                                                .upToLastOccurrenceOf(")", includeSubStringsInResults, ignoreCase),
                                                            ",",
                                                            "");

            if (args.size() == 4)
            {
                const juce::Point startControlPoint{
                    static_cast<float>(args[0].getDoubleValue()),
                    static_cast<float>(args[1].getDoubleValue()),
                };
                const juce::Point endControlPoint{
                    static_cast<float>(args[2].getDoubleValue()),
                    static_cast<float>(args[3].getDoubleValue()),
                };

                return [startControlPoint, endControlPoint](double x) {
                    static const auto transferFunction = jive::cubicBezier(startControlPoint, endControlPoint);
                    return solveFor(transferFunction, x);
                };
            }
        }

        return std::nullopt;
    }
} // namespace jive::easing

#if JIVE_UNIT_TESTS
class EasingTest : public juce::UnitTest
{
public:
    EasingTest()
        : juce::UnitTest{ "jive::Easing", "jive" }
    {
    }

    void runTest() final
    {
        testEasingFunctions();
        testParsing();
    }

private:
    void testEasingFunctions()
    {
        beginTest("easing::linear");
        expectEquals(jive::easing::linear(0.5), 0.5);

        beginTest("easing::inOut");
        expectGreaterThan(jive::easing::inOut(0.5), 0.5);

        beginTest("easing::out");
        expectGreaterThan(jive::easing::out(0.5), 0.5);

        beginTest("easing::in");
        expectLessThan(jive::easing::in(0.5), 0.5);
    }

    void testParsing()
    {
        beginTest("parsing / linear");
        expectEquals((*jive::easing::fromString("linear"))(0.5), jive::easing::linear(0.5));
        beginTest("parsing / ease");
        expectEquals((*jive::easing::fromString("ease"))(0.5), jive::easing::inOut(0.5));
        beginTest("parsing / ease-in");
        expectEquals((*jive::easing::fromString("ease-in"))(0.5), jive::easing::in(0.5));
        beginTest("parsing / ease-out");
        expectEquals((*jive::easing::fromString("ease-out"))(0.5), jive::easing::out(0.5));
        beginTest("parsing / ease-in-out");
        expectEquals((*jive::easing::fromString("ease-in-out"))(0.5), jive::easing::inOut(0.5));

        beginTest("parsing / cubic-bezier()");
        expect(!jive::easing::fromString("cubic-bezier()").has_value());
        expect(!jive::easing::fromString("cubic-bezier(0)").has_value());
        expect(!jive::easing::fromString("cubic-bezier(0, 1)").has_value());
        expect(!jive::easing::fromString("cubic-bezier(0, 4, 5)").has_value());
        expect(jive::easing::fromString("cubic-bezier(0, 4, 5, 2)").has_value());
    }
};

static EasingTest easingTest;
#endif
