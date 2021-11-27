#pragma once

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    template <typename ValueType>
    class TypedValue : private juce::Value::Listener
    {
    public:
        using VariantConverter = juce::VariantConverter<ValueType>;

        TypedValue(juce::ValueTree& tree,
                   const juce::Identifier& propertyID,
                   const ValueType& initialValue = ValueType{})
            : value{ tree.getPropertyAsValue(propertyID, nullptr, true) }
        {
            if (!tree.hasProperty(propertyID))
                value = VariantConverter::toVar(initialValue);

            value.addListener(this);
        }

        ValueType get() const
        {
            return VariantConverter::fromVar(value);
        }

        operator ValueType() const
        {
            return get();
        }

        TypedValue<ValueType>& operator=(const ValueType& newValue)
        {
            value = newValue;
            return *this;
        }

        std::function<void(void)> onValueChange = nullptr;

    private:
        void valueChanged(juce::Value& valueThatChanged) override
        {
            jassert(valueThatChanged.refersToSameSourceAs(value));

            if (onValueChange != nullptr)
                onValueChange();
        }

        juce::Value value;
    };
} // namespace jive
