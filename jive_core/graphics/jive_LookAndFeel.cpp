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
        applyTheme(Theme::steel);
    }

    LookAndFeel::~LookAndFeel()
    {
        if (attachedComponent != nullptr)
            attachedComponent->setLookAndFeel(nullptr);
    }

    juce::Uuid LookAndFeel::addPainter(const ComponentPredicate& predicate,
                                       const ComponentPainter& painter,
                                       Precedence precedence)
    {
        auto& newItem = painters.emplace_back(Painter<ComponentPainter, ComponentPredicate>{
            .id = juce::Uuid{},
            .precedence = precedence,
            .paint = painter,
            .appliesTo = predicate,
        });
        std::sort(std::begin(painters),
                  std::end(painters),
                  [](auto first, auto second) {
                      return first.precedence < second.precedence;
                  });

        return newItem.id;
    }

    juce::Uuid LookAndFeel::addPainter(const ComponentPredicate& predicate,
                                       const ProgressBarPainter& painter,
                                       Precedence precedence)
    {
        auto& newItem = progressBarPainters.emplace_back(Painter<ProgressBarPainter, ComponentPredicate>{
            .id = juce::Uuid{},
            .precedence = precedence,
            .paint = painter,
            .appliesTo = predicate,
        });
        std::sort(std::begin(progressBarPainters),
                  std::end(progressBarPainters),
                  [](auto first, auto second) {
                      return first.precedence < second.precedence;
                  });

        return newItem.id;
    }

    juce::Uuid LookAndFeel::addPainter(const PopupPredicate& predicate,
                                       const PopupPainter& painter,
                                       Precedence precedence)
    {
        auto& newItem = popupPainters.emplace_back(Painter<PopupPainter, PopupPredicate>{
            .id = juce::Uuid{},
            .precedence = precedence,
            .paint = painter,
            .appliesTo = predicate,
        });
        std::sort(std::begin(popupPainters),
                  std::end(popupPainters),
                  [](auto first, auto second) {
                      return first.precedence < second.precedence;
                  });

        return newItem.id;
    }

    juce::Uuid LookAndFeel::addPainter(const PopupItemPredicate& predicate,
                                       const PopupItemPainter& painter,
                                       Precedence precedence)
    {
        auto& newItem = popupItemPainters.emplace_back(Painter<PopupItemPainter, PopupItemPredicate>{
            .id = juce::Uuid{},
            .precedence = precedence,
            .paint = painter,
            .appliesTo = predicate,
        });
        std::sort(std::begin(popupItemPainters),
                  std::end(popupItemPainters),
                  [](auto first, auto second) {
                      return first.precedence < second.precedence;
                  });

        return newItem.id;
    }

    void LookAndFeel::removePainter(const juce::Uuid& uuid)
    {
        static constexpr auto remove = [](const juce::Uuid& id, auto& list) {
            list.erase(std::remove_if(std::begin(list),
                                      std::end(list),
                                      [&id](auto& item) {
                                          return item.id == id;
                                      }),
                       std::end(list));
        };

        remove(uuid, painters);
        remove(uuid, popupPainters);
        remove(uuid, popupItemPainters);
        remove(uuid, progressBarPainters);
    }

    void LookAndFeel::clearPainters()
    {
        painters.clear();
        popupPainters.clear();
        popupItemPainters.clear();
        progressBarPainters.clear();
    }

    juce::Uuid LookAndFeel::addStyles(ComponentPredicate predicate,
                                      const Styles& styles,
                                      const InteractionState& interactionState,
                                      Precedence precedence)
    {
        auto& newItem = stylers.emplace_back(Styler<ComponentPredicate>{
            .id = juce::Uuid{},
            .precedence = precedence,
            .interactionState = interactionState,
            .appliesTo = predicate,
            .styles = styles,
        });
        const auto result = newItem.id;

        std::sort(std::begin(stylers),
                  std::end(stylers),
                  [](auto first, auto second) {
                      if (first.precedence == second.precedence)
                      {
                          return first.interactionState.asInt()
                               > second.interactionState.asInt();
                      }

                      return first.precedence < second.precedence;
                  });

        return result;
    }

    juce::Uuid LookAndFeel::addStyles(const juce::String& componentID,
                                      const Styles& styles,
                                      const InteractionState& interactionState)
    {
        const auto predicate = [componentID](const juce::Component& component) {
            return component.getComponentID() == componentID;
        };
        return addStyles(predicate, styles, interactionState, painterPrecedence::componentID);
    }

    juce::Uuid LookAndFeel::addStyles(const Styles& styles,
                                      const InteractionState& interactionState)
    {
        static constexpr auto predicate = [](const auto&) {
            return true;
        };
        return addStyles(predicate, styles, interactionState, painterPrecedence::tautologicalPredicate);
    }

    juce::Uuid LookAndFeel::addStyles(PopupPredicate predicate,
                                      const Styles& styles,
                                      Precedence precedence)
    {
        auto& newItem = popupStylers.emplace_back(Styler<PopupPredicate>{
            .id = juce::Uuid{},
            .precedence = precedence,
            .interactionState = InteractionState{},
            .appliesTo = predicate,
            .styles = styles,
        });
        std::sort(std::begin(popupStylers),
                  std::end(popupStylers),
                  [](auto first, auto second) {
                      return first.precedence < second.precedence;
                  });

        return newItem.id;
    }

    Styles* LookAndFeel::findStyles(const juce::Uuid& uuid)
    {
        static constexpr auto find = [](const juce::Uuid& id, auto& list) -> Styles* {
            if (auto styles = std::find_if(std::begin(list),
                                           std::end(list),
                                           [&id](const auto& entry) {
                                               return entry.id == id;
                                           });
                styles != std::end(list))
            {
                return &styles->styles;
            }

            return nullptr;
        };

        if (auto* styles = find(uuid, stylers))
            return styles;
        if (auto* styles = find(uuid, popupStylers))
            return styles;
        if (auto* styles = find(uuid, popupItemStylers))
            return styles;

        return nullptr;
    }

    const Styles* LookAndFeel::findStyles(const juce::Uuid& uuid) const
    {
        return const_cast<LookAndFeel*>(this)->findStyles(uuid);
    }

    void LookAndFeel::removeStyles(const juce::Uuid& uuid)
    {
        static constexpr auto remove = [](const juce::Uuid& id, auto& list) {
            list.erase(std::remove_if(std::begin(list),
                                      std::end(list),
                                      [&id](auto& item) {
                                          return item.id == id;
                                      }),
                       std::end(list));
        };

        remove(uuid, stylers);
        remove(uuid, popupStylers);
        remove(uuid, popupItemStylers);
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

        for (auto& entry : stylers)
            entry.precedence += 10000;
        for (auto& entry : popupStylers)
            entry.precedence += 10000;
        for (auto& entry : popupItemStylers)
            entry.precedence += 10000;
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
        const auto styles = findMostApplicableStyles(progressBar);

        if (const auto painter = findMostApplicablePainter(progressBar);
            painter.has_value())
        {
            (*painter)(progressBar, progress, text, styles, g);
            return;
        }

        default_styles::paintProgressBar(g, progressBar, progress, text, styles);
        updateShadow<juce::ProgressBar>(progressBar);
    }

    std::optional<LookAndFeel::ComponentPainter> LookAndFeel::findMostApplicablePainter(const juce::Component& component) const
    {
        for (const auto& [uuid, precedence, painter, isApplicable] : painters)
        {
            juce::ignoreUnused(uuid, precedence);

            if (isApplicable(component))
                return painter;
        }

        return std::nullopt;
    }

    static void mergeLeft(Styles& destination, const Styles& source, bool onlyInheretableStyles = false)
    {
        destination.fill = destination.fill.hasValue() ? destination.fill : source.fill;

#if JUCE_MAJOR_VERSION >= 8
        destination.fontAscentOverride = destination.fontAscentOverride.hasValue() ? destination.fontAscentOverride : source.fontAscentOverride;
        destination.fontDescentOverride = destination.fontDescentOverride.hasValue() ? destination.fontDescentOverride : source.fontDescentOverride;
#endif

        destination.fontExtraKerningFactor = destination.fontExtraKerningFactor.hasValue() ? destination.fontExtraKerningFactor : source.fontExtraKerningFactor;
        destination.fontFamily = destination.fontFamily.hasValue() ? destination.fontFamily : source.fontFamily;
        destination.fontHorizontalScale = destination.fontHorizontalScale.hasValue() ? destination.fontHorizontalScale : source.fontHorizontalScale;
        destination.fontPointSize = destination.fontPointSize.hasValue() ? destination.fontPointSize : source.fontPointSize;

        if (source.fontStyleFlags.hasValue())
        {
            for (const auto& entry : source.fontStyleFlags.get())
                destination.fontStyleFlags.get().emplace(entry);
        }

        destination.foreground = destination.foreground.hasValue() ? destination.foreground : source.foreground;
        destination.transitions = destination.transitions != nullptr ? destination.transitions : source.transitions;

        if (onlyInheretableStyles)
            return;

        destination.accent = destination.accent.hasValue() ? destination.accent : source.accent;
        destination.background = destination.background.hasValue() ? destination.background : source.background;
        destination.borderFill = destination.borderFill.hasValue() ? destination.borderFill : source.borderFill;
        destination.borderRadius = destination.borderRadius.hasValue() ? destination.borderRadius : source.borderRadius;
        destination.borderWidth = destination.borderWidth.hasValue() ? destination.borderWidth : source.borderWidth;
        destination.shadow = destination.shadow.hasValue() ? destination.shadow : source.shadow;
        destination.stroke = destination.stroke.hasValue() ? destination.stroke : source.stroke;
        destination.thumb = destination.thumb.hasValue() ? destination.thumb : source.thumb;
        destination.track = destination.track.hasValue() ? destination.track : source.track;
    }

    template <typename Styler>
    [[nodiscard]] static auto collectApplicableStylersOrdered(const std::vector<Styler>& stylers,
                                                              const juce::Component& component)
    {
        using GenerationOffset = uint;

        std::vector<
            std::tuple<
                LookAndFeel::Precedence,
                GenerationOffset,
                Styles,
                InteractionState,
                juce::Uuid>>
            allApplicable;

        for (const auto& styler : stylers)
        {
            GenerationOffset generationOffset = 0;

            for (auto* ancestor = &component;
                 ancestor != nullptr;
                 ancestor = ancestor->getParentComponent())
            {
                if (styler.appliesTo(*ancestor) && styler.interactionState.appliesTo(*ancestor))
                {
                    allApplicable.emplace_back(
                        std::make_tuple(
                            styler.precedence,
                            generationOffset,
                            styler.styles,
                            styler.interactionState,
                            styler.id));
                    break;
                }

                ++generationOffset;
            }
        }

        std::sort(std::begin(allApplicable),
                  std::end(allApplicable),
                  [](auto& first, auto& second) {
                      if (std::get<LookAndFeel::Precedence>(first) == std::get<LookAndFeel::Precedence>(second))
                          return std::get<GenerationOffset>(first) < std::get<GenerationOffset>(second);

                      return std::get<LookAndFeel::Precedence>(first) < std::get<LookAndFeel::Precedence>(second);
                  });

        return allApplicable;
    }

    template <typename Cache>
    [[nodiscard]] static auto findCacheEntry(const Cache& cache,
                                             const Styles& styles,
                                             const InteractionState& interactionState)
    {
        return std::find_if(std::begin(cache),
                            std::end(cache),
                            [&styles, &interactionState](const auto& entry) {
                                return entry.styles == styles
                                    && entry.interactionState == interactionState;
                            });
    }

    template <typename Cache>
    static void cleanCache(std::vector<Cache>& cache)
    {
        cache.erase(
            std::remove_if(
                std::begin(cache),
                std::end(cache),
                [](auto& entry) {
                    return entry.component == nullptr
                        || !entry.interactionState.appliesTo(*entry.component);
                }),
            std::end(cache));
    }

    template <typename Styler>
    [[nodiscard]] static auto buildStyles(const std::vector<Styler>& stylers,
                                          const juce::Component& component)
    {
        Styles result;

        for (const auto& [precedence,
                          generationOffset,
                          styles,
                          interactionState,
                          uuid] : collectApplicableStylersOrdered(stylers, component))
        {
            juce::ignoreUnused(precedence, interactionState);
            mergeLeft(result, styles, generationOffset > 0);
        }

        return result;
    }

    template <typename Cache>
    [[nodiscard]] static auto& findOrAddCacheEntry(std::vector<Cache>& cache,
                                                   const juce::Component& component)
    {
        if (auto entry = std::find_if(std::begin(cache),
                                      std::end(cache),
                                      [&component](auto& cacheEntry) {
                                          return cacheEntry.component == &component;
                                      });
            entry != std::end(cache))
        {
            return *entry;
        }

        return cache.emplace_back(Cache{
            const_cast<juce::Component*>(&component),
            Styles{},
        });
    }

    static void update(Styles& destination, const Styles& source)
    {
        if (source.accent.hasValue())
            destination.accent = source.accent.get();
        if (source.background.hasValue())
            destination.background = source.background.get();
        if (source.borderFill.hasValue())
            destination.borderFill = source.borderFill.get();
        if (source.borderRadius.hasValue())
            destination.borderRadius = source.borderRadius.get();
        if (source.borderWidth.hasValue())
            destination.borderWidth = source.borderWidth.get();
        if (source.direction.hasValue())
            destination.direction = source.direction.get();
        if (source.fill.hasValue())
            destination.fill = source.fill.get();
#if JUCE_MAJOR_VERSION >= 8
        if (source.fontAscentOverride.hasValue())
            destination.fontAscentOverride = source.fontAscentOverride.get();
        if (source.fontDescentOverride.hasValue())
            destination.fontDescentOverride = source.fontDescentOverride.get();
#endif
        if (source.fontExtraKerningFactor.hasValue())
            destination.fontExtraKerningFactor = source.fontExtraKerningFactor.get();
        if (source.fontFamily.hasValue())
            destination.fontFamily = source.fontFamily.get();
        if (source.fontHorizontalScale.hasValue())
            destination.fontHorizontalScale = source.fontHorizontalScale.get();
        if (source.fontPointSize.hasValue())
            destination.fontPointSize = source.fontPointSize.get();
        if (source.fontStyleFlags.hasValue())
            destination.fontStyleFlags = source.fontStyleFlags.get();
        if (source.foreground.hasValue())
            destination.foreground = source.foreground.get();
        if (source.shadow.hasValue())
            destination.shadow = source.shadow.get();
        if (source.stroke.hasValue())
            destination.stroke = source.stroke.get();
        if (source.textAlign.hasValue())
            destination.textAlign = source.textAlign.get();
        if (source.thumb.hasValue())
            destination.thumb = source.thumb.get();
        if (source.track.hasValue())
            destination.track = source.track.get();

        if (source.transitions != nullptr)
            destination.transitions = source.transitions;
    }

    Styles LookAndFeel::findMostApplicableStyles(const juce::Component& component) const
    {
        auto& entry = findOrAddCacheEntry(stylesCache, component);
        auto latestStyles = buildStyles(stylers, component);

        update(entry.styles, latestStyles);

        return entry.styles;
    }

    std::optional<LookAndFeel::PopupPainter> LookAndFeel::findMostApplicablePainter(const juce::PopupMenu& popup,
                                                                                    const juce::Component& component) const
    {
        for (const auto& [uuid, precedence, painter, isApplicable] : popupPainters)
        {
            juce::ignoreUnused(uuid, precedence);

            if (isApplicable(popup, component))
                return painter;
        }

        return std::nullopt;
    }

    Styles LookAndFeel::findMostApplicableStyles(const juce::PopupMenu& popup,
                                                 const juce::Component& component) const
    {
        Styles result;

        for (const auto& styler : popupStylers)
        {
            if (styler.appliesTo(popup, component))
                mergeLeft(result, styler.styles);
        }

        mergeLeft(result, findMostApplicableStyles(component), true);

        return result;
    }

    std::optional<LookAndFeel::PopupItemPainter> LookAndFeel::findMostApplicablePainter(const juce::PopupMenu::Item& item,
                                                                                        const juce::PopupMenu& popup,
                                                                                        const juce::Component& component) const
    {
        for (const auto& [uuid, precedence, painter, isApplicable] : popupItemPainters)
        {
            juce::ignoreUnused(uuid, precedence);

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

        for (const auto& styler : popupItemStylers)
        {
            if (styler.appliesTo(item, popup, component))
                mergeLeft(result, styler.styles);
        }

        mergeLeft(result, findMostApplicableStyles(popup, component), true);

        return result;
    }

    std::optional<LookAndFeel::ProgressBarPainter> LookAndFeel::findMostApplicablePainter(const juce::ProgressBar& bar) const
    {
        for (const auto& [uuid, precedence, painter, isApplicable] : progressBarPainters)
        {
            juce::ignoreUnused(uuid, precedence);

            if (isApplicable(bar))
                return painter;
        }

        return std::nullopt;
    }
} // namespace jive
