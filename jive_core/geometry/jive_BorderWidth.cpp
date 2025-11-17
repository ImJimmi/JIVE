#include "jive_BorderWidth.h"

#include <jive_core/interface/jive_InteractionState.h>

namespace jive
{
    [[nodiscard]] static std::optional<juce::BorderSize<float>> findBorderWidth(const juce::Component& component,
                                                                                const Styles& styles)
    {
        static constexpr auto propertyName = "border-width";
        const InteractionState state{ component };

        if (const auto property = state.findMostApplicable(component.getProperties(),
                                                           propertyName);
            property.has_value())
        {
            return juce::VariantConverter<juce::BorderSize<float>>::fromVar(*property);
        }

        return styles.find<juce::BorderSize<float>>("border-width");
    }

    [[nodiscard]] static auto toFloat(const juce::BorderSize<int>& border)
    {
        return juce::BorderSize<float>{
            static_cast<float>(border.getTop()),
            static_cast<float>(border.getLeft()),
            static_cast<float>(border.getBottom()),
            static_cast<float>(border.getRight()),
        };
    }

    juce::BorderSize<float> getBorderWidth(const juce::Component& component,
                                           const Styles& styles,
                                           juce::BorderSize<float> defaultThickness)
    {
        return findBorderWidth(component, styles)
            .value_or(defaultThickness);
    }

    juce::BorderSize<float> getBorderWidth(const juce::Label& label,
                                           const Styles& styles)
    {
        return findBorderWidth(label, styles)
            .value_or(toFloat(label
                                  .getLookAndFeel()
                                  .getLabelBorderSize(*const_cast<juce::Label*>(&label))));
    }

    juce::BorderSize<float> getBorderWidth(const juce::Button& button,
                                           const Styles& styles)
    {
        return findBorderWidth(button, styles)
            .value_or(juce::BorderSize{ 1.0f });
    }

    juce::BorderSize<float> getBorderWidth(const juce::TextEditor& textEditor,
                                           const Styles& styles)
    {
        return findBorderWidth(textEditor, styles)
            .value_or(juce::BorderSize{ 1.0f });
    }

    juce::BorderSize<float> getBorderWidth(const juce::PopupMenu&,
                                           const Styles& styles)
    {
        return styles
            .find<juce::BorderSize<float>>("border-width")
            .value_or(juce::BorderSize{
#if JUCE_MAC
                0.0f,
#else
                1.0f,
#endif
            });
    }

    juce::BorderSize<float> getBorderWidth(const juce::PopupMenu::Item&,
                                           const Styles& styles)
    {
        return styles
            .find<juce::BorderSize<float>>("border-width")
            .value_or(juce::BorderSize{ 0.0f });
    }

    juce::BorderSize<float> getBorderWidth(const juce::ComboBox& comboBox,
                                           const Styles& styles)
    {
        return findBorderWidth(comboBox, styles)
            .value_or(juce::BorderSize{ 1.0f });
    }

    juce::BorderSize<float> getBorderWidth(const juce::GroupComponent& group,
                                           const Styles& styles)
    {
        return findBorderWidth(group, styles)
            .value_or(juce::BorderSize{ 2.0f });
    }

    juce::BorderSize<float> getBorderWidth(const juce::Slider& slider,
                                           const Styles& styles)
    {
        return findBorderWidth(slider, styles)
            .value_or(juce::BorderSize{ 1.0f });
    }

    juce::BorderSize<float> getBorderWidth(const juce::ProgressBar& bar,
                                           const Styles& styles)
    {
        return findBorderWidth(bar, styles)
            .value_or(juce::BorderSize{ 0.0f });
    }
} // namespace jive
