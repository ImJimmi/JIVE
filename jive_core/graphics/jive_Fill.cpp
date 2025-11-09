#include "jive_Fill.h"

#include "jive_LookAndFeel.h"
#include "jive_Styles.h"

#include <jive_core/geometry/jive_Geometry.h>
#include <jive_core/logging/jive_StringStreams.h>
#include <jive_core/values/variant-converters/jive_MiscVariantConverters.h>
#include <jive_core/values/variant-converters/jive_VariantConvertion.h>

namespace jive
{
    Fill::Fill(const juce::Colour& colour)
        : fill{ juce::FillType{ colour } }
    {
    }

    Fill::Fill(const juce::ColourGradient& gradient)
        : fill{ juce::FillType{ gradient } }
    {
    }

    Fill::Fill(const juce::FillType& fillType)
        : fill{ fillType }
    {
    }

    Fill::Fill(const Gradient& gradient)
        : fill{ gradient }
    {
    }

    Fill::Fill(const juce::String& fillString)
    {
        *this = fromString(fillString).value_or(juce::Colours::transparentBlack);
    }

    Fill::Fill(const char* fillString)
        : Fill{ juce::String{ fillString } }
    {
    }

    bool Fill::isColour() const
    {
        return std::holds_alternative<juce::FillType>(fill) && std::get<juce::FillType>(fill).isColour();
    }

    bool Fill::isGradient() const
    {
        return (std::holds_alternative<juce::FillType>(fill) && std::get<juce::FillType>(fill).isGradient())
            || std::holds_alternative<Gradient>(fill);
    }

    std::optional<Gradient> Fill::getJiveGradient() const
    {
        if (std::holds_alternative<Gradient>(fill))
            return std::get<Gradient>(fill);

        return std::nullopt;
    }

    juce::FillType Fill::toJuceFillType(const juce::Rectangle<float>& bounds) const
    {
        if (std::holds_alternative<juce::FillType>(fill))
            return std::get<juce::FillType>(fill);

        return juce::FillType{ std::get<Gradient>(fill).toJuceColourGradient(bounds) };
    }

    Fill& Fill::operator=(const juce::Colour& colour)
    {
        fill = juce::FillType{ colour };
        return *this;
    }

    Fill& Fill::operator=(const juce::ColourGradient& gradient)
    {
        fill = juce::FillType{ gradient };
        return *this;
    }

    Fill& Fill::operator=(const juce::FillType& fillType)
    {
        fill = fillType;
        return *this;
    }

    Fill& Fill::operator=(const Gradient& gradient)
    {
        fill = gradient;
        return *this;
    }

    Fill& Fill::operator=(const juce::String& fillString)
    {
        *this = fromString(fillString).value_or(juce::Colours::transparentBlack);
        return *this;
    }

    bool Fill::operator==(const Fill& other) const
    {
        return fill == other.fill;
    }

    bool Fill::operator!=(const Fill& other) const
    {
        return !(*this == other);
    }

    bool Fill::operator==(const juce::Colour& colour) const
    {
        return *this == juce::FillType{ colour };
    }

    bool Fill::operator==(const juce::ColourGradient& gradient) const
    {
        return *this == juce::FillType{ gradient };
    }

    bool Fill::operator==(const juce::FillType& fillType) const
    {
        return std::holds_alternative<juce::FillType>(fill) && std::get<juce::FillType>(fill) == fillType;
    }

    bool Fill::operator==(const Gradient& gradient) const
    {
        return std::holds_alternative<Gradient>(fill) && std::get<Gradient>(fill) == gradient;
    }

    std::optional<Fill> Fill::fromString(const juce::String& fillString)
    {
        if (auto gradient = Gradient::fromString(fillString))
            return Fill{ *gradient };

        return Fill{ parseColour(fillString) };
    }

