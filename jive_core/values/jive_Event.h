#pragma once

#include <cstdint>

#include "jive_Property.h"

namespace jive
{
    class Event : protected juce::ValueTree::Listener
    {
    public:
        Event() = delete;

        Event(juce::ValueTree sourceState,
              juce::Identifier eventID,
              std::function<void()> onEventTrigger = nullptr)
            : id{ eventID }
            , onTrigger{ std::move(onEventTrigger) }
            , event{ sourceState, eventID }
        {
            if (!event.exists())
                event = new Object{}; // NOLINT

            auto callbacks = event.get()->getProperty(ids::callbacks());

            callbacks.append(juce::var{
                [weakThis = juce::WeakReference{ this }](const juce::var::NativeFunctionArgs&) {
                    if (weakThis != nullptr && weakThis->onTrigger != nullptr)
                        weakThis->onTrigger();

                    return juce::var{};
                },
            });

            event.get()->setProperty(ids::callbacks(), callbacks);
        }

        Event(const Event& other)
            : event{ other.event }
        {
        }

        Event(Event&& other) noexcept : event{ std::move(other.event) }
        {
        }

        Event& operator=(const Event& other)
        {
            if (&other != this)
                event = other.event.get();

            return *this;
        }

        Event& operator=(Event&& other) = delete;
        ~Event() override = default;

        int getAssumedTriggerCount() const
        {
            if (!event.get()->hasProperty(ids::count()))
                return 0;

            return static_cast<int>(event.get()->getProperty(ids::count()));
        }

        juce::Time getTimeLastTriggered() const
        {
            if (!event.get()->hasProperty(ids::time()))
                return juce::Time{};

            return juce::Time{
                static_cast<std::int64_t>(event.get()->getProperty(ids::time())),
            };
        }

        void trigger()
        {
            event
                .get()
                ->setProperty(ids::count(), getAssumedTriggerCount() + 1);
            event
                .get()
                ->setProperty(ids::time(), juce::Time::currentTimeMillis());

            if (const auto* callbacks = event.get()->getProperty(ids::callbacks()).getArray())
            {
                for (const auto& callback : *callbacks)
                {
                    callback.getNativeFunction()(juce::var::NativeFunctionArgs{
                        callback,
                        nullptr,
                        0,
                    });
                }
            }
        }

        void triggerWithoutSelfCallback()
        {
            const auto callback = onTrigger;
            onTrigger = nullptr;

            const juce::WeakReference weakThis{ this };
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
