#include "jive_LookAndFeel.h"

#include "default-styles/jive_DefaultComboBoxStyles.h"
#include "default-styles/jive_DefaultGroupComponentStyles.h"
#include "default-styles/jive_DefaultLabelStyles.h"
#include "default-styles/jive_DefaultProgressBarStyles.h"
#include "default-styles/jive_DefaultScrollBarStyles.h"
#include "default-styles/jive_DefaultSliderStyles.h"
#include "jive_Shadow.h"
#include "themes/jive_SteelTheme.h"

namespace jive
{
    using namespace std::placeholders;

    LookAndFeel::LookAndFeel(juce::Component& comp)
        : attachedComponent{ &comp }
    {
        attachedComponent->setLookAndFeel(this);
    }

    LookAndFeel::~LookAndFeel()
    {
        if (attachedComponent != nullptr)
            attachedComponent->setLookAndFeel(nullptr);
    }

    void LookAndFeel::addPainter(const ComponentPredicate& predicate,
                                 const ComponentPainter& painter,
                                 Precedence precedence)
    {
        painters.emplace_back(precedence, painter, predicate);
        std::sort(std::begin(painters),
                  std::end(painters),
                  [](auto first, auto second) {
                      return std::get<0>(first) > std::get<0>(second);
                  });
    }

    void LookAndFeel::addPainter(const ComponentPredicate& predicate,
                                 const ProgressBarPainter& painter,
                                 Precedence precedence)
    {
        progressBarPainters.emplace_back(precedence, painter, predicate);
        std::sort(std::begin(progressBarPainters),
                  std::end(progressBarPainters),
                  [](auto first, auto second) {
                      return std::get<0>(first) > std::get<0>(second);
                  });
    }

    void LookAndFeel::addPainter(const PopupPredicate& predicate,
                                 const PopupPainter& painter,
                                 Precedence precedence)
    {
        popupPainters.emplace_back(precedence, painter, predicate);
        std::sort(std::begin(popupPainters),
                  std::end(popupPainters),
                  [](auto first, auto second) {
                      return std::get<0>(first) > std::get<0>(second);
                  });
    }

    void LookAndFeel::addPainter(const PopupItemPredicate& predicate,
                                 const PopupItemPainter& painter,
                                 Precedence precedence)
    {
        popupItemPainters.emplace_back(precedence, painter, predicate);
        std::sort(std::begin(popupItemPainters),
                  std::end(popupItemPainters),
                  [](auto first, auto second) {
                      return std::get<0>(first) > std::get<0>(second);
                  });
    }

    void LookAndFeel::clearPainters()
    {
        painters.clear();
        popupPainters.clear();
        popupItemPainters.clear();
        progressBarPainters.clear();
    }

    void LookAndFeel::addStyles(ComponentPredicate predicate,
                                const Styles& styles,
                                const InteractionState& interactionState,
                                Precedence precedence)
    {
        stylers.emplace_back(precedence, styles, interactionState, predicate);
        std::sort(std::begin(stylers),
                  std::end(stylers),
                  [](auto first, auto second) {
                      if (std::get<0>(first) == std::get<0>(second))
                          return std::get<2>(first).asInt() > std::get<2>(second).asInt();

                      return std::get<0>(first) > std::get<0>(second);
                  });
    }

    void LookAndFeel::addStyles(const juce::String& componentID,
                                const Styles& styles,
                                const InteractionState& interactionState)
    {
        const auto predicate = [componentID](const juce::Component& component) {
            return component.getComponentID() == componentID;
        };
        addStyles(predicate, styles, interactionState, painterPrecedence::componentID);
    }

    void LookAndFeel::addStyles(const Styles& styles,
                                const InteractionState& interactionState)
    {
        static constexpr auto predicate = [](const auto&) {
            return true;
        };
        addStyles(predicate, styles, interactionState, painterPrecedence::tautologicalPredicate);
    }

