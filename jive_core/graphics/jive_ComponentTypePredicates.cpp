#include "jive_ComponentTypePredicates.h"

#include <jive_core/jive_core.h>

namespace jive
{
    template <typename ComponentType>
    void addStandardComponent(std::unordered_map<juce::Uuid, ComponentTypePredicates::Predicate>& predicates,
                              const juce::String& expectedTypeName)
    {
        predicates.emplace(juce::Uuid{},
                           [expectedTypeName](const juce::Component& component,
                                              const juce::String& typeName) {
                               return typeName == expectedTypeName
                                   && dynamic_cast<const ComponentType*>(&component) != nullptr;
                           });
    }

    static void addSlider(std::unordered_map<juce::Uuid, ComponentTypePredicates::Predicate>& predicates,
                          const juce::String& expectedTypeName,
                          const std::function<bool(const juce::Slider&)>& additionalCheck)
    {
        predicates.emplace(juce::Uuid{},
                           [expectedTypeName, additionalCheck](const juce::Component& component,
                                                               const juce::String& typeName) {
                               const auto* slider = dynamic_cast<const juce::Slider*>(&component);

                               return typeName == expectedTypeName
                                   && slider != nullptr && additionalCheck(*slider);
                           });
    }

    ComponentTypePredicates::ComponentTypePredicates()
    {
        addStandardComponent<juce::Button>(predicates, "Button");
        addStandardComponent<juce::ToggleButton>(predicates, "Checkbox");
        addStandardComponent<juce::ComboBox>(predicates, "ComboBox");
#if JIVE_IS_PLUGIN_PROJECT
        addStandardComponent<juce::AudioProcessorEditor>(predicates, "Editor");
#endif
        addStandardComponent<juce::HyperlinkButton>(predicates, "Hyperlink");
        addStandardComponent<juce::Label>(predicates, "Label");
        addStandardComponent<juce::ProgressBar>(predicates, "ProgressBar");
        addStandardComponent<juce::TopLevelWindow>(predicates, "Window");

        addSlider(predicates, "Knob", [](const juce::Slider& slider) {
            return slider.isRotary();
        });
        addSlider(predicates, "Slider", [](const juce::Slider& slider) {
            return !slider.isRotary() && slider.getSliderStyle() != juce::Slider::IncDecButtons;
        });
        addSlider(predicates, "Spinner", [](const juce::Slider& slider) {
            return slider.getSliderStyle() == juce::Slider::IncDecButtons;
        });
    }

    ComponentTypePredicates::~ComponentTypePredicates()
    {
        clearSingletonInstance();
    }
} // namespace jive
