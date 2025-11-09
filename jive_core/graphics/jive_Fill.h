#pragma once

#include "jive_Gradient.h"

#include <jive_core/interface/jive_InteractionState.h>

#include <juce_gui_basics/juce_gui_basics.h>

namespace jive
{
    namespace detail
    {
        using V4Colour = juce::LookAndFeel_V4::ColourScheme::UIColour;
        [[nodiscard]] inline auto getV4Colour(V4Colour colour)
        {
            return juce::LookAndFeel_V4::getDarkColourScheme().getUIColour(colour);
        }
    } // namespace detail

    struct Styles;

    class Fill
    {
    public:
        Fill() = default;
        Fill(const Fill& other) = default;
        Fill(Fill&& other) = default;
        Fill& operator=(const Fill& other) = default;
        Fill& operator=(Fill&& other) = default;
        ~Fill() = default;

        Fill(const juce::Colour&);
        Fill(const juce::ColourGradient&);
        Fill(const juce::FillType&);
        Fill(const Gradient&);

        // If the given string isn't valid, this will fallback to transparent black.
        // Use fromString() and check the optional result for better error handling.
        Fill(const juce::String&);
        Fill(const char*);

        [[nodiscard]] bool isColour() const;
        [[nodiscard]] bool isGradient() const;
        [[nodiscard]] std::optional<Gradient> getJiveGradient() const;
        [[nodiscard]] juce::FillType toJuceFillType(const juce::Rectangle<float>& bounds = juce::Rectangle<float>{}) const;

        Fill& operator=(const juce::Colour&);
        Fill& operator=(const juce::ColourGradient&);
        Fill& operator=(const juce::FillType&);
        Fill& operator=(const Gradient&);

        // If the given string isn't valid, this will fallback to transparent black.
        // Use fromString() and check the optional result for better error handling.
        Fill& operator=(const juce::String&);

        [[nodiscard]] bool operator==(const Fill&) const;
        [[nodiscard]] bool operator!=(const Fill&) const;
        [[nodiscard]] bool operator==(const juce::Colour&) const;
        [[nodiscard]] bool operator==(const juce::ColourGradient&) const;
        [[nodiscard]] bool operator==(const juce::FillType&) const;
        [[nodiscard]] bool operator==(const Gradient&) const;

        [[nodiscard]] static std::optional<Fill> fromString(const juce::String&);

    private:
        std::variant<juce::FillType, Gradient> fill;
    };

    template <>
    struct Interpolate<Fill>
    {
        [[nodiscard]] Fill operator()(const Fill& start,
                                      const Fill& end,
                                      double proportion) const
        {
            if (start.isColour() && end.isColour())
            {
                return Fill{
                    interpolate(start.toJuceFillType().colour,
                                end.toJuceFillType().colour,
                                proportion),
                };
            }

            return end;
        }
    };

    [[nodiscard]] std::optional<Fill> findBackgroundFill(const juce::Component& component,
                                                         const Styles& styles,
                                                         int colourID = -1);
    [[nodiscard]] std::optional<Fill> findForegroundFill(const juce::Component& component,
                                                         const Styles& styles,
                                                         int colourID = -1);
    [[nodiscard]] std::optional<Fill> findBorderFill(const juce::Component& component,
                                                     const Styles& styles,
                                                     int colourID = -1);
    [[nodiscard]] std::optional<Fill> findTrackFill(const juce::Component& component,
                                                    const Styles& styles,
                                                    int colourID = -1);
    [[nodiscard]] std::optional<Fill> findCaretFill(const juce::Component& component,
                                                    const Styles& styles,
                                                    int colourID = -1);
    [[nodiscard]] std::optional<Fill> findAccentFill(const juce::Component& component,
                                                     const Styles& styles,
                                                     int colourID = -1);

    [[nodiscard]] juce::FillType getBackgroundFill(const juce::Component& component,
                                                   const Styles& styles,
                                                   const Fill& defaultFill = juce::Colours::transparentBlack);
    [[nodiscard]] juce::FillType getBorderFill(const juce::Component& component,
                                               const Styles& styles,
                                               const Fill& defaultFill = detail::getV4Colour(detail::V4Colour::outline));

    [[nodiscard]] juce::FillType getBackgroundFill(const juce::Label& label,
                                                   const Styles& styles,
                                                   const Fill& defaultFill = juce::Colours::transparentBlack);
    [[nodiscard]] juce::FillType getBorderFill(const juce::Label& label,
                                               const Styles& styles,
                                               const Fill& defaultFill = detail::getV4Colour(detail::V4Colour::outline));
    [[nodiscard]] juce::FillType getForegroundFill(const juce::Label& label,
                                                   const Styles& styles,
                                                   const Fill& defaultFill = detail::getV4Colour(detail::V4Colour::defaultText));

