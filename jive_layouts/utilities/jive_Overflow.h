#pragma once

#include <jive_core/values/variant-converters/jive_VariantValidation.h>

#include <juce_data_structures/juce_data_structures.h>

namespace jive
{
    enum class Overflow
    {
        hidden,
        scroll,
    };
} // namespace jive

namespace juce
{
    template <>
    struct VariantConverter<jive::Overflow>
    {
    public:
        static var toVar(const jive::Overflow& overflow)
        {
            jassert(stringsMap.count(overflow) == 1);
            return stringsMap.at(overflow);
        }

        static jive::Overflow fromVar(const var& v)
        {
            auto overflowStringPair = std::find_if(std::begin(stringsMap),
                                                   std::end(stringsMap),
                                                   [v](const auto& pair) {
                                                       return pair.second == v.toString();
                                                   });
            jassert(overflowStringPair != std::end(stringsMap));

            return overflowStringPair->first;
        }

        static bool isValid(const var& v)
        {
            return v.isVoid()
                || std::any_of(std::begin(stringsMap),
                               std::end(stringsMap),
                               [v](const auto& pair) {
                                   return pair.second == v.toString();
                               });
        }

        static String getSyntax()
        {
            StringArray options;

            for (const auto& [overflow, name] : stringsMap)
                options.add(name);

            options.sortNatural();

            return jive::oneOfSyntax(options);
        }

    private:
        static const std::unordered_map<jive::Overflow, String> stringsMap;
    };
} // namespace juce
