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

        TypedValue(juce::ValueTree& sourceTree,
                   const juce::Identifier& propertyID)
            : id{ propertyID }
            , tree{ sourceTree }
            , value{ tree.getPropertyAsValue(id, nullptr, true) }
        {
            value.addListener(this);
        }

        TypedValue(juce::ValueTree& sourceTree,
                   const juce::Identifier& propertyID,
                   const ValueType& initialValue)
            : TypedValue{ sourceTree, propertyID }
        {
            if (!exists())
                value = VariantConverter::toVar(initialValue);
        }

        ValueType get() const
        {
            return VariantConverter::fromVar(value);
        }

        void clear()
        {
            tree.removeProperty(id, nullptr);
        }

        bool exists() const
        {
            return tree.hasProperty(id);
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

        const juce::Identifier id;
        std::function<void(void)> onValueChange = nullptr;

    private:
        void valueChanged(juce::Value& valueThatChanged) override
        {
            jassertquiet(valueThatChanged.refersToSameSourceAs(value));

            if (onValueChange != nullptr && exists())
                onValueChange();
        }

        juce::ValueTree tree;
        juce::Value value;
    };
} // namespace jive
