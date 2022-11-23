#pragma once

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
    class TypedValue : protected juce::ValueTree::Listener
    {
    public:
        //==============================================================================================================
        using Converter = juce::VariantConverter<ValueType>;

        //==============================================================================================================
        TypedValue(juce::ValueTree sourceTree,
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
        }

        TypedValue(juce::ValueTree sourceTree,
                   const juce::Identifier& propertyID,
                   const ValueType& initialValue)
            : TypedValue{ sourceTree, propertyID }
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
            if (!exists())
                return valueIfNotExists;

            return get();
        }

        void clear()
        {
            tree.removeProperty(id, nullptr);
        }

        bool exists() const
        {
            return tree.hasProperty(id);
        }

        //==============================================================================================================
        operator ValueType() const
        {
            return get();
        }

        TypedValue<ValueType>& operator=(const ValueType& newValue)
        {
            tree.setProperty(id, Converter::toVar(newValue), nullptr);
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

            while (!treeToSearch.hasProperty(id))
                treeToSearch = treeToSearch.getParent();

            return treeToSearch[id];
        }

        //==============================================================================================================
        juce::ValueTree treeToListenTo;
        juce::ValueTree tree;
    };
} // namespace jive