    void LookAndFeel::addStyles(PopupPredicate predicate,
                                const Styles& styles,
                                Precedence precedence)
    {
        popupStylers.emplace_back(precedence, styles, predicate);
        std::sort(std::begin(stylers),
                  std::end(stylers),
                  [](auto first, auto second) {
                      if (std::get<0>(first) == std::get<0>(second))
                          return std::get<2>(first).asInt() > std::get<2>(second).asInt();

                      return std::get<0>(first) > std::get<0>(second);
                  });
    }

    void LookAndFeel::clearStyles()
    {
        stylers.clear();
        popupStylers.clear();
        popupItemStylers.clear();
    }

    void LookAndFeel::drawComponent(juce::Graphics& g, const juce::Component& component) const
    {
        draw(g,
             component,
             [&g, &component](const auto& styles) {
                 default_styles::paintComponent(g, component, styles);
             });
    }

    // juce::ScrollBar
    void LookAndFeel::drawScrollbarButton(juce::Graphics& g,
                                          juce::ScrollBar& scrollBar,
                                          int width,
                                          int height,
                                          int direction,
                                          bool,
                                          bool,
                                          bool)
    {
        draw(g,
             scrollBar,
             [&g, &scrollBar, width, height, direction](const auto& styles) {
                 juce::Rectangle<int> bounds;

                 if (direction % 2 == 0)
                     bounds = scrollBar.getLocalBounds().removeFromTop(height);
                 else
                     bounds = scrollBar.getLocalBounds().removeFromLeft(width);

                 default_styles::paintScrollBarButton(g, scrollBar, styles, bounds.toFloat(), direction);
             });
    }

    void LookAndFeel::drawScrollbar(juce::Graphics& g,
                                    juce::ScrollBar& scrollBar,
                                    int,
                                    int,
                                    int,
                                    int,
                                    bool,
                                    int thumbStart,
                                    int thumbSize,
                                    bool,
                                    bool)
    {
        draw(g,
             scrollBar,
             [&g, &scrollBar, thumbStart, thumbSize](const auto& styles) {
                 default_styles::paintScrollBar(g, scrollBar, styles, thumbStart, thumbSize);
             });
    }

    // juce::Button
    void LookAndFeel::drawButtonBackground(juce::Graphics& g,
                                           juce::Button& button,
                                           const juce::Colour&,
                                           bool,
                                           bool)
    {
        if (auto* textButton = dynamic_cast<juce::TextButton*>(&button))
        {
            draw(g,
                 *textButton,
                 [&g, textButton](const auto& styles) {
                     default_styles::paintComponent(g, *textButton, styles);
                 });
        }
        else
        {
            draw(g,
                 button,
                 [&g, &button](const auto& styles) {
                     default_styles::paintComponent(g, button, styles);
                 });
        }
    }

    void LookAndFeel::drawButtonText(juce::Graphics& g,
                                     juce::TextButton& button,
                                     bool,
                                     bool)
    {
        draw(g,
             button,
             [&g, &button](const auto& styles) {
                 default_styles::paintText(g,
                                           button,
                                           button.getLocalBounds().toFloat(),
                                           styles,
                                           button.getButtonText(),
                                           juce::Justification::centred);
             });
    }

    void LookAndFeel::applyTheme(Theme theme)
    {
        clearPainters();
        clearStyles();

        if (theme == Theme::steel)
            themes::applySteel(*this);
    }

    juce::Rectangle<int> LookAndFeel::getToggleButtonCheckboxBounds(const juce::ToggleButton& button,
                                                                    const Styles& styles) const
    {
        auto bounds = button.getLocalBounds().toFloat();
        const auto boxSize = getFont(button, styles).getHeight() * 1.25f;
        return bounds
            .removeFromLeft(boxSize)
            .withSizeKeepingCentre(boxSize, boxSize)
            .toNearestInt();
    }

