#pragma once

namespace jive
{
    template <typename T>
    [[nodiscard]] auto toVar(const T& value)
    {
        return juce::VariantConverter<T>::toVar(value);
    }

    template <typename T>
    [[nodiscard]] auto toVar(const std::optional<T>& value)
    {
        if (value.has_value())
            return toVar(*value);

        return juce::var{};
    }

    template <typename T>
    [[nodiscard]] auto fromVar(const juce::var& value)
    {
        return juce::VariantConverter<T>::fromVar(value);
    }
} // namespace jive
