#include "jive_BorderRadii.h"

#include "jive_BorderWidth.h"

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

        return styles.find<BorderRadii<float>>("border-radius");
    }

    BorderRadii<float> getBorderRadius(const juce::Component& component,
                                       const Styles& styles,
                                       BorderRadii<float> defaultRadius)
    {
        return findBorderRadius(component, styles)
            .value_or(defaultRadius);
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
        return styles
            .find<BorderRadii<float>>("border-radius")
            .value_or(BorderRadii{ 0.0f });
    }

    // Items are rounded to match the inside of their popup's border.
    BorderRadii<float> getBorderRadius(const juce::PopupMenu::Item&,
                                       const juce::PopupMenu& popup,
                                       const Styles& styles)
    {
        const auto popupRadius = getBorderRadius(popup, styles);
        const auto popupBorderWidth = getBorderWidth(popup, styles);

        return BorderRadii{
            std::max(0.0f, popupRadius.topLeft - std::max(popupBorderWidth.getTop(), popupBorderWidth.getLeft())),
            std::max(0.0f, popupRadius.topRight - std::max(popupBorderWidth.getTop(), popupBorderWidth.getRight())),
            std::max(0.0f, popupRadius.bottomRight - std::max(popupBorderWidth.getBottom(), popupBorderWidth.getRight())),
            std::max(0.0f, popupRadius.bottomLeft - std::max(popupBorderWidth.getBottom(), popupBorderWidth.getLeft())),
        };
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
                static_cast<float>(std::min(bar.getWidth(), bar.getHeight())) / 2.0f,
            });
    }
} // namespace jive