    void LookAndFeel::drawToggleButton(juce::Graphics& g,
                                       juce::ToggleButton& button,
                                       bool,
                                       bool)
    {
        const auto boxBounds = getToggleButtonCheckboxBounds(button, findMostApplicableStyles(button));
        button.getProperties().set("jive::shadow-bounds", boxBounds.translated(button.getX(), button.getY()).toString());

        draw(g,
             button,
             [this, &g, &button, &boxBounds](const auto& styles) {
                 default_styles::drawRoundedRectangle(g,
                                                      boxBounds.toFloat(),
                                                      getBorderRadius(button, styles),
                                                      getBorderWidth(button, styles),
                                                      getBackgroundFill(button, styles),
                                                      getBorderFill(button, styles));

                 if (button.getToggleState())
                 {
                     g.setFillType(getAccentFill(button, styles));
                     const auto tick = getTickShape(1.0f);
                     g.fillPath(tick,
                                tick.getTransformToScaleToFit(boxBounds
                                                                  .toFloat()
                                                                  .withSizeKeepingCentre(juce::jmin(boxBounds.toFloat().getWidth(), 13.0f),
                                                                                         juce::jmin(boxBounds.toFloat().getHeight(), 13.0f)),
                                                              true));
                 }

                 g.setFillType(getForegroundFill(button, styles));
                 default_styles::paintText(g,
                                           button,
                                           button.getLocalBounds().toFloat().withTrimmedLeft(boxBounds.toFloat().getWidth() + 5.0f),
                                           styles,
                                           button.getButtonText(),
                                           juce::Justification::centredLeft);
             });
    }

    // juce::TextEditor
    void LookAndFeel::fillTextEditorBackground(juce::Graphics& g,
                                               int,
                                               int,
                                               juce::TextEditor& textEditor)
    {
        draw(g,
             textEditor,
             [&g, &textEditor](const auto& styles) {
                 default_styles::drawRoundedRectangle(g,
                                                      textEditor.getLocalBounds().toFloat(),
                                                      getBorderRadius(textEditor, styles),
                                                      getBorderWidth(textEditor, styles),
                                                      getBackgroundFill(textEditor, styles),
                                                      juce::Colour{});

                 const auto font = getFont(textEditor, styles);
                 textEditor.applyFontToAllText(font);
             });
    }

    void LookAndFeel::drawTextEditorOutline(juce::Graphics& g,
                                            int,
                                            int,
                                            juce::TextEditor& textEditor)
    {
        draw(g,
             textEditor,
             [&g, &textEditor](const auto& styles) {
                 default_styles::drawRoundedRectangle(g,
                                                      textEditor.getLocalBounds().toFloat(),
                                                      getBorderRadius(textEditor, styles),
                                                      getBorderWidth(textEditor, styles),
                                                      juce::Colour{},
                                                      getBorderFill(textEditor, styles));
             });
    }

    void Caret::paint(juce::Graphics& g)
    {
        dynamic_cast<LookAndFeel&>(getLookAndFeel())
            .drawComponent(g, *this);
    }

    juce::CaretComponent* LookAndFeel::createCaretComponent(juce::Component* keyFocusOwner)
    {
        return new Caret{ keyFocusOwner };
    }

    // juce::PopupMenu
    void LookAndFeel::drawComboBoxPopupBackground(juce::Graphics& g,
                                                  const juce::ComboBox& comboBox,
                                                  const juce::PopupMenu::Options& options,
                                                  const juce::Rectangle<float>& bounds) const
    {
        const auto* const popup = comboBox.getRootMenu();
        jassert(popup != nullptr);

        const auto styles = findMostApplicableStyles(*popup, comboBox);

        if (const auto painter = findMostApplicablePainter(*popup, comboBox);
            painter.has_value())
        {
            (*painter)(*popup, options, comboBox, bounds, styles, g);
            return;
        }

        default_styles::paintPopupMenu(g, *popup, options, bounds, styles);
    }

    void LookAndFeel::drawPopupMenuBackgroundWithOptions(juce::Graphics& g,
                                                         int width,
                                                         int height,
                                                         const juce::PopupMenu::Options& options)
    {
        if (const auto* const comboBox = dynamic_cast<juce::ComboBox*>(options.getTopLevelTargetComponent()))
            return drawComboBoxPopupBackground(g, *comboBox, options, juce::Rectangle{ width, height }.toFloat());

        // Don't know how to draw a popup menu that hasn't come from a combo box!
        // Override this method to decide how to handle this.
        jassertfalse;
    }

