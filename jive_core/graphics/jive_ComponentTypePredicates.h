#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace jive
{
    class ComponentTypePredicates : public juce::DeletedAtShutdown
    {
    public:
        using Predicate = std::function<bool(const juce::Component&, const juce::String&)>;

        JUCE_DECLARE_SINGLETON_INLINE(ComponentTypePredicates, false)

        ComponentTypePredicates();
        ~ComponentTypePredicates();

        [[nodiscard]] static auto addPredicate(const Predicate& predicate)
        {
            juce::Uuid uuid;
            getInstance()->predicates.emplace(uuid, predicate);
            return uuid;
        }

        [[nodiscard]] static auto check(const juce::Component& component,
                                        const juce::String& typeName)
        {
            for (const auto& [uuid, hasMatchingType] : getInstance()->predicates)
            {
                juce::ignoreUnused(uuid);

                if (hasMatchingType(component, typeName))
                    return true;
            }

            return false;
        }

    private:
        std::unordered_map<juce::Uuid, Predicate> predicates;
    };
} // namespace jive
