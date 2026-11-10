#pragma once

#include <juce_core/juce_core.h>

namespace jive
{
    template <typename Value, typename = void>
    struct hasVariantValidator : std::false_type
    {
    };

    template <typename Value>
    struct hasVariantValidator<Value,
                               std::void_t<decltype(juce::VariantConverter<Value>::isValid(std::declval<const juce::var&>()))>>
        : std::true_type
    {
    };

    template <typename Value, typename = void>
    struct hasVariantDefault : std::false_type
    {
    };

    template <typename Value>
    struct hasVariantDefault<Value,
                             std::void_t<decltype(juce::VariantConverter<Value>::defaultValue())>>
        : std::is_convertible<decltype(juce::VariantConverter<Value>::defaultValue()), Value>
    {
    };

    template <typename Value, typename = void>
    struct hasVariantSyntax : std::false_type
    {
    };

    template <typename Value>
    struct hasVariantSyntax<Value,
                            std::void_t<decltype(juce::VariantConverter<Value>::getSyntax())>>
        : std::is_convertible<decltype(juce::VariantConverter<Value>::getSyntax()), juce::String>
    {
    };

    template <typename Value>
    [[nodiscard]] bool isValidVar(const juce::var& value)
    {
        if constexpr (hasVariantValidator<Value>())
            return juce::VariantConverter<Value>::isValid(value);
        else
            return true;
    }

    [[nodiscard]] inline bool isNumber(const juce::String& text,
                                       const juce::StringArray& allowedSuffixes = {})
    {
        auto number = text.trim();

        for (const auto& suffix : allowedSuffixes)
        {
            if (number.endsWithIgnoreCase(suffix))
            {
                number = number.dropLastCharacters(suffix.length());
                break;
            }
        }

        if (number.isEmpty())
            return false;

        const auto* start = number.toRawUTF8();
        char* end = nullptr;
        std::strtod(start, &end);

        return end != start && *end == '\0';
    }

    [[nodiscard]] inline bool isNumber(const juce::var& value,
                                       const juce::StringArray& allowedSuffixes = {})
    {
        if (value.isInt() || value.isInt64() || value.isDouble())
            return true;

        return value.isString() && isNumber(value.toString(), allowedSuffixes);
    }

    [[nodiscard]] inline bool areAllNumbers(const juce::var& value,
                                            int minTokens,
                                            int maxTokens,
                                            const juce::StringArray& allowedSuffixes = {})
    {
        if (!value.isString())
            return minTokens <= 1 && isNumber(value, allowedSuffixes);

        auto tokens = juce::StringArray::fromTokens(value.toString(), " ,", "");
        tokens.removeEmptyStrings();

        return juce::isPositiveAndNotGreaterThan(tokens.size() - minTokens, maxTokens - minTokens)
            && std::all_of(std::begin(tokens),
                           std::end(tokens),
                           [&allowedSuffixes](const auto& token) {
                               return isNumber(token, allowedSuffixes);
                           });
    }

    template <typename Options>
    [[nodiscard]] bool isOneOf(const juce::var& value, const Options& options)
    {
        if (value.isVoid())
            return true;

        return std::any_of(std::begin(options),
                           std::end(options),
                           [text = value.toString()](const auto& option) {
                               if constexpr (std::is_convertible_v<decltype(option), juce::var>)
                                   return juce::var{ option }.toString() == text;
                               else
                                   return option.first == text;
                           });
    }

    template <typename Options>
    [[nodiscard]] juce::String oneOfSyntax(const Options& options,
                                           const juce::StringArray& extraOptions = {})
    {
        juce::StringArray names{ extraOptions };

        for (const auto& option : options)
        {
            if constexpr (std::is_convertible_v<decltype(option), juce::var>)
                names.addIfNotAlreadyThere("\"" + juce::var{ option }.toString() + "\"");
            else
                names.addIfNotAlreadyThere("\"" + option.first + "\"");
        }

        if constexpr (!std::is_convertible_v<decltype(*std::begin(options)), juce::var>)
            names.sortNatural();

        return "one of " + names.joinIntoString(", ");
    }
} // namespace jive
