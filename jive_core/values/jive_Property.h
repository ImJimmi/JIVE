#pragma once

#include "jive_Object.h"

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    enum class HereditaryValueBehaviour
    {
        inheritFromParent,
        inheritFromAncestors,
        doNotInherit,
    };

    //==================================================================================================================
    template <typename ValueType,
              HereditaryValueBehaviour hereditaryBehavior = HereditaryValueBehaviour::doNotInherit>
    class Property : protected juce::ValueTree::Listener
    {
    public:
        //==============================================================================================================
        using Converter = juce::VariantConverter<ValueType>;

        //==============================================================================================================
        Property(juce::ValueTree sourceTree,
                 const juce::Identifier& propertyID)
            : id{ propertyID }
            , tree{ sourceTree }
        {
            switch (hereditaryBehavior)
            {
            case HereditaryValueBehaviour::inheritFromParent:
                treeToListenTo = tree.getParent();
                break;
            case HereditaryValueBehaviour::inheritFromAncestors:
                treeToListenTo = tree.getRoot();
                break;
            case HereditaryValueBehaviour::doNotInherit:
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

        Property(juce::ValueTree sourceTree,
                 const juce::Identifier& propertyID,
                 const ValueType& initialValue)
            : Property{ sourceTree, propertyID }
        {
            if (!exists())
                *this = initialValue;
        }

        //==============================================================================================================
        virtual ValueType get() const
        {
            if (exists())
                return Converter::fromVar(tree[id]);

            switch (hereditaryBehavior)
            {
            case HereditaryValueBehaviour::inheritFromParent:
                return Converter::fromVar(tree.getParent()[id]);
            case HereditaryValueBehaviour::inheritFromAncestors:
                return Converter::fromVar(findPropertyInLatestAncestor());
            case HereditaryValueBehaviour::doNotInherit:
                return Converter::fromVar(tree[id]);
            }

            jassertfalse;
            return Converter::fromVar(juce::var{});
        }

        ValueType getOr(const ValueType& valueIfNotExists) const
        {
            if (exists())
                return get();

            switch (hereditaryBehavior)
            {
            case HereditaryValueBehaviour::inheritFromParent:
            {
                if (auto value = tree.getParent()[id];
                    value != juce::var{})
                    return Converter::fromVar(value);
            }
            case HereditaryValueBehaviour::inheritFromAncestors:
            {
                if (auto value = findPropertyInLatestAncestor();
                    value != juce::var{})
                    return Converter::fromVar(value);
            }
            case HereditaryValueBehaviour::doNotInherit:
                return valueIfNotExists;
            }

            return valueIfNotExists;
        }

        void set(const ValueType& newValue)
        {
            tree.setProperty(id, Converter::toVar(newValue), nullptr);
        }

        void setAuto()
        {
            tree.setProperty(id, "auto", nullptr);
        }

        void clear()
        {
            tree.removeProperty(id, nullptr);
        }

        bool exists() const
        {
            return tree.hasProperty(id);
        }

        bool isAuto() const
        {
            return (!exists()) || tree[id].toString().trim().equalsIgnoreCase("auto");
        }

        juce::String toString() const
        {
            if (!exists())
                return "";

            return tree[id].toString();
        }

        //==============================================================================================================
        operator ValueType() const
        {
            return get();
        }

        Property<ValueType, hereditaryBehavior>& operator=(const ValueType& newValue)
        {
            set(newValue);
            return *this;
        }

        //==============================================================================================================
        const juce::Identifier id;
        std::function<void(void)> onValueChange = nullptr;

    protected:
        //==============================================================================================================
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
            {
                onValueChange();
            }
        }

        bool respondToPropertyChanges(juce::ValueTree& treeWhosePropertyChanged) const
        {
            if (treeWhosePropertyChanged == tree)
                return true;

            switch (hereditaryBehavior)
            {
            case HereditaryValueBehaviour::inheritFromParent:
                return treeWhosePropertyChanged == tree.getParent();
            case HereditaryValueBehaviour::inheritFromAncestors:
                return tree.isAChildOf(treeWhosePropertyChanged);
            case HereditaryValueBehaviour::doNotInherit:
                return treeWhosePropertyChanged == tree;
            }

            return false;
        }

        juce::var findPropertyInLatestAncestor() const
        {
            auto treeToSearch = tree.getParent();

            while (treeToSearch.isValid() && !treeToSearch.hasProperty(id))
                treeToSearch = treeToSearch.getParent();

            if (treeToSearch.isValid())
                return treeToSearch[id];

            return {};
        }

        //==============================================================================================================
        juce::ValueTree treeToListenTo;
        juce::ValueTree tree;
    };
} // namespace jive