    [[nodiscard]] static std::optional<juce::Colour> findColour(const juce::Component& component,
                                                                int colourID,
                                                                bool inheritFromAncestors = true)
    {
        if (colourID < 0)
            return std::nullopt;

        for (const auto* currentComponent = &component;
             currentComponent != nullptr;
             currentComponent = inheritFromAncestors ? currentComponent->getParentComponent() : nullptr)
        {
            if (currentComponent->isColourSpecified(colourID)
                || currentComponent->getLookAndFeel().isColourSpecified(colourID))
            {
                return currentComponent->findColour(colourID);
            }
        }

        return std::nullopt;
    }

    static std::optional<Fill> findFill(const juce::Component& component,
                                        const juce::String& propertyName,
                                        const Styles& styles,
                                        bool inheritFromAncestors,
                                        int colourID)
    {
        const InteractionState state{ component };

        for (const auto* currentComponent = &component;
             currentComponent != nullptr;
             currentComponent = inheritFromAncestors ? currentComponent->getParentComponent() : nullptr)
        {
            if (const auto property = state.findMostApplicable(currentComponent->getProperties(),
                                                               propertyName);
                property.has_value())
            {
                return parseColour(property->toString());
            }
        }

        if (const auto style = styles.find<Fill>(propertyName);
            style.has_value())
        {
            return style;
        }

        if (const auto colour = findColour(component, colourID, inheritFromAncestors);
            colour.has_value())
        {
            return Fill{ *colour };
        }

        return std::nullopt;
    }

    std::optional<Fill> findBackgroundFill(const juce::Component& component,
                                           const Styles& styles,
                                           int colourID)
    {
        return findFill(component, "background", styles, false, colourID);
    }

    std::optional<Fill> findForegroundFill(const juce::Component& component,
                                           const Styles& styles,
                                           int colourID)
    {
        return findFill(component, "foreground", styles, true, colourID);
    }

    std::optional<Fill> findBorderFill(const juce::Component& component,
                                       const Styles& styles,
                                       int colourID)
    {
        return findFill(component, "border-fill", styles, true, colourID);
    }

    static std::optional<Fill> findThumbFill(const juce::Component& component,
                                             const Styles& styles,
                                             int colourID)
    {
        return findFill(component, "thumb", styles, true, colourID);
    }

    std::optional<Fill> findTrackFill(const juce::Component& component,
                                      const Styles& styles,
                                      int colourID)
    {
        return findFill(component, "track", styles, true, colourID);
    }

    std::optional<Fill> findAccentFill(const juce::Component& component,
                                       const Styles& styles,
                                       int colourID)
    {
        return findFill(component, "accent", styles, true, colourID);
    }

    juce::FillType getBackgroundFill(const juce::Component& component,
                                     const Styles& styles,
                                     const Fill& defaultFill)
    {
        return jive::findBackgroundFill(component, styles)
            .value_or(defaultFill)
            .toJuceFillType(component.getLocalBounds().toFloat());
    }

    juce::FillType getBorderFill(const juce::Component& component,
                                 const Styles& styles,
                                 const Fill& defaultFill)
    {
        return jive::findBorderFill(component, styles)
            .value_or(defaultFill)
            .toJuceFillType(component.getLocalBounds().toFloat());
    }

    juce::FillType getBackgroundFill(const juce::Label& label,
                                     const Styles& styles,
                                     const Fill& defaultFill)
    {
        if (label.isBeingEdited())
        {
            if (const auto result = jive::findBackgroundFill(label, styles, juce::Label::backgroundWhenEditingColourId);
                result.has_value())
            {
                return result->toJuceFillType(label.getLocalBounds().toFloat());
            }
        }

        return jive::findBackgroundFill(label, styles, juce::Label::backgroundColourId)
            .value_or(defaultFill)
            .toJuceFillType(label.getLocalBounds().toFloat());
    }

    juce::FillType getBorderFill(const juce::Label& label,
                                 const Styles& styles,
                                 const Fill& defaultFill)
    {
        if (label.isBeingEdited())
        {
            if (const auto result = jive::findBorderFill(label, styles, juce::Label::outlineWhenEditingColourId);
                result.has_value())
            {
                return result->toJuceFillType(label.getLocalBounds().toFloat());
            }
        }

        return jive::findBorderFill(label, styles, juce::Label::outlineColourId)
            .value_or(defaultFill)
            .toJuceFillType(label.getLocalBounds().toFloat());
    }