    void LookAndFeel::drawComboBoxPopupItem(juce::Graphics& g,
                                            const juce::PopupMenu::Item& item,
                                            const juce::ComboBox& comboBox,
                                            const juce::PopupMenu::Options& options,
                                            const InteractionState& interactionState,
                                            const juce::Rectangle<float>& bounds) const
    {
        const auto* const popup = comboBox.getRootMenu();
        jassert(popup != nullptr);

        const auto styles = findMostApplicableStyles(item, *popup, comboBox);

        if (const auto painter = findMostApplicablePainter(item, *popup, comboBox);
            painter.has_value())
        {
            (*painter)(item, *popup, options, comboBox, bounds, styles, interactionState, g);
            return;
        }

        default_styles::paintPopupMenuItem(g, item, *popup, options, comboBox, bounds, interactionState, styles);
    }

    void LookAndFeel::drawPopupMenuItemWithOptions(juce::Graphics& g,
                                                   const juce::Rectangle<int>& bounds,
                                                   bool isHighlighted,
                                                   const juce::PopupMenu::Item& item,
                                                   const juce::PopupMenu::Options& options)
    {
        if (const auto* const comboBox = dynamic_cast<juce::ComboBox*>(options.getTopLevelTargetComponent()))
        {
            InteractionState interactionState;

            if (isHighlighted)
                interactionState.setHovered();

            return drawComboBoxPopupItem(g, item, *comboBox, options, interactionState, bounds.toFloat());
        }

        // Don't know how to draw a popup menu that hasn't come from a combo box!
        // Override this method to decide how to handle this.
        jassertfalse;
    }

    void LookAndFeel::getIdealPopupMenuItemSizeWithOptions(const juce::String& text,
                                                           bool isSeparator,
                                                           int standardMenuItemHeight,
                                                           int& idealWidth,
                                                           int& idealHeight,
                                                           const juce::PopupMenu::Options& options)
    {
        if (isSeparator)
        {
            idealWidth = 50;
            idealHeight = standardMenuItemHeight > 0 ? standardMenuItemHeight / 2 : 10;
        }
        else
        {
            juce::Font font{ juce::FontOptions{} };

            if (auto* const comboBox = dynamic_cast<juce::ComboBox*>(options.getTopLevelTargetComponent());
                comboBox != nullptr && comboBox->getRootMenu() != nullptr)
            {
                juce::PopupMenu::Item dummyItem{ text };
                const auto styles = findMostApplicableStyles(dummyItem, *comboBox->getRootMenu(), *comboBox);
                font = getFont(dummyItem, *comboBox, styles);
            }
            else
            {
                font = getPopupMenuFont();
            }

            idealHeight = static_cast<int>(std::ceil(font.getHeight() * 1.3f));
            idealWidth = juce::GlyphArrangement::getStringWidthInt(font, text) + idealHeight * 2;
        }
    }

    // juce::ComboBox
    void LookAndFeel::drawComboBox(juce::Graphics& g,
                                   int,
                                   int,
                                   bool,
                                   int,
                                   int,
                                   int,
                                   int,
                                   juce::ComboBox& comboBox)
    {
        draw(g,
             comboBox,
             [&g, &comboBox](const auto& styles) {
                 default_styles::paintComboBox(g, comboBox, styles);
             });
    }

    juce::Label* LookAndFeel::createComboBoxTextBox(juce::ComboBox& comboBox)
    {
        auto* label = juce::LookAndFeel_V4::createComboBoxTextBox(comboBox);
        label->setInterceptsMouseClicks(false, false);
        return label;
    }

