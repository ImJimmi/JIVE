#include "jive_Transitions.h"

#include <jive_core/logging/jive_StringStreams.h>
#include <jive_core/time/jive_TimeParser.h>

namespace jive
{
    std::optional<Transitions::Transition> Transitions::Transition::fromString(const juce::String& transitionString)
    {
        auto parts = juce::StringArray::fromTokens(transitionString, " ", "");
        parts.removeEmptyStrings();

        if (parts.size() < 2)
            return std::nullopt;

        Transition transition;

        const auto duration = parseTime(parts.getReference(1));

        if (!duration.has_value())
            return std::nullopt;

        transition.duration = *duration;

        if (parts.size() > 2)
        {
            if (parts.size() == 3)
            {
                if (isValidTimeString(parts.getReference(2)))
                    parts.insert(2, "linear");
                else
                    parts.add("0s");
            }

            jassert(parts.size() == 4);
            transition.timingFunction = easing::fromString(parts.getReference(2))
                                            .value_or(easing::linear);
            transition.delay = parseTime(parts.getReference(3))
                                   .value_or(juce::RelativeTime::seconds(0.0));
        }

        return transition;
    }

    int Transitions::size() const
    {
        return static_cast<int>(std::size(transitions));
    }

    Transitions::Transition& Transitions::operator[](const juce::String& propertyName)
    {
        return transitions[propertyName];
    }

    Transitions::ReferenceCountedPointer Transitions::fromString(const juce::String& s)
    {
        const auto tokens = juce::StringArray::fromTokens(s, ",", "");
        auto object = std::make_unique<Transitions>();

        for (const auto& token : tokens)
        {
            const auto transitionString = token.trim();

            if (const auto transition = Transition::fromString(transitionString);
                transition.has_value())
            {
                const auto propertyName = transitionString.upToFirstOccurrenceOf(" ", false, true);
                object->transitions[propertyName] = *transition;
            }
        }

        return object.release();
    }
} // namespace jive

#if JIVE_UNIT_TESTS
class TransitionsTests : public juce::UnitTest
{
public:
    TransitionsTests()
        : juce::UnitTest{ "jive::Transitions", "jive" }
    {
    }

    void runTest() final
    {
        testTransitionParsing();
        testTransitionsParsing();
        testInterpolation();
    }

private:
    [[nodiscard]] auto expectSameEasing(jive::Easing a, jive::Easing b)
    {
        for (auto x = 0.0; x <= 1.0; x += 0.01)
        {
            if (!juce::exactlyEqual(a(x), b(x)))
            {
                expectEquals(a(x), b(x));
                return;
            }
        }

        expect(true);
    }

