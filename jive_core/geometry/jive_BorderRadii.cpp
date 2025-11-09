#include "jive_BorderRadii.h"

#include <jive_core/graphics/jive_Styles.h>
#include <jive_core/interface/jive_InteractionState.h>

namespace jive
{
    [[nodiscard]] static std::optional<BorderRadii<float>> findBorderRadius(const juce::Component& component,
                                                                            const Styles& styles)
    {
        static constexpr auto propertyName = "border-radius";
        const InteractionState state{ component };

        if (const auto property = state.findMostApplicable(component.getProperties(),
                                                           propertyName);
            property.has_value())
        {
            return juce::VariantConverter<BorderRadii<float>>::fromVar(*property);
        }

        return styles.borderRadius;
    }

    BorderRadii<float> getBorderRadius(const juce::Component& component,
                                       const Styles& styles,
                                       BorderRadii<float> defaultRadius)
    {
        static constexpr auto propertyName = "border-radius";
        const InteractionState state{ component };

        if (const auto property = state.findMostApplicable(component.getProperties(),
                                                           propertyName);
            property.has_value())
        {
            return juce::VariantConverter<BorderRadii<float>>::fromVar(*property);
        }

        return styles.borderRadius.value_or(defaultRadius);
    }

    BorderRadii<float> getBorderRadius(const juce::Button& button,
                                       const Styles& styles)
    {
        return findBorderRadius(button, styles)
            .value_or(BorderRadii<float>{
                button.isConnectedOnLeft() || button.isConnectedOnTop() ? 0.0f : 6.0f,
                button.isConnectedOnRight() || button.isConnectedOnTop() ? 0.0f : 6.0f,
                button.isConnectedOnRight() || button.isConnectedOnBottom() ? 0.0f : 6.0f,
                button.isConnectedOnLeft() || button.isConnectedOnBottom() ? 0.0f : 6.0f,
            });
    }

    BorderRadii<float> getBorderRadius(const juce::PopupMenu&,
                                       const Styles& styles)
    {
        return styles.borderRadius.value_or(BorderRadii{ 0.0f });
    }

    BorderRadii<float> getBorderRadius(const juce::PopupMenu::Item&,
                                       const Styles& styles)
    {
        return styles.borderRadius.value_or(BorderRadii{ 0.0f });
    }

    BorderRadii<float> getBorderRadius(const juce::ComboBox& comboBox,
                                       const Styles& styles)
    {
        return findBorderRadius(comboBox, styles)
            .value_or(BorderRadii{ 3.0f });
    }

    BorderRadii<float> getBorderRadius(const juce::GroupComponent& group,
                                       const Styles& styles)
    {
        return findBorderRadius(group, styles)
            .value_or(BorderRadii{ 5.0f });
    }

    BorderRadii<float> getBorderRadius(const juce::Slider& slider,
                                       const Styles& styles)
    {
        return findBorderRadius(slider, styles)
            .value_or(BorderRadii{ 3.0f });
    }

    BorderRadii<float> getBorderRadius(const juce::ProgressBar& bar,
                                       const Styles& styles)
    {
        return findBorderRadius(bar, styles)
            .value_or(BorderRadii{
                static_cast<float>(juce::jmin(bar.getWidth(), bar.getHeight())) / 2.0f,
            });
    }
} // namespace jive