    void LookAndFeel::positionComboBoxText(juce::ComboBox& comboBox, juce::Label& label)
    {
        const auto styles = findMostApplicableStyles(label);
        const auto font = getFont(label, styles);
        const auto labelInsetY = (static_cast<float>(comboBox.getHeight()) - font.getHeight()) / 2.0f;
        const auto labelInsetX = labelInsetY * 1.5f;
        label.setBounds(comboBox.getLocalBounds()
                            .toFloat()
                            .reduced(labelInsetX, labelInsetY)
                            .toNearestInt());
    }

    // juce::Label
    void LookAndFeel::drawLabel(juce::Graphics& g, juce::Label& label)
    {
        draw(g,
             label,
             [&g, &label](const auto& styles) {
                 default_styles::paintLabel(g, label, styles);
             });
    }

    // juce::Slider
    void LookAndFeel::drawLinearSlider(juce::Graphics& g,
                                       int,
                                       int,
                                       int,
                                       int,
                                       float,
                                       float,
                                       float,
                                       juce::Slider::SliderStyle,
                                       juce::Slider& slider)
    {
        std::function<void(const Styles&)> defaultPainter;

        if (slider.isBar())
        {
            defaultPainter = [&g, &slider](const auto& styles) {
                default_styles::paintBarSlider(g, slider, styles);
            };
        }
        else
        {
            defaultPainter = [&g, &slider](const auto& styles) {
                default_styles::paintLinearSlider(g, slider, styles);
            };
        }

        draw(g, slider, defaultPainter);
    }

    void LookAndFeel::drawRotarySlider(juce::Graphics& g,
                                       int,
                                       int,
                                       int,
                                       int,
                                       float,
                                       float,
                                       float,
                                       juce::Slider& slider)
    {
        draw(g,
             slider,
             [&g, &slider](const auto& styles) {
                 default_styles::paintRotarySlider(g, slider, styles);
             });
    }

    void LookAndFeel::fillResizableWindowBackground(juce::Graphics& g,
                                                    int,
                                                    int,
                                                    const juce::BorderSize<int>&,
                                                    juce::ResizableWindow& window)
    {
        draw(g,
             window,
             [&g, &window](const auto& styles) {
                 g.setFillType(getBackgroundFill(window, styles));
                 g.fillAll();
             });
    }

    void LookAndFeel::drawGroupComponentOutline(juce::Graphics& g,
                                                int,
                                                int,
                                                const juce::String&,
                                                const juce::Justification&,
                                                juce::GroupComponent& group)
    {
        const auto bounds = group.getBounds().toFloat();
        group
            .getProperties()
            .set("jive::shadow-bounds",
                 getGroupBounds(group,
                                findMostApplicableStyles(group))
                     .translated(bounds.getX(), bounds.getY())
                     .toString());

        draw(g,
             group,
             [&g, &group](const auto& styles) {
                 default_styles::paintGroupComponent(g, group, styles);
             });
    }

    void LookAndFeel::drawProgressBar(juce::Graphics& g,
                                      juce::ProgressBar& progressBar,
                                      int,
                                      int,
                                      double progress,
                                      const juce::String& text)
    {
        updateShadow<juce::ProgressBar>(progressBar);

        const auto styles = findMostApplicableStyles(progressBar);

        if (const auto painter = findMostApplicablePainter(progressBar);
            painter.has_value())
        {
            (*painter)(progressBar, progress, text, styles, g);
            return;
        }

        default_styles::paintProgressBar(g, progressBar, progress, text, styles);
    }

    std::optional<LookAndFeel::ComponentPainter> LookAndFeel::findMostApplicablePainter(const juce::Component& component) const
    {
        for (const auto& [_, painter, isApplicable] : painters)
        {
            if (isApplicable(component))
                return painter;
        }

        return std::nullopt;
    }