    juce::FillType getForegroundFill(const juce::Label& label,
                                     const Styles& styles,
                                     const Fill& defaultFill)
    {
        if (label.isBeingEdited())
        {
            if (const auto result = jive::findForegroundFill(label, styles, juce::Label::textWhenEditingColourId);
                result.has_value())
            {
                return result->toJuceFillType(label.getLocalBounds().toFloat());
            }
        }

        return jive::findForegroundFill(label, styles, juce::Label::textColourId)
            .value_or(defaultFill)
            .toJuceFillType(label.getLocalBounds().toFloat());
    }

    juce::FillType getBackgroundFill(const juce::Slider& slider,
                                     const Styles& styles,
                                     const Fill& defaultFill)
    {
        if (slider.isRotary())
        {
            if (const auto result = jive::findBackgroundFill(slider, styles, juce::Slider::rotarySliderOutlineColourId);
                result.has_value())
            {
                return result->toJuceFillType(slider.getLocalBounds().toFloat());
            }
        }

        return jive::findBackgroundFill(slider, styles, juce::Slider::backgroundColourId)
            .value_or(defaultFill)
            .toJuceFillType(slider.getLocalBounds().toFloat());
    }

    juce::FillType getThumbFill(const juce::Slider& slider,
                                const Styles& styles,
                                const juce::Rectangle<float>& thumbBounds,
                                const Fill& defaultFill)
    {
        return jive::findThumbFill(slider, styles, juce::Slider::thumbColourId)
            .value_or(defaultFill)
            .toJuceFillType(thumbBounds);
    }

    juce::FillType getTrackFill(const juce::Slider& slider,
                                const Styles& styles,
                                const Fill& defaultFill)
    {
        return jive::findTrackFill(slider, styles, juce::Slider::trackColourId)
            .value_or(defaultFill)
            .toJuceFillType(slider.getLocalBounds().toFloat());
    }

    juce::FillType getBackgroundFill(const juce::ScrollBar& scrollBar,
                                     const Styles& styles,
                                     const Fill& defaultFill)
    {
        return jive::findBackgroundFill(scrollBar, styles, juce::ScrollBar::backgroundColourId)
            .value_or(defaultFill)
            .toJuceFillType(scrollBar.getLocalBounds().toFloat());
    }

    juce::FillType getThumbFill(const juce::ScrollBar& scrollBar,
                                const Styles& styles,
                                const Fill& defaultFill)
    {
        return jive::findForegroundFill(scrollBar, styles, juce::ScrollBar::thumbColourId)
            .value_or(defaultFill)
            .toJuceFillType(scrollBar.getLocalBounds().toFloat());
    }

    juce::FillType getTrackFill(const juce::ScrollBar& scrollBar,
                                const Styles& styles,
                                const Fill& defaultFill)
    {
        return jive::findTrackFill(scrollBar, styles, juce::ScrollBar::trackColourId)
            .value_or(defaultFill)
            .toJuceFillType(scrollBar.getLocalBounds().toFloat());
    }

    juce::FillType getBackgroundFill(const juce::Button& button,
                                     const Styles& styles,
                                     const Fill& defaultFill,
                                     const Fill& defaultToggledFill)
    {
        if (const auto result = jive::findBackgroundFill(button, styles))
            return result->toJuceFillType(button.getLocalBounds().toFloat());

        const auto colourID = button.getToggleState()
                                ? juce::TextButton::buttonOnColourId
                                : juce::TextButton::buttonColourId;

        if (auto colour = findColour(button, colourID))
        {
            if (button.isOver() || button.isDown())
                colour = colour->contrasting(button.isDown() ? 0.2f : 0.05f);

            return juce::FillType{ *colour };
        }

        return (button.getToggleState() ? defaultToggledFill : defaultFill)
            .toJuceFillType(button.getLocalBounds().toFloat());
    }

