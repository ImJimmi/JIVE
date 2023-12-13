#include "jive_Timer.h"

namespace jive
{
#if JIVE_UNIT_TESTS
    FakeTime::~FakeTime()
    {
        clearSingletonInstance();
    }

    void FakeTime::addListener(Listener& listener)
    {
        listeners.add(&listener);
    }

    void FakeTime::removeListener(Listener& listener)
    {
        listeners.remove(&listener);
    }

    void FakeTime::incrementTime(juce::RelativeTime increment)
    {
        getInstance()->incrementTimeInternal(increment);
    }

    juce::Time FakeTime::now() noexcept
    {
        return getInstance()->currentTime;
    }

    void FakeTime::incrementTimeInternal(juce::RelativeTime increment)
    {
        currentTime += increment;
        listeners.call(&Listener::timeChanged);
    }

    JUCE_IMPLEMENT_SINGLETON(FakeTime)
#endif

    Timer::Timer(Timer::Callback timerCallback,
                 juce::RelativeTime callbackInterval)
        : callback{ timerCallback }
        , interval{ callbackInterval }
    {
#if JIVE_UNIT_TESTS
        timeLastCallbackInvoked = FakeTime::now();
        FakeTime::getInstance()->addListener(*this);
#else
        timeLastCallbackInvoked = juce::Time::getCurrentTime();
        startTimer(static_cast<int>(interval.inMilliseconds()));
#endif
    }

    Timer::~Timer()
    {
#if JIVE_UNIT_TESTS
        FakeTime::getInstance()->removeListener(*this);
#endif
    }

#if JIVE_UNIT_TESTS
    void Timer::timeChanged()
    {
        for (auto elapsed = FakeTime::now() - timeLastCallbackInvoked;
             elapsed.inMilliseconds() > interval.inMilliseconds();
             elapsed -= interval)
        {
            timeLastCallbackInvoked = FakeTime::now() - elapsed + interval;
            callback(timeLastCallbackInvoked);
        }
    }
#else
    void Timer::timerCallback()
    {
        callback(juce::Time::getCurrentTime());
    }
#endif
} // namespace jive

#if JIVE_UNIT_TESTS
class TimerUnitTest : public juce::UnitTest
{
public:
    TimerUnitTest()
        : juce::UnitTest{ "jive::Timer", "jive" }
    {
    }

    void runTest() final
    {
        const auto initialTime = jive::FakeTime::now();

        beginTest("faking the passing of time");
        jive::FakeTime::incrementTime(juce::RelativeTime::seconds(5.3));
        expectEquals(jive::FakeTime::now(), initialTime + juce::RelativeTime::seconds(5.3));

        beginTest("getting callbacks");
        juce::Array<juce::Time> invokeTimes;
        jive::Timer timer{
            [&invokeTimes](juce::Time now) mutable {
                invokeTimes.add(now);
            },
            juce::RelativeTime::seconds(0.1),
        };
        jive::FakeTime::incrementTime(juce::RelativeTime::seconds(0.25));
        expectEquals(invokeTimes.size(), 2);
        expectEquals(invokeTimes[0], initialTime + juce::RelativeTime::seconds(5.4));
        expectEquals(invokeTimes[1], initialTime + juce::RelativeTime::seconds(5.5));

        jive::FakeTime::incrementTime(juce::RelativeTime::seconds(0.3333));
        expectEquals(invokeTimes.size(), 5);
        expectEquals(invokeTimes[0], initialTime + juce::RelativeTime::seconds(5.4));
        expectEquals(invokeTimes[1], initialTime + juce::RelativeTime::seconds(5.5));
        expectEquals(invokeTimes[2], initialTime + juce::RelativeTime::seconds(5.6));
        expectEquals(invokeTimes[3], initialTime + juce::RelativeTime::seconds(5.7));
        expectEquals(invokeTimes[4], initialTime + juce::RelativeTime::seconds(5.8));
    }
};

static TimerUnitTest timerUnitTest;
#endif
