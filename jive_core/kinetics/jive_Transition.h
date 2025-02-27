#pragma once

#include "jive_Easing.h"

#include <jive_core/algorithms/jive_Interpolate.h>
#include <jive_core/values/jive_PropertyBehaviours.h>

namespace jive
{
    class Transition
    {
    public:
        struct Listener
        {
            virtual ~Listener() = default;

            virtual void transitionProgressed(const juce::String& propertyName,
                                              const Transition& transition) = 0;
        };

        Transition() = default;

        Transition(const Transition& other);
        Transition& operator=(const Transition& other);

        [[nodiscard]] double calculateProgress(juce::Time commencementTime) const;
        [[nodiscard]] double calculateProgress() const;

        template <typename Value>
        [[nodiscard]] Value calculateCurrent(const Value& sourceValue,
                                             const Value& targetValue,
                                             juce::Time commencementTime) const
        {
            const auto progress = calculateProgress(commencementTime);

            if (progress <= 0.0)
                return sourceValue;
            if (progress >= 1.0)
                return targetValue;

            return interpolate(sourceValue, targetValue, progress);
        }

        template <typename Value>
        [[nodiscard]] Value calculateCurrent() const
        {
            jassert(!source.isVoid() && !target.isVoid());

            using Converter = juce::VariantConverter<Value>;
            return calculateCurrent(Converter::fromVar(source),
                                    Converter::fromVar(target),
                                    commencement);
        }

        void addListener(Listener& listener) const;
        void removeListener(Listener& listener) const;

        [[nodiscard]] static std::optional<Transition> fromString(const juce::String& transitionString);

        juce::RelativeTime duration;
        Easing timingFunction = easing::linear;
        juce::RelativeTime delay;

    private:
        template <typename ValueType, Inheritance, Accumulation, bool, Responsiveness>
        friend class Property;
        friend class Transitions;

        juce::Time commencement;
        juce::var source;
        juce::var target;
        double cachedProgress{ -1.0 };
        mutable juce::ListenerList<Listener> listeners;
    };
} // namespace jive
