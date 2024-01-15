#pragma once

#include "jive_Easing.h"

#include <jive_core/algorithms/jive_Interpolate.h>
#include <jive_core/time/jive_Timer.h>

namespace jive
{
    class Transitions : public juce::ReferenceCountedObject
    {
    public:
        using ReferenceCountedPointer = juce::ReferenceCountedObjectPtr<Transitions>;

        struct Transition
        {
            template <typename Interpolatable>
            [[nodiscard]] Interpolatable calculateCurrent(const Interpolatable& source,
                                                          const Interpolatable& target,
                                                          juce::Time commencement) const
            {
                const auto elapsed = now() - commencement;

                if (elapsed < delay)
                    return source;
                if (elapsed > delay + duration)
                    return target;

                const auto proportionLinear = elapsed.inSeconds() / duration.inSeconds();
                const auto proportionEased = timingFunction(proportionLinear);

                return interpolate(source, target, proportionEased);
            }

            [[nodiscard]] static std::optional<Transition> fromString(const juce::String& transitionString);

            juce::RelativeTime duration;
            Easing timingFunction = easing::linear;
            juce::RelativeTime delay;
        };

        [[nodiscard]] int size() const;

        [[nodiscard]] Transition& operator[](const juce::String& propertyName);

        [[nodiscard]] static ReferenceCountedPointer fromString(const juce::String& s);

    private:
        std::unordered_map<juce::String, Transition> transitions;
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
