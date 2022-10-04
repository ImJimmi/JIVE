#pragma once

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    template <typename T>
    class AutoValue
    {
    public:
        //==============================================================================================================
        AutoValue() = default;

        AutoValue(const AutoValue& other)
            : hasExplicitValue{ other.hasExplicitValue }
            , value{ other.value }
        {
        }

        AutoValue& operator=(const AutoValue& other)
        {
            hasExplicitValue = other.hasExplicitValue;
            value = other.value;

            return *this;
        }

        AutoValue(AutoValue&& other)
            : hasExplicitValue{ other.hasExplicitValue }
            , value{ std::move(other.value) }
        {
        }

        AutoValue& operator=(AutoValue&& other)
        {
            hasExplicitValue = other.hasExplicitValue;
            value = std::move(other.value);

            return *this;
        }

        explicit AutoValue(const T& value)
        {
            set(value);
        }

        AutoValue& operator=(const T& value)
        {
            set(value);
            return *this;
        }

        ~AutoValue() = default;

        //==============================================================================================================
        bool isAuto() const
        {
            return !hasExplicitValue;
        }

        void set(const T& newValue)
        {
            value = newValue;
            hasExplicitValue = true;
        }

        T getWithDefault(const T& defaultValue = {}) const
        {
            if (isAuto())
                return defaultValue;

            return value;
        }

        void clear()
        {
            hasExplicitValue = false;
        }

    private:
        //==============================================================================================================
        bool hasExplicitValue{ false };
        T value;
    };
} // namespace jive

//======================================================================================================================
namespace juce
{
    //==================================================================================================================
    template <typename T>
    struct VariantConverter<jive::AutoValue<T>>
    {
        //==============================================================================================================
        static jive::AutoValue<T> fromVar(const var& v)
        {
            if (v.isVoid() || v.toString().compareIgnoreCase("auto") == 0)
                return jive::AutoValue<T>{};

            return jive::AutoValue<T>{ VariantConverter<T>::fromVar(v) };
        }

        static var toVar(const jive::AutoValue<T>& value)
        {
            if (value.isAuto())
                return "auto";

            return VariantConverter<T>::toVar(*value.get());
        }
    };
} // namespace juce