    juce::FillType getBorderFill(const juce::Button& button,
                                 const Styles& styles,
                                 const Fill& defaultFill)
    {
        return jive::findBorderFill(button, styles, juce::ComboBox::outlineColourId)
            .value_or(defaultFill)
            .toJuceFillType(button.getLocalBounds().toFloat());
    }

    juce::FillType getForegroundFill(const juce::Button& button,
                                     const Styles& styles,
                                     const Fill& defaultFill,
                                     const Fill& defaultToggledFill)
    {
        return jive::findForegroundFill(button,
                                        styles,
                                        button.getToggleState()
                                            ? juce::TextButton::textColourOffId
                                            : juce::TextButton::textColourOnId)
            .value_or(button.getToggleState() ? defaultToggledFill : defaultFill)
            .toJuceFillType(button.getLocalBounds().toFloat());
    }

    juce::FillType getBackgroundFill(const juce::ToggleButton& button,
                                     const Styles& styles,
                                     const Fill& defaultFill)
    {
        return jive::findBackgroundFill(button, styles)
            .value_or(defaultFill)
            .toJuceFillType(dynamic_cast<LookAndFeel&>(button.getLookAndFeel())
                                .getToggleButtonCheckboxBounds(button, styles)
                                .toFloat());
    }

    juce::FillType getBorderFill(const juce::ToggleButton& button,
                                 const Styles& styles,
                                 const Fill& defaultFill)
    {
        return jive::findBorderFill(button, styles, juce::ComboBox::outlineColourId)
            .value_or(defaultFill)
            .toJuceFillType(button.getLocalBounds().toFloat());
    }

    juce::FillType getForegroundFill(const juce::ToggleButton& button,
                                     const Styles& styles,
                                     const Fill& defaultFill)
    {
        return jive::findForegroundFill(button, styles, juce::ToggleButton::textColourId)
            .value_or(defaultFill)
            .toJuceFillType(button.getLocalBounds().toFloat());
    }

    juce::FillType getAccentFill(const juce::ToggleButton& button,
                                 const Styles& styles,
                                 const Fill& defaultFill)
    {
        return jive::findAccentFill(button, styles, juce::ToggleButton::tickColourId)
            .value_or(defaultFill)
            .toJuceFillType(button.getLocalBounds().toFloat());
    }

    juce::FillType getBackgroundFill(const juce::TextEditor& textEditor,
                                     const Styles& styles,
                                     const Fill& defaultFill)
    {
        return jive::findBackgroundFill(textEditor, styles, juce::TextEditor::backgroundColourId)
            .value_or(defaultFill)
            .toJuceFillType(textEditor.getLocalBounds().toFloat());
    }

    juce::FillType getBorderFill(const juce::TextEditor& textEditor,
                                 const Styles& styles,
                                 const Fill& defaultFill)
    {
        const auto colourID = textEditor.hasKeyboardFocus(true)
                                ? juce::TextEditor::focusedOutlineColourId
                                : juce::TextEditor::outlineColourId;
        return jive::findBorderFill(textEditor, styles, colourID)
            .value_or(defaultFill)
            .toJuceFillType(textEditor.getLocalBounds().toFloat());
    }

    juce::FillType getForegroundFill(const juce::TextEditor& textEditor,
                                     const Styles& styles,
                                     const Fill& defaultFill)
    {
        return jive::findForegroundFill(textEditor, styles, juce::TextEditor::textColourId)
            .value_or(defaultFill)
            .toJuceFillType(textEditor.getLocalBounds().toFloat());
    }

    juce::FillType getBackgroundFill(const juce::PopupMenu&,
                                     const juce::PopupMenu::Options& options,
                                     const Styles& styles,
                                     const Fill& defaultFill)
    {
        if (const auto* const target = options.getTopLevelTargetComponent())
        {
            return jive::findBackgroundFill(*target, styles, juce::PopupMenu::backgroundColourId)
                .value_or(defaultFill)
                .toJuceFillType(options.getTargetScreenArea().withZeroOrigin().toFloat());
        }

        return styles.background.value_or(defaultFill)
            .toJuceFillType(options.getTargetScreenArea().withZeroOrigin().toFloat());
    }

