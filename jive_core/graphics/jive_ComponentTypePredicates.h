#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace jive
{
    class ComponentTypePredicates : public juce::DeletedAtShutdown
    {
    public:
        using Predicate = std::function<bool(const juce::Component&, const juce::String&)>;

        // NOLINTNEXTLINE(juce-plugin-global-state)
        JUCE_DECLARE_SINGLETON_INLINE(ComponentTypePredicates, false)

        ComponentTypePredicates();
        JUCE_DECLARE_NON_COPYABLE(ComponentTypePredicates)
        JUCE_DECLARE_NON_MOVEABLE(ComponentTypePredicates)
        ~ComponentTypePredicates() override;

        static auto addPredicate(const Predicate& predicate)
        {
            juce::Uuid uuid;
            getInstance()->predicates.emplace(uuid, predicate);
            return uuid;
        }

        [[nodiscard]] static auto check(const juce::Component& component,
                                        const juce::String& typeName)
        {
            auto& predicates = getInstance()->predicates;
            return std::any_of(std::begin(predicates),
                               std::end(predicates),
                               [&component, &typeName](auto&& entry) {
                                   return entry.second(component, typeName);
                               });
        }

    private:
        std::unordered_map<juce::Uuid, Predicate> predicates;
    };
} // namespace jive
