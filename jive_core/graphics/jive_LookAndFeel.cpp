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
        , vBlank{
            &comp,
            [this]() {
                onVBlankUpdate();
            },
        }
    {
        attachedComponent->setLookAndFeel(this);
        applyTheme(Theme::steel);
    }

    LookAndFeel::~LookAndFeel()
    {
        if (attachedComponent != nullptr)
            attachedComponent->setLookAndFeel(nullptr);
    }

    juce::Uuid LookAndFeel::addPainter(const StyleSelector& selector,
                                       const ComponentPainter& painter)
    {
        auto& newItem = painters.emplace_back(Painter<ComponentPainter>{
            juce::Uuid{},
            painter,
            selector,
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
    }

    void LookAndFeel::clearPainters()
    {
        painters.clear();
    }

    juce::Uuid LookAndFeel::addStyles(const StyleSelector& selector,
                                      const Styles& styles)
    {
        auto& newItem = stylers.emplace_back(Styler{
            juce::Uuid{},
            selector,
            styles,
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
    }

    void LookAndFeel::clearStyles()
    {
        stylers.clear();
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

        const auto styles = findMostApplicableStyles(comboBox);

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

        const auto styles = findMostApplicableStyles(comboBox);

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
                const auto styles = findMostApplicableStyles(*comboBox);
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

        default_styles::paintProgressBar(g, progressBar, progress, text, styles);
        updateShadow<juce::ProgressBar>(progressBar);
    }

    std::optional<LookAndFeel::ComponentPainter> LookAndFeel::findMostApplicablePainter(const juce::Component& component) const
    {
        for (const auto& [uuid, painter, selector] : painters)
        {
            juce::ignoreUnused(uuid);

            if (selector.appliesTo(component))
                return painter;
        }

        return std::nullopt;
    }

    static void mergeLeft(Styles& destination, const Styles& source, bool onlyInheretableStyles = false)
    {
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
        destination.direction = destination.direction.hasValue() ? destination.direction : source.direction;
        destination.fill = destination.fill.hasValue() ? destination.fill : source.fill;
        destination.shadow = destination.shadow.hasValue() ? destination.shadow : source.shadow;
        destination.stroke = destination.stroke.hasValue() ? destination.stroke : source.stroke;
        destination.textAlign = destination.textAlign.hasValue() ? destination.textAlign : source.textAlign;
        destination.thumb = destination.thumb.hasValue() ? destination.thumb : source.thumb;
        destination.track = destination.track.hasValue() ? destination.track : source.track;
    }

    template <typename Styler>
    [[nodiscard]] static auto collectApplicableStylersOrdered(const std::vector<Styler>& stylers,
                                                              const juce::Component& component)
    {
        using GenerationOffset = std::uint16_t;

        std::vector<std::tuple<GenerationOffset, Styles, juce::Uuid, StyleSelector>> allApplicable;

        for (const auto& styler : stylers)
        {
            GenerationOffset generationOffset = 0;

            for (auto* ancestor = &component;
                 ancestor != nullptr;
                 ancestor = ancestor->getParentComponent())
            {
                if (styler.selector.appliesTo(*ancestor))
                {
                    allApplicable.emplace_back(
                        std::make_tuple(
                            generationOffset,
                            styler.styles,
                            styler.id,
                            styler.selector));
                    break;
                }

                ++generationOffset;
            }
        }

        std::sort(std::begin(allApplicable),
                  std::end(allApplicable),
                  [](auto& first, auto& second) {
                      if (std::get<StyleSelector>(first) == std::get<StyleSelector>(second))
                          return std::get<GenerationOffset>(first) < std::get<GenerationOffset>(second);

                      return std::get<StyleSelector>(first) < std::get<StyleSelector>(second);
                  });

        return allApplicable;
    }

    template <typename Styler>
    [[nodiscard]] static auto buildStyles(const std::vector<Styler>& stylers,
                                          const juce::Component& component)
    {
        Styles result;

        for (const auto& [generationOffset,
                          styles,
                          uuid,
                          selector] : collectApplicableStylersOrdered(stylers, component))
        {
            juce::ignoreUnused(uuid);
            juce::ignoreUnused(selector);
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

        if (entry.styles.hasTransitionsInProgress())
            transitioningComponents.addIfNotAlreadyThere(const_cast<juce::Component*>(&component));
        else
            transitioningComponents.removeAllInstancesOf(const_cast<juce::Component*>(&component));

        return entry.styles;
    }

    void LookAndFeel::onVBlankUpdate()
    {
        for (auto& component : transitioningComponents)
            component->repaint();
    }
} // namespace jive