    juce::FillType getBorderFill(const juce::PopupMenu&,
                                 const juce::PopupMenu::Options& options,
                                 const Styles& styles,
                                 const Fill& defaultFill)
    {
        if (styles.borderFill.has_value())
            return styles.borderFill->toJuceFillType(options.getTargetScreenArea().withZeroOrigin().toFloat());

        if (const auto* const target = options.getTopLevelTargetComponent())
        {
            if (const auto& colour = findColour(*target, juce::PopupMenu::textColourId))
                return colour->withAlpha(0.6f);
        }

        return defaultFill.toJuceFillType(options.getTargetScreenArea().withZeroOrigin().toFloat());
    }

    juce::FillType getBackgroundFill(const juce::PopupMenu::Item& item,
                                     const juce::PopupMenu::Options& options,
                                     const Styles& styles,
                                     const InteractionState& interactionState,
                                     const juce::Rectangle<float>& bounds,
                                     const Fill& defaultFill,
                                     const Fill& defaultSelectedFill)
    {
        if (const auto* const target = options.getTopLevelTargetComponent())
        {
            const auto colourID = (interactionState.isHovered() && item.isEnabled) ? juce::PopupMenu::highlightedBackgroundColourId : -1;
            return jive::findBackgroundFill(*target, styles, colourID)
                .value_or(defaultFill)
                .toJuceFillType(bounds);
        }

        return styles
            .background
            .value_or(item.isTicked ? defaultSelectedFill : defaultFill)
            .toJuceFillType(bounds);
    }

    juce::FillType getBorderFill(const juce::PopupMenu::Item&,
                                 const juce::PopupMenu::Options& options,
                                 const Styles& styles,
                                 const juce::Rectangle<float>& bounds,
                                 const Fill& defaultFill)
    {
        if (styles.borderFill.has_value())
            return styles.borderFill->toJuceFillType(bounds);

        if (const auto* const target = options.getTopLevelTargetComponent())
        {
            if (const auto& colour = findColour(*target, juce::PopupMenu::textColourId))
                return colour->withAlpha(0.3f);
        }

        return defaultFill.toJuceFillType(bounds);
    }

    juce::FillType getForegroundFill(const juce::PopupMenu::Item& item,
                                     const juce::PopupMenu::Options& options,
                                     const Styles& styles,
                                     const InteractionState& interactionState,
                                     const juce::Rectangle<float>& bounds,
                                     const Fill& defaultFill,
                                     const Fill& defaultSelectedFill)
    {
        if (styles.foreground.has_value())
            return styles.foreground->toJuceFillType(bounds);

        if (const auto* const target = options.getTopLevelTargetComponent())
        {
            const auto colourID = interactionState.isHovered()
                                    ? juce::PopupMenu::highlightedTextColourId
                                    : juce::PopupMenu::textColourId;

            if (const auto& colour = findColour(*target, colourID))
                return colour->withAlpha((interactionState.isHovered() || item.isEnabled) ? 1.0f : 0.5f);
        }

        return (item.isTicked ? defaultSelectedFill : defaultFill)
            .toJuceFillType(bounds);
    }

    juce::FillType getBackgroundFill(const juce::ComboBox& comboBox,
                                     const Styles& styles,
                                     const Fill& defaultFill)
    {
        return findBackgroundFill(comboBox, styles, juce::ComboBox::backgroundColourId)
            .value_or(defaultFill)
            .toJuceFillType(comboBox.getLocalBounds().toFloat());
    }

    juce::FillType getBorderFill(const juce::ComboBox& comboBox,
                                 const Styles& styles,
                                 const Fill& defaultFill)
    {
        return findBorderFill(comboBox, styles, juce::ComboBox::outlineColourId)
            .value_or(defaultFill)
            .toJuceFillType(comboBox.getLocalBounds().toFloat());
    }