    static void merge(Styles& destination, const Styles& source, bool onlyInheretableStyles = false)
    {
#if JUCE_MAJOR_VERSION >= 8
        destination.fontAscentOverride = destination.fontAscentOverride.has_value() ? destination.fontAscentOverride : source.fontAscentOverride;
        destination.fontDescentOverride = destination.fontDescentOverride.has_value() ? destination.fontDescentOverride : source.fontDescentOverride;
#endif

        destination.fontExtraKerningFactor = destination.fontExtraKerningFactor.has_value() ? destination.fontExtraKerningFactor : source.fontExtraKerningFactor;
        destination.fontFamily = destination.fontFamily.has_value() ? destination.fontFamily : source.fontFamily;
        destination.fontHorizontalScale = destination.fontHorizontalScale.has_value() ? destination.fontHorizontalScale : source.fontHorizontalScale;
        destination.fontPointSize = destination.fontPointSize.has_value() ? destination.fontPointSize : source.fontPointSize;

        for (const auto& entry : source.fontStyleFlags)
            destination.fontStyleFlags.emplace(entry);

        destination.foreground = destination.foreground.has_value() ? destination.foreground : source.foreground;

        if (onlyInheretableStyles)
            return;

        destination.accent = destination.accent.has_value() ? destination.accent : source.accent;
        destination.background = destination.background.has_value() ? destination.background : source.background;
        destination.borderFill = destination.borderFill.has_value() ? destination.borderFill : source.borderFill;
        destination.borderRadius = destination.borderRadius.has_value() ? destination.borderRadius : source.borderRadius;
        destination.borderWidth = destination.borderWidth.has_value() ? destination.borderWidth : source.borderWidth;
        destination.shadow = destination.shadow.has_value() ? destination.shadow : source.shadow;
        destination.thumb = destination.thumb.has_value() ? destination.thumb : source.thumb;
        destination.track = destination.track.has_value() ? destination.track : source.track;
    }

    Styles LookAndFeel::findMostApplicableStyles(const juce::Component& component) const
    {
        Styles result;

        for (const auto& [_, styles, interactionState, isApplicable] : stylers)
        {
            if (isApplicable(component) && interactionState.appliesTo(component))
                merge(result, styles);
        }

        if (const auto* const parent = component.getParentComponent())
            merge(result, findMostApplicableStyles(*parent), true);

        return result;
    }

    std::optional<LookAndFeel::PopupPainter> LookAndFeel::findMostApplicablePainter(const juce::PopupMenu& popup,
                                                                                    const juce::Component& component) const
    {
        for (const auto& [_, painter, isApplicable] : popupPainters)
        {
            if (isApplicable(popup, component))
                return painter;
        }

        return std::nullopt;
    }

    Styles LookAndFeel::findMostApplicableStyles(const juce::PopupMenu& popup,
                                                 const juce::Component& component) const
    {
        Styles result;

        for (const auto& [_, styles, isApplicable] : popupStylers)
        {
            if (isApplicable(popup, component))
                merge(result, styles);
        }

        merge(result, findMostApplicableStyles(component), true);

        return result;
    }

    std::optional<LookAndFeel::PopupItemPainter> LookAndFeel::findMostApplicablePainter(const juce::PopupMenu::Item& item,
                                                                                        const juce::PopupMenu& popup,
                                                                                        const juce::Component& component) const
    {
        for (const auto& [_, painter, isApplicable] : popupItemPainters)
        {
            if (isApplicable(item, popup, component))
                return painter;
        }

        return std::nullopt;
    }

    Styles LookAndFeel::findMostApplicableStyles(const juce::PopupMenu::Item& item,
                                                 const juce::PopupMenu& popup,
                                                 const juce::Component& component) const
    {
        Styles result;

        for (const auto& [_, styles, isApplicable] : popupItemStylers)
        {
            if (isApplicable(item, popup, component))
                merge(result, styles);
        }

        merge(result, findMostApplicableStyles(popup, component), true);

        return result;
    }

    std::optional<LookAndFeel::ProgressBarPainter> LookAndFeel::findMostApplicablePainter(const juce::ProgressBar& bar) const
    {
        for (const auto& [_, painter, isApplicable] : progressBarPainters)
        {
            if (isApplicable(bar))
                return painter;
        }

        return std::nullopt;
    }
} // namespace jive