    [[nodiscard]] juce::FillType getBackgroundFill(const juce::Slider& slider,
                                                   const Styles& styles,
                                                   const Fill& defaultFill = detail::getV4Colour(detail::V4Colour::widgetBackground));
    [[nodiscard]] juce::FillType getThumbFill(const juce::Slider& slider,
                                              const Styles& styles,
                                              const juce::Rectangle<float>& thumbBounds,
                                              const Fill& defaultFill = detail::getV4Colour(detail::V4Colour::defaultFill));
    [[nodiscard]] juce::FillType getTrackFill(const juce::Slider& slider,
                                              const Styles& styles,
                                              const Fill& defaultFill = detail::getV4Colour(detail::V4Colour::highlightedFill));

    [[nodiscard]] juce::FillType getBackgroundFill(const juce::ScrollBar& scrollBar,
                                                   const Styles& styles,
                                                   const Fill& defaultFill = juce::Colours::transparentBlack);
    [[nodiscard]] juce::FillType getThumbFill(const juce::ScrollBar& scrollBar,
                                              const Styles& styles,
                                              const Fill& defaultFill = detail::getV4Colour(detail::V4Colour::defaultFill));
    [[nodiscard]] juce::FillType getTrackFill(const juce::ScrollBar& scrollBar,
                                              const Styles& styles,
                                              const Fill& defaultFill = juce::Colours::transparentBlack);

    [[nodiscard]] juce::FillType getBackgroundFill(const juce::Button& button,
                                                   const Styles& styles,
                                                   const Fill& defaultFill = detail::getV4Colour(detail::V4Colour::widgetBackground),
                                                   const Fill& defaultToggledFill = detail::getV4Colour(detail::V4Colour::highlightedFill));
    [[nodiscard]] juce::FillType getBorderFill(const juce::Button& button,
                                               const Styles& styles,
                                               const Fill& defaultFill = detail::getV4Colour(detail::V4Colour::outline));
    [[nodiscard]] juce::FillType getForegroundFill(const juce::Button& button,
                                                   const Styles& styles,
                                                   const Fill& defaultFill = detail::getV4Colour(detail::V4Colour::defaultText),
                                                   const Fill& defaultToggledFill = detail::getV4Colour(detail::V4Colour::highlightedText));

    [[nodiscard]] juce::FillType getBackgroundFill(const juce::ToggleButton& button,
                                                   const Styles& styles,
                                                   const Fill& defaultFill = juce::Colours::transparentBlack);
    [[nodiscard]] juce::FillType getBorderFill(const juce::ToggleButton& button,
                                               const Styles& styles,
                                               const Fill& defaultFill = detail::getV4Colour(detail::V4Colour::outline));
    [[nodiscard]] juce::FillType getForegroundFill(const juce::ToggleButton& button,
                                                   const Styles& styles,
                                                   const Fill& defaultFill = detail::getV4Colour(detail::V4Colour::defaultText));
    [[nodiscard]] juce::FillType getAccentFill(const juce::ToggleButton& button,
                                               const Styles& styles,
                                               const Fill& defaultFill = detail::getV4Colour(detail::V4Colour::defaultText));

    [[nodiscard]] juce::FillType getBackgroundFill(const juce::TextEditor& textEditor,
                                                   const Styles& styles,
                                                   const Fill& defaultFill = detail::getV4Colour(detail::V4Colour::widgetBackground));
    [[nodiscard]] juce::FillType getBorderFill(const juce::TextEditor& textEditor,
                                               const Styles& styles,
                                               const Fill& defaultFill = detail::getV4Colour(detail::V4Colour::outline));
    [[nodiscard]] juce::FillType getForegroundFill(const juce::TextEditor& textEditor,
                                                   const Styles& styles,
                                                   const Fill& defaultFill = detail::getV4Colour(detail::V4Colour::defaultText));

    [[nodiscard]] juce::FillType getBackgroundFill(const juce::PopupMenu& popup,
                                                   const juce::PopupMenu::Options& options,
                                                   const Styles& styles,
                                                   const Fill& defaultFill = detail::getV4Colour(detail::V4Colour::menuBackground));
    [[nodiscard]] juce::FillType getBorderFill(const juce::PopupMenu& popup,
                                               const juce::PopupMenu::Options& options,
                                               const Styles& styles,
                                               const Fill& defaultFill = detail::getV4Colour(detail::V4Colour::menuText).withAlpha(0.6f));

