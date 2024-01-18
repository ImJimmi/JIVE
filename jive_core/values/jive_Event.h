#pragma once

#include "jive_Property.h"

namespace jive
{
    class Event : protected juce::ValueTree::Listener
    {
    public:
        Event() = delete;

        Event(juce::ValueTree sourceState, const juce::Identifier& eventID)
            : id{ eventID }
            , event{ sourceState, eventID }
        {
            if (!event.exists())
                event = new Object{};

            auto callbacks = event.get()->getProperty("callbacks");

            callbacks.append(juce::var{
                [weakThis = juce::WeakReference{ this }](const juce::var::NativeFunctionArgs&) {
                    if (weakThis != nullptr && weakThis->onTrigger != nullptr)
                        weakThis->onTrigger();

                    return juce::var{};
                },
            });

            event.get()->setProperty("callbacks", callbacks);
        }

        Event(const Event& other)
            : event{ other.event }
        {
        }

        Event(Event&& other)
            : event{ std::move(other.event) }
        {
        }

        Event& operator=(const Event& other)
        {
            event = other.event.get();
            return *this;
        }

        Event& operator=(Event&& other) = delete;
        ~Event() = default;

        int getAssumedTriggerCount() const
        {
            if (!event.get()->hasProperty("count"))
                return 0;

            return static_cast<int>(event.get()->getProperty("count"));
        }

        juce::Time getTimeLastTriggered() const
        {
            if (!event.get()->hasProperty("time"))
                return juce::Time{};

            return juce::Time{
                static_cast<juce::int64>(event.get()->getProperty("time")),
            };
        }

        void trigger()
        {
            event
                .get()
                ->setProperty("count", getAssumedTriggerCount() + 1);
            event
                .get()
                ->setProperty("time", juce::Time::currentTimeMillis());

            for (auto& callback : *event.get()->getProperty("callbacks").getArray())
            {
                callback.getNativeFunction()(juce::var::NativeFunctionArgs{
                    callback,
                    nullptr,
                    0,
                });
            }
        }

        void triggerWithoutSelfCallback()
        {
            const auto callback = onTrigger;
            onTrigger = nullptr;

            juce::WeakReference weakThis{ this };
            trigger();

            if (weakThis != nullptr)
                weakThis->onTrigger = callback;
        }

        const juce::Identifier id;

        std::function<void()> onTrigger = nullptr;

    private:
        Property<Object::ReferenceCountedPointer> event;

        JUCE_DECLARE_WEAK_REFERENCEABLE(Event)
    };
} // namespace jive
