#include "jive_ComponentTypePredicates.h"

#include <jive_style_sheets/jive_style_sheets.h>

namespace jive
{
    // Components that no element creates, such as a viewport's scroll bars,
    // have no element name, so are matched by their type instead.
    template <typename ComponentType>
    void addInternalComponent(std::unordered_map<juce::Uuid, ComponentTypePredicates::Predicate>& predicates,
                              const juce::String& expectedTypeName)
    {
        predicates.emplace(juce::Uuid{},
                           [expectedTypeName](const juce::Component& component,
                                              const juce::String& typeName) {
                               return typeName == expectedTypeName
                                   && !component.getProperties().contains(ids::type())
                                   && dynamic_cast<const ComponentType*>(&component) != nullptr;
                           });
    }

    ComponentTypePredicates::ComponentTypePredicates()
    {
        predicates.emplace(juce::Uuid{},
                           [](const juce::Component& component, const juce::String& typeName) {
                               return component.getProperties()[ids::type()].toString() == typeName;
                           });

        addInternalComponent<juce::ScrollBar>(predicates, "ScrollBar");
        addInternalComponent<juce::TextEditor>(predicates, "TextEditor");
    }

    ComponentTypePredicates::~ComponentTypePredicates()
    {
        clearSingletonInstance();
    }

    JUCE_IMPLEMENT_SINGLETON(ComponentTypePredicates)
} // namespace jive
