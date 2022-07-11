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
