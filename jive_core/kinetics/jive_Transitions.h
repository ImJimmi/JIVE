#pragma once

#include "jive_Easing.h"
#include "jive_Transition.h"

#include <jive_core/algorithms/jive_Interpolate.h>
#include <jive_core/time/jive_Timer.h>
#include <jive_core/values/jive_PropertyBehaviours.h>

namespace jive
{
    class Transitions : public juce::ReferenceCountedObject
    {
    public:
        using ReferenceCountedPointer = juce::ReferenceCountedObjectPtr<Transitions>;

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
            return var{ transitions.get() };
        }
    };
} // namespace juce
