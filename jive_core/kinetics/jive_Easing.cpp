#include "jive_Easing.h"

namespace jive::easing
{
    std::optional<Easing> fromString(const juce::String& name)
    {
        static const std::unordered_map<juce::String, Easing> namedEasings{
            { "linear", linear },
            { "ease", inOut },
            { "ease-in", in },
            { "ease-out", out },
            { "ease-in-out", inOut },
        };

        if (const auto result = namedEasings.find(name);
            result != std::end(namedEasings))
            return result->second;

        if (name.startsWith("cubic-bezier(") && name.endsWith(")"))
        {
            static constexpr auto includeSubStringsInResults = false;
            static constexpr auto ignoreCase = true;
            const auto args = juce::StringArray::fromTokens(name
                                                                .fromFirstOccurrenceOf("(", includeSubStringsInResults, ignoreCase)
                                                                .upToLastOccurrenceOf(")", includeSubStringsInResults, ignoreCase),
                                                            ",",
                                                            "");

            if (args.size() == 4)
            {
                const juce::Point startControlPoint{
                    static_cast<float>(args[0].getDoubleValue()),
                    static_cast<float>(args[1].getDoubleValue()),
                };
                const juce::Point endControlPoint{
                    static_cast<float>(args[2].getDoubleValue()),
                    static_cast<float>(args[3].getDoubleValue()),
                };

                return [startControlPoint, endControlPoint](double x) {
                    static const auto transferFunction = jive::cubicBezier(startControlPoint, endControlPoint);
                    return solveFor(transferFunction, x);
                };
            }
        }

        return std::nullopt;
    }
} // namespace jive::easing