    [[nodiscard]] juce::FillType getBackgroundFill(const juce::PopupMenu::Item& item,
                                                   const juce::PopupMenu::Options& options,
                                                   const Styles& styles,
                                                   const InteractionState& interactionState,
                                                   const juce::Rectangle<float>& bounds,
                                                   const Fill& defaultFill = juce::Colours::transparentBlack,
                                                   const Fill& defaultSelectedFill = detail::getV4Colour(detail::V4Colour::highlightedFill));
    [[nodiscard]] juce::FillType getBorderFill(const juce::PopupMenu::Item& item,
                                               const juce::PopupMenu::Options& options,
                                               const Styles& styles,
                                               const juce::Rectangle<float>& bounds,
                                               const Fill& defaultFill = detail::getV4Colour(detail::V4Colour::menuText).withAlpha(0.6f));
    [[nodiscard]] juce::FillType getForegroundFill(const juce::PopupMenu::Item& item,
                                                   const juce::PopupMenu::Options& options,
                                                   const Styles& styles,
                                                   const InteractionState& interactionState,
                                                   const juce::Rectangle<float>& bounds,
                                                   const Fill& defaultFill = detail::getV4Colour(detail::V4Colour::menuText),
                                                   const Fill& defaultSelectedFill = detail::getV4Colour(detail::V4Colour::highlightedText));

    [[nodiscard]] juce::FillType getBackgroundFill(const juce::ComboBox& comboBox,
                                                   const Styles& styles,
                                                   const Fill& defaultFill = detail::getV4Colour(detail::V4Colour::widgetBackground));
    [[nodiscard]] juce::FillType getBorderFill(const juce::ComboBox& comboBox,
                                               const Styles& styles,
                                               const Fill& defaultFill = detail::getV4Colour(detail::V4Colour::outline));
    [[nodiscard]] juce::FillType getForegroundFill(const juce::ComboBox& comboBox,
                                                   const Styles& styles,
                                                   const Fill& defaultFill = detail::getV4Colour(detail::V4Colour::defaultText));
    [[nodiscard]] juce::FillType getArrowFill(const juce::ComboBox& comboBox,
                                              const Styles& styles,
                                              const Fill& defaultFill = detail::getV4Colour(detail::V4Colour::defaultText));

    [[nodiscard]] juce::FillType getBackgroundFill(const juce::ResizableWindow& window,
                                                   const Styles& styles,
                                                   const Fill& defaultFill = detail::getV4Colour(detail::V4Colour::windowBackground));

    [[nodiscard]] juce::FillType getForegroundFill(const juce::GroupComponent& group,
                                                   const Styles& styles,
                                                   const Fill& defaultFill = detail::getV4Colour(detail::V4Colour::defaultText));
    [[nodiscard]] juce::FillType getBackgroundFill(const juce::GroupComponent& group,
                                                   const Styles& styles,
                                                   const Fill& defaultFill = juce::Colours::transparentBlack);
    [[nodiscard]] juce::FillType getBorderFill(const juce::GroupComponent& group,
                                               const Styles& styles,
                                               const Fill& defaultFill = detail::getV4Colour(detail::V4Colour::outline));

    [[nodiscard]] juce::FillType getBackgroundFill(const juce::ProgressBar& bar,
                                                   const Styles& styles,
                                                   const Fill& defaultFill = detail::getV4Colour(detail::V4Colour::widgetBackground));
    [[nodiscard]] juce::FillType getTrackFill(const juce::ProgressBar& bar,
                                              const Styles& styles,
                                              const Fill& defaultFill = detail::getV4Colour(detail::V4Colour::highlightedFill));
    [[nodiscard]] juce::FillType getBorderFill(const juce::ProgressBar& bar,
                                               const Styles& styles,
                                               const Fill& defaultFill = detail::getV4Colour(detail::V4Colour::outline));
    [[nodiscard]] juce::FillType getForegroundFill(const juce::ProgressBar& bar,
                                                   const Styles& styles,
                                                   const Fill& defaultFill = detail::getV4Colour(detail::V4Colour::defaultText));
} // namespace jive

namespace juce
{
    template <>
    class VariantConverter<jive::Fill>
    {
    public:
        static jive::Fill fromVar(const var& v);
        static var toVar(const jive::Fill& fill);
    };
} // namespace juce
