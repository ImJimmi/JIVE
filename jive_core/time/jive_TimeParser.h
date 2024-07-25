#pragma once

#include <regex>

namespace jive
{
    [[nodiscard]] static auto isValidTimeString(const juce::String& str)
    {
        static const std::regex timeDataTypeRegex{ R"(^(\d+(\.\d+)?)(s|ms)$)" };
        return std::regex_match(str.toRawUTF8(), timeDataTypeRegex);
    }

    [[nodiscard, maybe_unused]] static std::optional<juce::RelativeTime> parseTime(const juce::String& timeString)
    {
        if (!isValidTimeString(timeString))
            return std::nullopt;

        if (timeString.endsWith("ms"))
            return juce::RelativeTime::milliseconds(juce::roundToInt(timeString.getDoubleValue()));

        if (timeString.endsWith("s"))
            return juce::RelativeTime::seconds(timeString.getDoubleValue());

        return std::nullopt;
    }
} // namespace jive
