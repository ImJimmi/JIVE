#pragma once

#include "jive_Easing.h"

#include <jive_core/algorithms/jive_Interpolate.h>
#include <jive_core/time/jive_Timer.h>
#include <jive_core/values/jive_PropertyBehaviours.h>

namespace jive
{
    class Transitions : public juce::ReferenceCountedObject
    {
    public:
        using ReferenceCountedPointer = juce::ReferenceCountedObjectPtr<Transitions>;

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

            Transition(const Transition& other)
                : duration{ other.duration }
                , timingFunction{ other.timingFunction }
                , delay{ other.delay }
                , commencement{ other.commencement }
                , source{ other.source }
                , target{ other.target }
                , cachedProgress{ other.cachedProgress }
            {
            }

            auto& operator=(const Transition& other)
            {
                duration = other.duration;
                timingFunction = other.timingFunction;
                delay = other.delay;
                commencement = other.commencement;
                source = other.source;
                target = other.target;
                cachedProgress = other.cachedProgress;

                return *this;
            }

            [[nodiscard]] auto calculateProgress(juce::Time commencementTime) const
            {
                if (duration <= juce::RelativeTime::seconds(0.0))
                    return 1.0;

                const auto elapsed = now() - commencementTime - delay;
                const auto progressLinear = elapsed.inSeconds() / duration.inSeconds();

                if (progressLinear >= 0.0 && progressLinear <= 1.0)
                    return timingFunction(progressLinear);

                return progressLinear;
            }

            [[nodiscard]] auto calculateProgress() const
            {
                return calculateProgress(commencement);
            }

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

            void addListener(Listener& listener) const
            {
                listeners.add(&listener);
            }

            void removeListener(Listener& listener) const
            {
                listeners.remove(&listener);
            }

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

        [[nodiscard]] int size() const;

        [[nodiscard]] Transition* operator[](const juce::String& propertyName);

        [[nodiscard]] static ReferenceCountedPointer fromString(const juce::String& s);

    private:
        void updateTransitions()
        {
            for (auto& [propertyName, transition] : transitions)
            {
                const auto progress = transition.source == transition.target
                                        ? -1.0
                                        : transition.calculateProgress();
                const auto inProgress = transition.duration > juce::RelativeTime{}
                                     && progress >= 0.0 && progress <= 1.0;
                const auto wasInProgress = transition.cachedProgress >= 0.0 && transition.cachedProgress <= 1.0;
                const auto justFinished = progress >= 1.0 && wasInProgress;

                if (inProgress || justFinished)
                {
                    transition.listeners.call(&Transition::Listener::transitionProgressed, propertyName, transition);
                    transition.cachedProgress = progress;
                }
            }
        }

        std::unordered_map<juce::String, Transition> transitions;
        const Timer transitionsUpdater{
            std::bind(&Transitions::updateTransitions, this),
            juce::RelativeTime::seconds(1.0 / 60.0),
        };
    };
} // namespace jive

namespace juce
{
    template <>
    struct VariantConverter<jive::Transitions::ReferenceCountedPointer>
    {
        static jive::Transitions::ReferenceCountedPointer fromVar(const var& v)
        {
            if (auto* transitions = dynamic_cast<jive::Transitions*>(v.getObject()))
                return transitions;

            if (v.isString())
                return jive::Transitions::fromString(v.toString());

            return nullptr;
        }

        static var toVar(jive::Transitions::ReferenceCountedPointer transitions)
        {
            return var{ transitions };
        }
    };
} // namespace juce