    juce::FillType getForegroundFill(const juce::ComboBox& comboBox,
                                     const Styles& styles,
                                     const Fill& defaultFill)
    {
        return findForegroundFill(comboBox, styles, juce::ComboBox::textColourId)
            .value_or(defaultFill)
            .toJuceFillType(comboBox.getLocalBounds().toFloat());
    }

    juce::FillType getArrowFill(const juce::ComboBox& comboBox,
                                const Styles& styles,
                                const Fill& defaultFill)
    {
        return findForegroundFill(comboBox, styles, juce::ComboBox::arrowColourId)
            .value_or(defaultFill)
            .toJuceFillType(comboBox.getLocalBounds().toFloat());
    }

    juce::FillType getBackgroundFill(const juce::ResizableWindow& window,
                                     const Styles& styles,
                                     const Fill& defaultFill)
    {
        return findBackgroundFill(window, styles, juce::ResizableWindow::backgroundColourId)
            .value_or(defaultFill)
            .toJuceFillType(window.getLocalBounds().toFloat());
    }

    juce::FillType getForegroundFill(const juce::GroupComponent& group,
                                     const Styles& styles,
                                     const Fill& defaultFill)
    {
        return findForegroundFill(group, styles, juce::GroupComponent::textColourId)
            .value_or(defaultFill)
            .toJuceFillType(getTitleBounds(group, styles));
    }

    juce::FillType getBackgroundFill(const juce::GroupComponent& group,
                                     const Styles& styles,
                                     const Fill& defaultFill)
    {
        return findBackgroundFill(group, styles)
            .value_or(defaultFill)
            .toJuceFillType(getGroupBounds(group, styles));
    }

    juce::FillType getBorderFill(const juce::GroupComponent& group,
                                 const Styles& styles,
                                 const Fill& defaultFill)
    {
        return findBorderFill(group, styles, juce::GroupComponent::outlineColourId)
            .value_or(defaultFill)
            .toJuceFillType(getGroupBounds(group, styles));
    }

    juce::FillType getBackgroundFill(const juce::ProgressBar& bar,
                                     const Styles& styles,
                                     const Fill& defaultFill)
    {
        return findBackgroundFill(bar, styles, juce::ProgressBar::backgroundColourId)
            .value_or(defaultFill)
            .toJuceFillType(bar.getLocalBounds().toFloat());
    }

    juce::FillType getTrackFill(const juce::ProgressBar& bar,
                                const Styles& styles,
                                const Fill& defaultFill)
    {
        return findTrackFill(bar, styles, juce::ProgressBar::foregroundColourId)
            .value_or(defaultFill)
            .toJuceFillType(bar.getLocalBounds().toFloat());
    }

    juce::FillType getBorderFill(const juce::ProgressBar& bar,
                                 const Styles& styles,
                                 const Fill& defaultFill)
    {
        return findBorderFill(bar, styles)
            .value_or(defaultFill)
            .toJuceFillType(bar.getLocalBounds().toFloat());
    }

    juce::FillType getForegroundFill(const juce::ProgressBar& bar,
                                     const Styles& styles,
                                     const Fill& defaultFill)
    {
        return findForegroundFill(bar, styles)
            .value_or(defaultFill)
            .toJuceFillType(bar.getLocalBounds().toFloat());
    }
} // namespace jive

namespace juce
{
    jive::Fill VariantConverter<jive::Fill>::fromVar(const var& v)
    {
        if (v.isString())
            return jive::Fill{ VariantConverter<Colour>::fromVar(v) };

        if (v.hasProperty("gradient"))
            return jive::Fill{ VariantConverter<jive::Gradient>::fromVar(v) };

        return jive::Fill{ Colour{} };
    }

    var VariantConverter<jive::Fill>::toVar(const jive::Fill& fill)
    {
        if (auto gradient = fill.getJiveGradient())
            return jive::toVar(*gradient);

        return jive::toVar(fill.toJuceFillType().colour);
    }
} // namespace juce
