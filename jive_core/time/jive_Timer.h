#pragma once

#include <juce_events/juce_events.h>

namespace jive
{
#if JIVE_UNIT_TESTS
    class FakeTime : public juce::DeletedAtShutdown
    {
    public:
        struct Listener
        {
            virtual ~Listener() = default;

            virtual void timeChanged() = 0;
        };

        FakeTime() = default;
        ~FakeTime();

        void addListener(Listener& listener);
        void removeListener(Listener& listener);

        static void incrementTime(juce::RelativeTime);
        static juce::Time now() noexcept;

        JUCE_DECLARE_SINGLETON(FakeTime, false)

    private:
        void incrementTimeInternal(juce::RelativeTime);

        juce::Time currentTime;
        juce::ListenerList<Listener> listeners;
    };
#endif

    class Timer
#if JIVE_UNIT_TESTS
        : private FakeTime::Listener
#else
        : private juce::Timer
#endif
    {
    public:
        using Callback = std::function<void(juce::Time)>;

        Timer(Callback timerCallback,
              juce::RelativeTime callbackInterval);
        ~Timer();

    private:
#if JIVE_UNIT_TESTS
        void timeChanged() final;
#else
        void timerCallback() final;
#endif

        Callback callback;
        juce::RelativeTime interval;
        juce::Time timeLastCallbackInvoked;
    };

    [[nodiscard]] static inline juce::Time now() noexcept
    {
#if JIVE_UNIT_TESTS
        return FakeTime::now();
#else
        return juce::Time::getCurrentTime();
#endif
    }
} // namespace jive
