#include "jive_Event.h"

#if JIVE_UNIT_TESTS
    #include <jive_core/logging/jive_StringStreams.h>

class EventUnitTest : public juce::UnitTest
{
public:
    EventUnitTest()
        : juce::UnitTest{ "jive::Event", "jive" }
    {
    }

    void runTest() final
    {
        testTriggering();
        testObserving();
        testTiming();
        testCopying();
    }

private:
    void testTriggering()
    {
        beginTest("triggering");

        juce::ValueTree state{ "State" };
        jive::Event event{ state, "foo" };
        expectEquals(event.getAssumedTriggerCount(), 0);

        event.trigger();
        expect(state.hasProperty("foo"));
        expectEquals(event.getAssumedTriggerCount(), 1);
        const auto value = state["foo"].clone();

        event.trigger();
        expectNotEquals(state["foo"], value);
        expectEquals(event.getAssumedTriggerCount(), 2);

        for (auto i = 0; i < 100; i++)
            event.trigger();

        expectEquals(event.getAssumedTriggerCount(), 102);
    }

    void testObserving()
    {
        beginTest("observing");

        juce::ValueTree state{ "State" };
        jive::Event event1{ state, "bar" };

        {
            auto wasTriggered = false;
            event1.onTrigger = [&wasTriggered]() {
                wasTriggered = true;
            };
            expect(!wasTriggered);
            event1.trigger();
            expect(wasTriggered);

            wasTriggered = false;
            event1.triggerWithoutSelfCallback();
            expect(!wasTriggered);

            event1.onTrigger = nullptr;
        }

        {
            jive::Event event2{ state, "bar" };

            auto wasTriggered = false;
            event2.onTrigger = [&wasTriggered]() {
                wasTriggered = true;
            };
            expect(!wasTriggered);
            event1.trigger();
            expect(wasTriggered);
        }
    }

    void testTiming()
    {
        beginTest("timing");

        juce::ValueTree state{ "State" };
        jive::Event event{ state, "fizz" };
        expectEquals(event.getTimeLastTriggered(), juce::Time{});

        const auto timeJustBefore = juce::Time::getCurrentTime();
        event.trigger();
        const auto timeJustAfter = juce::Time::getCurrentTime();
        expect(event.getTimeLastTriggered() == timeJustBefore
               || event.getTimeLastTriggered() == timeJustAfter);
    }

    void testCopying()
    {
        beginTest("copying");

        juce::ValueTree originalState{ "State" };
        jive::Event originalEvent{ originalState, "event" };

        auto wasTriggered = false;
        originalEvent.onTrigger = [&wasTriggered]() {
            wasTriggered = true;
        };

        auto clonedState = originalState.createCopy();
        originalState = juce::ValueTree{};
        jive::Event clonedEvent{ clonedState, "event" };
        clonedEvent.trigger();
        expect(wasTriggered);
    }
};

static EventUnitTest eventUnitTest;
#endif