    void testTransitionParsing()
    {
        beginTest("transition with 1 part");
        {
            {
                const auto transition = jive::Transitions::Transition::fromString("foo");
                expect(!transition.has_value());
            }
            {
                const auto transition = jive::Transitions::Transition::fromString("50ms");
                expect(!transition.has_value());
            }
            {
                const auto transition = jive::Transitions::Transition::fromString("ease");
                expect(!transition.has_value());
            }
        }

        beginTest("transition with 2 parts");
        {
            {
                const auto transition = jive::Transitions::Transition::fromString("margin-right 4s");
                expect(transition.has_value());
                expectEquals(transition->duration, juce::RelativeTime::seconds(4.0));
                expectSameEasing(transition->timingFunction, jive::easing::linear);
                expectEquals(transition->delay, juce::RelativeTime::seconds(0.0));
            }
            {
                const auto transition = jive::Transitions::Transition::fromString("whatever 37ms");
                expect(transition.has_value());
                expectEquals(transition->duration, juce::RelativeTime::seconds(0.037));
                expectSameEasing(transition->timingFunction, jive::easing::linear);
                expectEquals(transition->delay, juce::RelativeTime::seconds(0.0));
            }
            {
                const auto transition = jive::Transitions::Transition::fromString("foo bar");
                expect(!transition.has_value());
            }
            {
                const auto transition = jive::Transitions::Transition::fromString("x ease-in");
                expect(!transition.has_value());
            }
        }

        beginTest("transition with 3 parts");
        {
            {
                const auto transition = jive::Transitions::Transition::fromString("margin-right 4s 1s");
                expect(transition.has_value());
                expectEquals(transition->duration, juce::RelativeTime::seconds(4.0));
                expectSameEasing(transition->timingFunction, jive::easing::linear);
                expectEquals(transition->delay, juce::RelativeTime::seconds(1.0));
            }
            {
                const auto transition = jive::Transitions::Transition::fromString("width 3ms 96ms");
                expect(transition.has_value());
                expectEquals(transition->duration, juce::RelativeTime::seconds(0.003));
                expectSameEasing(transition->timingFunction, jive::easing::linear);
                expectEquals(transition->delay, juce::RelativeTime::seconds(0.096));
            }
            {
                const auto transition = jive::Transitions::Transition::fromString("margin-right 4s ease-in-out");
                expect(transition.has_value());
                expectEquals(transition->duration, juce::RelativeTime::seconds(4.0));
                expectSameEasing(transition->timingFunction, jive::easing::inOut);
                expectEquals(transition->delay, juce::RelativeTime::seconds(0.0));
            }
            {
                const auto transition = jive::Transitions::Transition::fromString("foo 84ms ease-out");
                expect(transition.has_value());
                expectEquals(transition->duration, juce::RelativeTime::seconds(0.084));
                expectSameEasing(transition->timingFunction, jive::easing::out);
                expectEquals(transition->delay, juce::RelativeTime::seconds(0.0));
            }
        }

        beginTest("transition with 4 parts");
        {
            {
                const auto transition = jive::Transitions::Transition::fromString("margin-right 4s ease-in-out 1s");
                expect(transition.has_value());
                expectEquals(transition->duration, juce::RelativeTime::seconds(4.0));
                expectSameEasing(transition->timingFunction, jive::easing::inOut);
                expectEquals(transition->delay, juce::RelativeTime::seconds(1.0));
            }
            {
                const auto transition = jive::Transitions::Transition::fromString("margin-right 3ms linear 99999ms");
                expect(transition.has_value());
                expectEquals(transition->duration, juce::RelativeTime::seconds(0.003));
                expectSameEasing(transition->timingFunction, jive::easing::linear);
                expectEquals(transition->delay, juce::RelativeTime::seconds(99.999));
            }
        }
    }

    void testTransitionsParsing()
    {
        beginTest("transitions");
        {
            const auto transitions = jive::Transitions::fromString("width 200ms, opacity 400ms");
            expect(transitions != nullptr);
            expectEquals(transitions->size(), 2);
            expectEquals((*transitions)["width"].duration, juce::RelativeTime::seconds(0.2));
            expectEquals((*transitions)["opacity"].duration, juce::RelativeTime::seconds(0.4));
            expectEquals((*transitions)["foo"].duration, juce::RelativeTime::seconds(0.0));
            expectEquals(transitions->size(), 3);
        }
    }

    void testInterpolation()
    {
        beginTest("linear interpolation");
        {
            const auto transition = jive::Transitions::Transition::fromString("width 3s");
            const auto commencement = jive::now();
            expectEquals(transition->calculateCurrent(0.0f, 100.0f, commencement), 0.0f);
            jive::FakeTime::incrementTime(juce::RelativeTime::seconds(1.5));
            expectEquals(transition->calculateCurrent(0.0f, 100.0f, commencement), 50.0f);
            jive::FakeTime::incrementTime(juce::RelativeTime::seconds(1.5));
            expectEquals(transition->calculateCurrent(0.0f, 100.0f, commencement), 100.0f);
            jive::FakeTime::incrementTime(juce::RelativeTime::seconds(1.5));
            expectEquals(transition->calculateCurrent(0.0f, 100.0f, commencement), 100.0f);
        }

        beginTest("eased interpolation");
        {
            const auto transition = jive::Transitions::Transition::fromString("height 420ms ease-in");
            const auto commencement = jive::now();
            expectEquals(transition->calculateCurrent(0.0f, 100.0f, commencement), 0.0f);
            jive::FakeTime::incrementTime(juce::RelativeTime::seconds(0.21));
            expectLessThan(transition->calculateCurrent(0.0f, 100.0f, commencement), 50.0f);
        }
    }
};

static TransitionsTests transitionsTests;
#endif
