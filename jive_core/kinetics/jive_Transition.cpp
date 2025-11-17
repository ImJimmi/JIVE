#include "jive_Transition.h"

#include <jive_core/time/jive_TimeParser.h>
#include <jive_core/time/jive_Timer.h>

namespace jive
{
    Transition::Transition(const Transition& other)
        : duration{ other.duration }
        , timingFunction{ other.timingFunction }
        , delay{ other.delay }
        , commencement{ other.commencement }
        , source{ other.source }
        , target{ other.target }
        , cachedProgress{ other.cachedProgress }
    {
    }

    Transition& Transition::operator=(const Transition& other)
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

    double Transition::calculateProgress(juce::Time commencementTime) const
    {
        if (duration <= juce::RelativeTime::seconds(0.0))
            return 1.0;

        const auto elapsed = now() - commencementTime - delay;
        const auto progressLinear = elapsed.inSeconds() / duration.inSeconds();

        if (progressLinear >= 0.0 && progressLinear <= 1.0)
            return timingFunction(progressLinear);

        return juce::jlimit(0.0, 1.0, progressLinear);
    }

    double Transition::calculateProgress() const
    {
        return calculateProgress(commencement);
    }

    void Transition::addListener(Listener& listener) const
    {
        listeners.add(&listener);
    }

    void Transition::removeListener(Listener& listener) const
    {
        listeners.remove(&listener);
    }

    std::optional<Transition> Transition::fromString(const juce::String& transitionString)
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
} // namespace jive
