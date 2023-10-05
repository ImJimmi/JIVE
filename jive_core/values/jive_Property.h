#pragma once

#include "jive_Object.h"

namespace jive
{
    enum class Inheritance
    {
        inheritFromParent,
        inheritFromAncestors,
        doNotInherit,
    };

    enum class Accumulation
    {
        accumulate,
        doNotAccumulate,
    };

    template <typename ValueType,
              Inheritance inheritance = Inheritance::doNotInherit,
              Accumulation accumulation = Accumulation::doNotAccumulate>
    class Property : protected juce::ValueTree::Listener
    {
    public:
        using Converter = juce::VariantConverter<ValueType>;

        Property(juce::ValueTree sourceTree,
                 const juce::Identifier& propertyID)
            : id{ propertyID }
            , tree{ sourceTree }
        {
            switch (inheritance)
            {
            case Inheritance::inheritFromParent:
                treeToListenTo = tree.getParent();
                break;
            case Inheritance::inheritFromAncestors:
                treeToListenTo = tree.getRoot();
                break;
            case Inheritance::doNotInherit:
                treeToListenTo = tree;
            }

            if (!treeToListenTo.isValid())
                treeToListenTo = tree;

            treeToListenTo.addListener(this);

            if constexpr (std::is_same<ValueType, Object::ReferenceCountedPointer>())
            {
                if (tree[id].isString())
                    tree.setProperty(id, parseJSON(tree[id].toString()), nullptr);
            }
        }

        Property(const Property& other) = delete;
        Property(Property&& other) = delete;
        Property& operator=(const Property& other) = delete;
        Property& operator=(Property&& other) = delete;
        ~Property() override = default;

        [[nodiscard]] virtual ValueType get() const
        {
            return getFrom(getRootOfInheritance());
        }

        [[nodiscard]] auto getOr(const ValueType& valueIfNoneSpecified) const
        {
            if (auto root = getRootOfInheritance();
                root.isValid())
            {
                return getFrom(root);
            }

            if constexpr (accumulation == Accumulation::accumulate)
            {
                if (auto source = getFirstAncestorWithProperty(tree);
                    source.isValid())
                {
                    return getFrom(tree);
                }
            }

            return valueIfNoneSpecified;
        }

        void set(const ValueType& newValue)
        {
            tree.setProperty(id, Converter::toVar(newValue), nullptr);
        }

        void set(std::function<ValueType()> function)
        {
            const auto nativeFunction = [function](const auto&) {
                return Converter::toVar(function());
            };
            tree.setProperty(id, juce::var{ nativeFunction }, nullptr);
        }

        void setAuto()
        {
            tree.setProperty(id, "auto", nullptr);
        }

        void clear()
        {
            tree.removeProperty(id, nullptr);
        }

        [[nodiscard]] auto exists() const
        {
            return tree.hasProperty(id);
        }

        [[nodiscard]] auto isAuto() const
        {
            return (!exists()) || tree[id].toString().trim().equalsIgnoreCase("auto");
        }

        [[nodiscard]] auto isFunctional() const
        {
            return exists() && tree[id].isMethod();
        }

        [[nodiscard]] auto toString() const
        {
            if (!exists())
                return juce::String{};

            return tree[id].toString();
        }

        [[nodiscard]] operator ValueType() const
        {
            return get();
        }

        auto& operator=(const ValueType& newValue)
        {
            set(newValue);
            return *this;
        }

        auto& operator=(std::function<ValueType()> function)
        {
            set(function);
            return *this;
        }

        auto& operator=(std::function<ValueType(std::vector<juce::var>)> function)
        {
            set(function);
            return *this;
        }

        const juce::Identifier id;
        std::function<void(void)> onValueChange = nullptr;

    protected:
        void valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyChanged,
                                      const juce::Identifier& property) override
        {
            if (property != id)
                return;
            if (!treeWhosePropertyChanged.hasProperty(property))
                return;
            if (!respondToPropertyChanges(treeWhosePropertyChanged))
                return;

            if (onValueChange != nullptr)
                onValueChange();
        }

        [[nodiscard]] auto respondToPropertyChanges(juce::ValueTree& treeWhosePropertyChanged) const
        {
            if (treeWhosePropertyChanged == tree)
                return true;

            switch (inheritance)
            {
            case Inheritance::inheritFromParent:
                return treeWhosePropertyChanged == tree.getParent();
            case Inheritance::inheritFromAncestors:
                return tree.isAChildOf(treeWhosePropertyChanged);
            case Inheritance::doNotInherit:
                break;
            }

            switch (accumulation)
            {
            case Accumulation::accumulate:
                return treeWhosePropertyChanged.isAChildOf(tree);
            case Accumulation::doNotAccumulate:
                break;
            }

            return false;
        }

        [[nodiscard]] auto getRootOfInheritance() const
        {
            if (exists() || accumulation == Accumulation::accumulate)
                return tree;

            if constexpr (inheritance == Inheritance::inheritFromParent)
            {
                if (auto parent = tree.getParent();
                    parent.hasProperty(id))
                {
                    return parent;
                }
            }
            if constexpr (inheritance == Inheritance::inheritFromAncestors)
            {
                for (auto ancestor = tree.getParent();
                     ancestor.isValid();
                     ancestor = ancestor.getParent())
                {
                    if (ancestor.hasProperty(id))
                        return ancestor;
                }
            }

            return juce::ValueTree{};
        }

        [[nodiscard]] auto getFirstAncestorWithProperty(const juce::ValueTree& root) const
        {
            for (const auto& child : root)
            {
                if (child.hasProperty(id))
                    return child;
            }

            for (const auto& child : root)
            {
                if (auto ancestor = getFirstAncestorWithProperty(child);
                    ancestor.isValid())
                {
                    return ancestor;
                }
            }

            return juce::ValueTree{};
        }

        [[nodiscard]] ValueType getFrom(const juce::ValueTree& root) const
        {
            if constexpr (accumulation == Accumulation::accumulate)
            {
                auto result = Converter::fromVar(root[id]);

                for (const auto& child : root)
                    result += getFrom(child);

                return result;
            }
            else
            {
                auto var = root.hasProperty(id) ? root[id] : getFirstAncestorWithProperty(root).getProperty(id, juce::var{});

                if (var.isMethod())
                {
                    juce::var::NativeFunctionArgs args{ var, nullptr, 0 };
                    var = var.getNativeFunction()(args);
                }

                return Converter::fromVar(var);
            }
        }

        juce::ValueTree treeToListenTo;
        juce::ValueTree tree;
    };
} // namespace jive
