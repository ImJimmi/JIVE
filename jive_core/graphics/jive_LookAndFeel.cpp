#include "jive_LookAndFeel.h"

#include "default-styles/jive_DefaultComboBoxStyles.h"
#include "default-styles/jive_DefaultGroupComponentStyles.h"
#include "default-styles/jive_DefaultLabelStyles.h"
#include "default-styles/jive_DefaultProgressBarStyles.h"
#include "default-styles/jive_DefaultScrollBarStyles.h"
#include "default-styles/jive_DefaultSliderStyles.h"
#include "jive_Shadow.h"
#include "themes/jive_HighContrastTheme.h"
#include "themes/jive_MaterialTheme.h"
#include "themes/jive_SteelTheme.h"
#include "themes/jive_SynthwaveTheme.h"

#include <jive_core/components/widgets/jive_ComboBoxComponent.h>

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
        stylersVersion++;
        usesOpacity = usesOpacity || styles.opacity.hasValue();

        return newItem.id;
    }

    Styles* LookAndFeel::findStyles(const juce::Uuid& uuid)
    {
        stylersVersion++;
        return const_cast<Styles*>(std::as_const(*this).findStyles(uuid));
    }

    const Styles* LookAndFeel::findStyles(const juce::Uuid& uuid) const
    {
        static constexpr auto find = [](const juce::Uuid& id, const auto& list) -> const Styles* {
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

        if (const auto* styles = find(uuid, stylers))
            return styles;

        return nullptr;
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
        stylersVersion++;
    }

    void LookAndFeel::clearStyles()
    {
        stylers.clear();
        stylersVersion++;
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
        for (const auto& uuid : std::exchange(themeStyles, {}))
            removeStyles(uuid);

        const auto numStylersBeforeTheme = std::size(stylers);

        switch (theme)
        {
        case Theme::juce:
            break;
        case Theme::steel:
            themes::applySteel(*this);
            break;
        case Theme::material:
            themes::applyMaterial(*this);
            break;
        case Theme::synthwave:
            themes::applySynthwave(*this);
            break;
        case Theme::highContrast:
            themes::applyHighContrast(*this);
            break;
        }

        for (auto i = numStylersBeforeTheme; i < std::size(stylers); i++)
            themeStyles.push_back(stylers[i].id);
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

    float LookAndFeel::getToggleButtonTextInset(const juce::ToggleButton& button,
                                                const Styles& styles) const
    {
        static constexpr auto gapBetweenCheckboxAndText = 5.0f;
        return static_cast<float>(getToggleButtonCheckboxBounds(button, styles).getWidth())
             + gapBetweenCheckboxAndText;
    }

    void LookAndFeel::drawToggleButton(juce::Graphics& g,
                                       juce::ToggleButton& button,
                                       bool,
                                       bool)
    {
        const auto boxBounds = getToggleButtonCheckboxBounds(button, findMostApplicableStyles(button));

        draw(g,
             button,
             [this, &g, &button, &boxBounds](const auto& styles) {
                 const auto borderRadius = getBorderRadius(button, styles);
                 default_styles::drawRoundedRectangle(g,
                                                      boxBounds.toFloat(),
                                                      borderRadius,
                                                      getBorderWidth(button, styles),
                                                      getBackgroundFill(button, styles),
                                                      getBorderFill(button, styles));
                 button.getProperties().set(ids::shadowPath(), getShape(borderRadius, boxBounds.toFloat()).toString());

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
                                           button.getLocalBounds().toFloat().withTrimmedLeft(getToggleButtonTextInset(button, styles)),
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

        default_styles::paintPopupMenu(g, *popup, options, bounds, findPopupStyles(comboBox));
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

        default_styles::paintPopupMenuItem(g,
                                           item,
                                           *popup,
                                           options,
                                           comboBox,
                                           bounds,
                                           interactionState,
                                           findPopupStyles(comboBox),
                                           findPopupItemStyles(comboBox, item, interactionState));
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

    // JUCE only says which item it's measuring by its text.
    [[nodiscard]] static juce::PopupMenu::Item findPopupMenuItem(const juce::PopupMenu& menu, const juce::String& text)
    {
        for (juce::PopupMenu::MenuItemIterator iterator{ menu, true }; iterator.next();)
        {
            if (iterator.getItem().text == text)
                return iterator.getItem();
        }

        return juce::PopupMenu::Item{ text };
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
            auto font = getPopupMenuFont();
            auto textFont = font;

            if (auto* const comboBox = dynamic_cast<juce::ComboBox*>(options.getTopLevelTargetComponent());
                comboBox != nullptr && comboBox->getRootMenu() != nullptr)
            {
                const auto item = findPopupMenuItem(*comboBox->getRootMenu(), text);
                font = getFont(juce::PopupMenu::Item{}, *comboBox, findPopupStyles(*comboBox));
                textFont = getFont(item, *comboBox, findPopupItemStyles(*comboBox, item, {}));
            }

            const auto size = default_styles::getIdealPopupMenuItemSize(text, font, textFont);
            idealWidth = static_cast<int>(std::ceil(size.getWidth()));
            idealHeight = static_cast<int>(std::ceil(size.getHeight()));
        }
    }

    int LookAndFeel::getPopupMenuBorderSizeWithOptions(const juce::PopupMenu::Options& options)
    {
        if (const auto* const comboBox = dynamic_cast<juce::ComboBox*>(options.getTopLevelTargetComponent());
            comboBox != nullptr && comboBox->getRootMenu() != nullptr)
        {
            const auto styles = findPopupStyles(*comboBox);
            return default_styles::getPopupMenuBorderSize(*comboBox->getRootMenu(),
                                                          styles,
                                                          getFont(juce::PopupMenu::Item{}, *comboBox, styles));
        }

        return juce::LookAndFeel_V4::getPopupMenuBorderSizeWithOptions(options);
    }

    // JUCE makes popup windows opaque, which would leave the area outside of
    // rounded corners unpainted.
    void LookAndFeel::preparePopupMenuWindow(juce::Component& window)
    {
        if (juce::Desktop::canUseSemiTransparentWindows())
            window.setOpaque(false);
    }

    // juce::ComboBox
    juce::Rectangle<float> LookAndFeel::getComboBoxIconBounds(const juce::ComboBox& comboBox,
                                                              const Styles& styles) const
    {
        return default_styles::getComboBoxIconBounds(comboBox, styles);
    }

    juce::Rectangle<float> LookAndFeel::getIdealComboBoxSize(const juce::ComboBox& comboBox) const
    {
        return default_styles::getIdealComboBoxSize(comboBox, findMostApplicableStyles(comboBox));
    }

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

    // JUCE bases a combo box's font on its height, but here a combo box's height
    // is based on its font.
    juce::Font LookAndFeel::getComboBoxFont(juce::ComboBox&)
    {
        return parseFont("");
    }

    juce::Label* LookAndFeel::createComboBoxTextBox(juce::ComboBox& comboBox)
    {
        auto* label = juce::LookAndFeel_V4::createComboBoxTextBox(comboBox);
        label->setInterceptsMouseClicks(false, false);
        return label;
    }

    void LookAndFeel::positionComboBoxText(juce::ComboBox& comboBox, juce::Label& label)
    {
        const auto styles = findMostApplicableStyles(comboBox);
        label.setFont(getFont(comboBox, styles));
        label.setBounds(getBorderWidth(label, findMostApplicableStyles(label))
                            .addedTo(default_styles::getComboBoxTextBounds(comboBox, styles))
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
        updateShadow<juce::ProgressBar>(progressBar, styles);
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
        destination.lineSpacing = destination.lineSpacing.hasValue() ? destination.lineSpacing : source.lineSpacing;

        if (!destination.fontStyleFlags.hasValue())
        {
            destination.fontStyleFlags = source.fontStyleFlags;
        }
        else if (source.fontStyleFlags.hasValue())
        {
            for (const auto& entry : source.fontStyleFlags.get())
                destination.fontStyleFlags.get().emplace(entry);
        }

        destination.fill = destination.fill.hasValue() ? destination.fill : source.fill;
        destination.foreground = destination.foreground.hasValue() ? destination.foreground : source.foreground;
        destination.stroke = destination.stroke.hasValue() ? destination.stroke : source.stroke;
        destination.transitions = destination.transitions != nullptr ? destination.transitions : source.transitions;

        if (onlyInheretableStyles)
            return;

        destination.accent = destination.accent.hasValue() ? destination.accent : source.accent;
        destination.background = destination.background.hasValue() ? destination.background : source.background;
        destination.borderFill = destination.borderFill.hasValue() ? destination.borderFill : source.borderFill;
        destination.borderRadius = destination.borderRadius.hasValue() ? destination.borderRadius : source.borderRadius;
        destination.borderWidth = destination.borderWidth.hasValue() ? destination.borderWidth : source.borderWidth;
        destination.direction = destination.direction.hasValue() ? destination.direction : source.direction;
        destination.icon = destination.icon.hasValue() ? destination.icon : source.icon;
        destination.opacity = destination.opacity.hasValue() ? destination.opacity : source.opacity;
        destination.shadow = destination.shadow.hasValue() ? destination.shadow : source.shadow;
        destination.textAlign = destination.textAlign.hasValue() ? destination.textAlign : source.textAlign;
        destination.thumb = destination.thumb.hasValue() ? destination.thumb : source.thumb;
        destination.track = destination.track.hasValue() ? destination.track : source.track;
    }

    template <typename Styler>
    [[nodiscard]] static auto collectApplicableStylersOrdered(const std::vector<Styler>& stylers,
                                                              const juce::Component& component)
    {
        using GenerationOffset = std::uint16_t;

        std::vector<std::tuple<GenerationOffset, const Styles*, const StyleSelector*>> allApplicable;

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
                            &styler.styles,
                            &styler.selector));
                    break;
                }

                ++generationOffset;
            }
        }

        std::sort(std::begin(allApplicable),
                  std::end(allApplicable),
                  [](const auto& first, const auto& second) {
                      const auto& firstSelector = *std::get<const StyleSelector*>(first);
                      const auto& secondSelector = *std::get<const StyleSelector*>(second);

                      if (firstSelector == secondSelector)
                          return std::get<GenerationOffset>(first) < std::get<GenerationOffset>(second);

                      return firstSelector < secondSelector;
                  });

        return allApplicable;
    }

    template <typename Styler>
    [[nodiscard]] static auto buildStyles(const std::vector<Styler>& stylers,
                                          const juce::Component& component)
    {
        Styles result;

        for (const auto& [generationOffset, styles, selector] : collectApplicableStylersOrdered(stylers, component))
        {
            juce::ignoreUnused(selector);
            mergeLeft(result, *styles, generationOffset > 0);
        }

        return result;
    }

    template <typename Cache>
    [[nodiscard]] static auto& findOrAddCacheEntry(std::vector<Cache>& cache,
                                                   const juce::Component& component)
    {
        cache.erase(std::remove_if(std::begin(cache),
                                   std::end(cache),
                                   [](const auto& cacheEntry) {
                                       return cacheEntry.component == nullptr;
                                   }),
                    std::end(cache));

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
            {},
            0,
            false,
        });
    }

    [[nodiscard]] static auto getInteractionState(const juce::Component& component)
    {
        const auto* button = dynamic_cast<const juce::Button*>(&component);

        return (isMouseOverOrDragging(component) ? InteractionState::hover : 0)
             | (isMouseDown(component) ? InteractionState::active : 0)
             | (hasKeyboardFocus(component) ? InteractionState::focus : 0)
             | (component.isEnabled() ? 0 : InteractionState::disabled)
             | (button != nullptr && button->getToggleState() ? InteractionState::checked : 0);
    }

    [[nodiscard]] static std::optional<juce::Slider::SliderStyle> getSliderStyle(const juce::Component& component)
    {
        if (const auto* slider = dynamic_cast<const juce::Slider*>(&component))
            return slider->getSliderStyle();

        return std::nullopt;
    }

    [[nodiscard]] static auto getClasses(const juce::Component& component)
    {
        return component.getProperties()[ids::classes()];
    }

    LookAndFeel::SelectorInputs::SelectorInputs(const juce::Component& comp)
        : component{ &comp }
        , id{ comp.getComponentID() }
        , classes{ getClasses(comp) }
        , interactionState{ getInteractionState(comp) }
        , sliderStyle{ getSliderStyle(comp) }
    {
    }

    bool LookAndFeel::SelectorInputs::operator==(const SelectorInputs& other) const
    {
        return component == other.component
            && id == other.id
            && classes == other.classes
            && interactionState == other.interactionState
            && sliderStyle == other.sliderStyle;
    }

    // Copies the latest value of a style property into the persistent cached
    // entry. When the value has actually changed, this uses set() semantics so
    // that the previously-cached value is retained as the transition's starting
    // point and the property's last-updated time is reset to now. This is what
    // allows transitions to work even though the source styles are rebuilt from
    // scratch every time a style property changes.
    template <typename T>
    static void updateStyleProperty(Styles::StyleProperty<T>& destination,
                                    const Styles::StyleProperty<T>& source,
                                    Transitions* transitions)
    {
        if (!source.hasValue())
        {
            destination = source;
            return;
        }

        // If the value is changing while a transition is still in progress,
        // restart the transition from the current (interpolated) value so the
        // animation continues smoothly instead of jumping back to the old
        // target value.
        if (destination.hasValue()
            && !(destination == source)
            && transitions != nullptr)
        {
            if (auto* transition = (*transitions)[destination.id.toString()])
            {
                if (const auto progress = transition->calculateProgress(destination.getLastUpdated());
                    progress < 1.0)
                {
                    destination.setTransitioningFrom(destination.interpolated(progress),
                                                     source.get());
                    return;
                }
            }
        }

        destination.set(source.get());
    }

    static void update(Styles& destination, const Styles& source)
    {
        // Interruptions are calculated against the transition that's currently
        // governing the cached value (i.e. the existing one), so this is read
        // before the transitions themselves are updated below.
        auto* const transitions = destination.transitions.get();

        updateStyleProperty(destination.accent, source.accent, transitions);
        updateStyleProperty(destination.background, source.background, transitions);
        updateStyleProperty(destination.borderFill, source.borderFill, transitions);
        updateStyleProperty(destination.borderRadius, source.borderRadius, transitions);
        updateStyleProperty(destination.borderWidth, source.borderWidth, transitions);
        updateStyleProperty(destination.direction, source.direction, transitions);
        updateStyleProperty(destination.fill, source.fill, transitions);
#if JUCE_MAJOR_VERSION >= 8
        updateStyleProperty(destination.fontAscentOverride, source.fontAscentOverride, transitions);
        updateStyleProperty(destination.fontDescentOverride, source.fontDescentOverride, transitions);
#endif
        updateStyleProperty(destination.fontExtraKerningFactor, source.fontExtraKerningFactor, transitions);
        updateStyleProperty(destination.fontFamily, source.fontFamily, transitions);
        updateStyleProperty(destination.fontHorizontalScale, source.fontHorizontalScale, transitions);
        updateStyleProperty(destination.fontPointSize, source.fontPointSize, transitions);
        updateStyleProperty(destination.fontStyleFlags, source.fontStyleFlags, transitions);
        updateStyleProperty(destination.foreground, source.foreground, transitions);
        updateStyleProperty(destination.icon, source.icon, transitions);
        updateStyleProperty(destination.lineSpacing, source.lineSpacing, transitions);
        updateStyleProperty(destination.opacity, source.opacity, transitions);
        updateStyleProperty(destination.shadow, source.shadow, transitions);
        updateStyleProperty(destination.stroke, source.stroke, transitions);
        updateStyleProperty(destination.textAlign, source.textAlign, transitions);
        updateStyleProperty(destination.thumb, source.thumb, transitions);
        updateStyleProperty(destination.track, source.track, transitions);

        destination.transitions = source.transitions;
    }

    Styles LookAndFeel::findMostApplicableStyles(const juce::Component& component) const
    {
        auto& entry = findOrAddCacheEntry(stylesCache, component);

        // Checking every styler against the component and all its ancestors
        // is expensive, so it's only done when something a selector could
        // match against has changed since last time.
        std::vector<SelectorInputs> inputs;

        for (const auto* c = &component; c != nullptr; c = c->getParentComponent())
            inputs.emplace_back(*c);

        if (inputs != entry.resolvedFrom || stylersVersion != entry.resolvedFromStylersVersion)
        {
            update(entry.styles, buildStyles(stylers, component));
            entry.resolvedFrom = std::move(inputs);
            entry.resolvedFromStylersVersion = stylersVersion;
        }

        applyOpacity(entry);
        transitioningComponents.removeAllInstancesOf(nullptr);

        if (entry.styles.hasTransitionsInProgress())
            transitioningComponents.addIfNotAlreadyThere(const_cast<juce::Component*>(&component));
        else
            transitioningComponents.removeAllInstancesOf(const_cast<juce::Component*>(&component));

        return entry.styles;
    }

    // An alpha that wasn't set by a style is left alone.
    void LookAndFeel::applyOpacity(ComponentStylesCache& entry) const
    {
        const auto opacity = entry.styles.find<float>("opacity");

        if (opacity.has_value() || entry.appliedOpacity)
            entry.component->setAlpha(opacity.value_or(1.0f));

        entry.appliedOpacity = opacity.has_value();
        usesOpacity = usesOpacity || entry.appliedOpacity;
    }

    // JUCE reads a component's alpha before painting it, and doesn't paint it
    // at all with no alpha, so a component's opacity can't wait until it's
    // painted. Resolving every child's styles isn't free, so it's only done
    // once something has used an opacity.
    void LookAndFeel::applyChildOpacities(const juce::Component& component) const
    {
        if (!usesOpacity)
            return;

        for (auto* child : component.getChildren())
        {
            if (child->isVisible())
                juce::ignoreUnused(findMostApplicableStyles(*child));
        }
    }

    // Popups are painted from hidden stand-ins, so nothing repaints a popup
    // while their styles transition. A popup only repaints an item when its
    // highlight changes, so a transition would leave the colours it started
    // from on screen, such as the previous theme's after a theme change.
    [[nodiscard]] static Styles withoutTransitions(Styles styles)
    {
        styles.transitions = nullptr;
        return styles;
    }

    Styles LookAndFeel::findPopupStyles(const juce::ComboBox& comboBox) const
    {
        const juce::ScopedValueSetter ignoringInteractions{ ignoreInteractions, true };
        Styles styles;

        if (const auto* comboBoxComponent = dynamic_cast<const ComboBoxComponent*>(&comboBox))
            styles = findMostApplicableStyles(comboBoxComponent->getPopup());

        // The combo box's cached styles are the ones it's painted with, so
        // resolving it at rest here mustn't replace them.
        mergeLeft(styles, buildStyles(stylers, comboBox));
        return withoutTransitions(styles);
    }

    Styles LookAndFeel::findPopupItemStyles(const juce::ComboBox& comboBox,
                                            const juce::PopupMenu::Item& item,
                                            const InteractionState& interactionState) const
    {
        const juce::ScopedValueSetter ignoringInteractions{ ignoreInteractions, true };

        if (const auto* comboBoxComponent = dynamic_cast<const ComboBoxComponent*>(&comboBox))
        {
            if (auto* standIn = comboBoxComponent->findItem(item.itemID))
            {
                standIn->setHighlighted(interactionState.isHovered());
                return withoutTransitions(findMostApplicableStyles(*standIn));
            }
        }

        Styles styles;
        mergeLeft(styles, findPopupStyles(comboBox), true);
        return withoutTransitions(styles);
    }

    void LookAndFeel::onVBlankUpdate()
    {
        // The entries are safe-pointers that become null when their component is
        // deleted (e.g. when a view is rebuilt while a transition is still in
        // progress), so they must be null-checked before being dereferenced.
        for (auto& component : transitioningComponents)
        {
            if (component != nullptr)
                component->repaint();
        }

        // Painting usually takes a component off the list once its transitions
        // have finished, but not every component resolves its own styles when
        // it's painted, such as a spinner, whose buttons do the painting.
        transitioningComponents.removeIf([this](const auto& component) {
            const auto entry = std::find_if(std::begin(stylesCache),
                                            std::end(stylesCache),
                                            [&component](const auto& cacheEntry) {
                                                return cacheEntry.component.getComponent() == component.getComponent();
                                            });

            return component == nullptr
                || entry == std::end(stylesCache)
                || !entry->styles.hasTransitionsInProgress();
        });

        addAndRemoveShadows();
    }

    void LookAndFeel::addAndRemoveShadows()
    {
        for (const auto& component : std::exchange(componentsAwaitingShadowChanges, {}))
        {
            if (component == nullptr)
                continue;

            const auto styles = findMostApplicableStyles(*component);
            auto& properties = component->getProperties();

            if (!styles.shadow.hasValue())
            {
                properties.remove(ids::attachedShadowComponent());
                properties.remove(ids::shadowPath());
                continue;
            }

            if (!properties.contains(ids::attachedShadowComponent()))
            {
                properties.set(ids::attachedShadowComponent(),
                               new ReferenceCountedShadowComponent{
                                   std::make_unique<ShadowComponent>(*component),
                               });
            }

            updateShadow<juce::Component>(*component, styles);
        }
    }
} // namespace jive

namespace juce
{
    const Array<var> VariantConverter<jive::Theme>::options = {
        "juce",
        "steel",
        "material",
        "synthwave",
        "high-contrast",
    };

    jive::Theme VariantConverter<jive::Theme>::fromVar(const var& v)
    {
        if (const auto index = options.indexOf(v); index >= 0)
            return static_cast<jive::Theme>(index);

        jassertfalse;
        return jive::Theme::steel;
    }

    var VariantConverter<jive::Theme>::toVar(const jive::Theme& theme)
    {
        return options[static_cast<int>(theme)];
    }
} // namespace juce

#if JIVE_UNIT_TESTS
class LookAndFeelTest : public juce::UnitTest
{
public:
    LookAndFeelTest()
        : juce::UnitTest{ "jive::LookAndFeel", "jive" }
    {
    }

    void runTest() final
    {
        beginTest("vblank / safely handles a transitioning component being destroyed");
        {
            juce::Component parent;
            parent.setSize(100, 100);
            jive::LookAndFeel lookAndFeel{ parent };

            {
                juce::TextButton child;
                child.setBounds(0, 0, 50, 25);
                parent.addAndMakeVisible(child);

                // Resolving a component's styles registers it as transitioning
                // (the default theme applies a transition to every component),
                // adding it to the look-and-feel's list of components to repaint
                // on each vertical blank.
                juce::ignoreUnused(lookAndFeel.findMostApplicableStyles(child));
            }

            // The child has now been destroyed, so its entry in the
            // transitioning-components list is a null safe-pointer. The vblank
            // update must not dereference it. This used to crash when, for
            // example, a view was hot-reloaded while components were still
            // transitioning.
            lookAndFeel.simulateVBlank();
            expect(true);
        }

        beginTest("vblank / stops repainting a component once its transitions finish, even if it never resolves its styles again");
        {
            juce::Component parent;
            parent.setSize(100, 100);
            jive::LookAndFeel lookAndFeel{ parent };

            // A spinner is like this: its buttons paint, but it doesn't.
            juce::TextButton child;
            child.setBounds(0, 0, 50, 25);
            parent.addAndMakeVisible(child);
            juce::ignoreUnused(lookAndFeel.findMostApplicableStyles(child));

            lookAndFeel.simulateVBlank();
            expectEquals(lookAndFeel.getNumTransitioningComponents(), 1);

            jive::FakeTime::incrementTime(juce::RelativeTime::seconds(1.0));
            lookAndFeel.simulateVBlank();
            expectEquals(lookAndFeel.getNumTransitioningComponents(), 0);
        }

        testResolvedStylesAreReused();
        testComboBoxPopupFonts();
        testShadowShapes();
        testShadowsWaitForTheNextFrame();
        testValueDirections();
        testThemes();
    }

private:
    void testResolvedStylesAreReused()
    {
        beginTest("styles / resolving again with nothing changed doesn't check any selectors");
        {
            static auto numChecks = 0;
            static const auto predicate = jive::ComponentTypePredicates::addPredicate(
                [](const juce::Component&, const juce::String& typeName) {
                    if (typeName == "CountedType")
                        numChecks++;

                    return false;
                });
            juce::ignoreUnused(predicate);

            juce::Component parent;
            jive::LookAndFeel lookAndFeel{ parent };
            juce::Component child;
            parent.addAndMakeVisible(child);
            lookAndFeel.addStyles("CountedType", jive::Styles{});

            juce::ignoreUnused(lookAndFeel.findMostApplicableStyles(child));
            const auto numChecksAfterFirstResolve = numChecks;
            expectGreaterThan(numChecksAfterFirstResolve, 0);

            juce::ignoreUnused(lookAndFeel.findMostApplicableStyles(child));
            expectEquals(numChecks, numChecksAfterFirstResolve);

            child.getProperties().set("class", "changed");
            juce::ignoreUnused(lookAndFeel.findMostApplicableStyles(child));
            expectGreaterThan(numChecks, numChecksAfterFirstResolve);
        }

        beginTest("styles / are resolved again when an ancestor's class changes");
        {
            juce::Component parent;
            jive::LookAndFeel lookAndFeel{ parent };
            juce::Component child;
            parent.addAndMakeVisible(child);
            lookAndFeel.addStyles(".large", jive::Styles{}.withFontPointSize(30.0f));

            expectEquals(lookAndFeel.findMostApplicableStyles(child).fontPointSize.get(), 14.0f);

            parent.getProperties().set("class", "large");
            expectEquals(lookAndFeel.findMostApplicableStyles(child).fontPointSize.get(), 30.0f);
        }

        beginTest("styles / are resolved again when an ancestor's ID changes");
        {
            juce::Component parent;
            jive::LookAndFeel lookAndFeel{ parent };
            juce::Component child;
            parent.addAndMakeVisible(child);
            lookAndFeel.addStyles("#panel *", jive::Styles{}.withAccent(juce::Colours::red));

            expect(!lookAndFeel.findMostApplicableStyles(child).accent.hasValue());

            parent.setComponentID("panel");
            expect(lookAndFeel.findMostApplicableStyles(child).accent.get() == juce::Colours::red);
        }

        beginTest("styles / are resolved again when an ancestor is disabled");
        {
            juce::Component parent;
            jive::LookAndFeel lookAndFeel{ parent };
            juce::Component child;
            parent.addAndMakeVisible(child);
            lookAndFeel.addStyles(":disabled", jive::Styles{}.withAccent(juce::Colours::red));

            expect(!lookAndFeel.findMostApplicableStyles(child).accent.hasValue());

            parent.setEnabled(false);
            expect(lookAndFeel.findMostApplicableStyles(child).accent.get() == juce::Colours::red);
        }

        beginTest("styles / are resolved again when a component's interaction state changes");
        {
            juce::Component parent;
            jive::LookAndFeel lookAndFeel{ parent };
            juce::ToggleButton button;
            parent.addAndMakeVisible(button);
            lookAndFeel.addStyles(":checked", jive::Styles{}.withThumb(juce::Colours::red));
            lookAndFeel.addStyles(":focus", jive::Styles{}.withTrack(juce::Colours::green));
            lookAndFeel.addStyles(":hover", jive::Styles{}.withStroke(juce::Colours::blue));

            const auto styles = lookAndFeel.findMostApplicableStyles(button);
            expect(!styles.thumb.hasValue());
            expect(!styles.track.hasValue());
            expect(!styles.stroke.hasValue());

            button.setToggleState(true, juce::dontSendNotification);
            expect(lookAndFeel.findMostApplicableStyles(button).thumb.get() == juce::Colours::red);

            button.getProperties().set("jive::focus", true);
            expect(lookAndFeel.findMostApplicableStyles(button).track.get() == juce::Colours::green);

            button.getProperties().set("jive::hover", true);
            expect(lookAndFeel.findMostApplicableStyles(button).stroke.get() == juce::Colours::blue);
        }

        beginTest("styles / are resolved again when a slider's style changes");
        {
            juce::Component parent;
            jive::LookAndFeel lookAndFeel{ parent };
            juce::Slider slider{ juce::Slider::LinearHorizontal, juce::Slider::NoTextBox };
            parent.addAndMakeVisible(slider);
            lookAndFeel.addStyles("Knob", jive::Styles{}.withAccent(juce::Colours::red));

            expect(!lookAndFeel.findMostApplicableStyles(slider).accent.hasValue());

            slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
            expect(lookAndFeel.findMostApplicableStyles(slider).accent.get() == juce::Colours::red);
        }

        beginTest("styles / are resolved again when styles are added or removed");
        {
            juce::Component parent;
            jive::LookAndFeel lookAndFeel{ parent };
            juce::Component child;
            parent.addAndMakeVisible(child);

            expect(!lookAndFeel.findMostApplicableStyles(child).accent.hasValue());

            const auto id = lookAndFeel.addStyles("*", jive::Styles{}.withAccent(juce::Colours::red));
            expect(lookAndFeel.findMostApplicableStyles(child).accent.get() == juce::Colours::red);

            lookAndFeel.findStyles(id)->accent = juce::Colours::blue;
            expect(lookAndFeel.findMostApplicableStyles(child).accent.get() == juce::Colours::blue);

            lookAndFeel.removeStyles(id);
            expect(!lookAndFeel.findMostApplicableStyles(child).accent.hasValue());
        }
    }

    void testComboBoxPopupFonts()
    {
        beginTest("combo box popups / items use their combo box's font by default");
        {
            juce::Component parent;
            jive::LookAndFeel lookAndFeel{ parent };
            juce::ComboBox comboBox;
            parent.addAndMakeVisible(comboBox);
            comboBox.getProperties().set("font-size", 23.0f);

            const auto styles = jive::Styles{}
                                    .withFontFamily("Times New Roman")
                                    .withFontStyleFlags({ juce::Font::bold });
            const auto comboBoxFont = jive::getFont(comboBox, styles);
            const auto itemFont = jive::getFont(juce::PopupMenu::Item{}, comboBox, styles);
            expectEquals(itemFont.getTypefaceName(), comboBoxFont.getTypefaceName());
            expectEquals(itemFont.getHeightInPoints(), comboBoxFont.getHeightInPoints());
            expectEquals(itemFont.getStyleFlags(), comboBoxFont.getStyleFlags());
        }

        beginTest("combo box popups / item fonts can be overridden with popup-prefixed properties");
        {
            juce::Component parent;
            jive::LookAndFeel lookAndFeel{ parent };
            juce::ComboBox comboBox;
            parent.addAndMakeVisible(comboBox);
            comboBox.getProperties().set("popup.font-size", 30.0f);
            comboBox.getProperties().set("popup.font-styles", "bold italic");

            const auto styles = jive::Styles{}
                                    .withFontFamily("Times New Roman")
                                    .withFontStyleFlags({ juce::Font::bold });
            const auto itemFont = jive::getFont(juce::PopupMenu::Item{}, comboBox, styles);
            expectEquals(itemFont.getTypefaceName(), juce::String{ "Times New Roman" });
            expectEquals(itemFont.getHeightInPoints(), 30.0f);
            expectEquals(itemFont.getStyleFlags(), juce::Font::bold | juce::Font::italic);
        }

        beginTest("combo box popups / popups are styled through their stand-in, falling back to their combo box's styles");
        {
            juce::Component parent;
            jive::LookAndFeel lookAndFeel{ parent };
            jive::ComboBoxComponent comboBox;
            parent.addAndMakeVisible(comboBox);
            lookAndFeel.addStyles("ComboBox",
                                  jive::Styles{}
                                      .withBackground(juce::Colours::red)
                                      .withBorderWidth(juce::BorderSize{ 2.0f }));
            lookAndFeel.addStyles("Popup", jive::Styles{}.withBackground(juce::Colours::blue));

            const auto styles = lookAndFeel.findPopupStyles(comboBox);
            expect(styles.background.get() == juce::Colours::blue);
            expect(styles.borderWidth.get() == juce::BorderSize{ 2.0f });
        }

        beginTest("combo box popups / popups and items don't transition, as nothing repaints them while they would");
        {
            juce::Component parent;
            jive::LookAndFeel lookAndFeel{ parent };
            jive::ComboBoxComponent comboBox;
            parent.addAndMakeVisible(comboBox);
            jive::ComboBoxComponent::Item standIn{ 1 };
            comboBox.getPopup().addChildComponent(standIn);
            juce::PopupMenu::Item item{ "One" };
            item.itemID = 1;

            juce::ignoreUnused(lookAndFeel.findPopupStyles(comboBox),
                               lookAndFeel.findPopupItemStyles(comboBox, item, {}));
            lookAndFeel.applyTheme(jive::Theme::material);

            const auto foreground = jive::parseColour(jive::themes::material::onSurface);
            expect(lookAndFeel.findPopupStyles(comboBox).find<jive::Fill>("foreground").value() == foreground);
            expect(lookAndFeel.findPopupItemStyles(comboBox, item, {}).find<jive::Fill>("foreground").value() == foreground);
        }

        beginTest("combo box popups / popups and items are styled as though their combo box were at rest");
        {
            juce::Component parent;
            jive::LookAndFeel lookAndFeel{ parent };
            jive::ComboBoxComponent comboBox;
            parent.addAndMakeVisible(comboBox);
            jive::ComboBoxComponent::Item standIn{ 1 };
            comboBox.getPopup().addChildComponent(standIn);
            juce::PopupMenu::Item item{ "One" };
            item.itemID = 1;
            lookAndFeel.addStyles("ComboBox",
                                  jive::Styles{}
                                      .withBackground(juce::Colours::red)
                                      .withForeground(juce::Colours::red));
            lookAndFeel.addStyles("ComboBox:hover:active",
                                  jive::Styles{}
                                      .withBackground(juce::Colours::blue)
                                      .withForeground(juce::Colours::blue));
            comboBox.getProperties().set("jive::hover", true);
            comboBox.getProperties().set("jive::active", true);

            expect(lookAndFeel.findPopupStyles(comboBox).background.get() == juce::Colours::red);
            expect(lookAndFeel.findPopupItemStyles(comboBox, item, {}).foreground.get() == juce::Colours::red);
            expect(lookAndFeel.findMostApplicableStyles(comboBox).background.get() == juce::Colours::blue);
        }

        beginTest("combo box popups / items are styled through their stand-ins, inheriting from their popup");
        {
            juce::Component parent;
            jive::LookAndFeel lookAndFeel{ parent };
            jive::ComboBoxComponent comboBox;
            parent.addAndMakeVisible(comboBox);
            jive::ComboBoxComponent::Item standIn{ 2 };
            standIn.getProperties().set("class", "georgia");
            comboBox.getPopup().addChildComponent(standIn);
            lookAndFeel.addStyles("Popup",
                                  jive::Styles{}
                                      .withBackground(juce::Colours::blue)
                                      .withFontPointSize(30.0f));
            lookAndFeel.addStyles(".georgia", jive::Styles{}.withFontFamily("Georgia"));

            juce::PopupMenu::Item item{ "Two" };
            item.itemID = 2;
            const auto styles = lookAndFeel.findPopupItemStyles(comboBox, item, {});
            expect(!styles.background.hasValue());

            const auto font = jive::getFont(item, comboBox, styles);
            expectEquals(font.getTypefaceName(), juce::String{ "Georgia" });
            expectEquals(font.getHeightInPoints(), 30.0f);
        }

        beginTest("combo box popups / highlighted items are styled as hovered");
        {
            juce::Component parent;
            jive::LookAndFeel lookAndFeel{ parent };
            jive::ComboBoxComponent comboBox;
            parent.addAndMakeVisible(comboBox);
            jive::ComboBoxComponent::Item standIn{ 1 };
            comboBox.getPopup().addChildComponent(standIn);
            lookAndFeel.addStyles("Option:hover", jive::Styles{}.withBackground(juce::Colours::green));

            juce::PopupMenu::Item item{ "One" };
            item.itemID = 1;
            expect(lookAndFeel.findPopupItemStyles(comboBox, item, jive::InteractionState::hover).background.get() == juce::Colours::green);
            expect(!lookAndFeel.findPopupItemStyles(comboBox, item, {}).background.hasValue());
        }

        beginTest("combo box popups / items without stand-ins only inherit their popup's styles");
        {
            juce::Component parent;
            jive::LookAndFeel lookAndFeel{ parent };
            juce::ComboBox comboBox;
            parent.addAndMakeVisible(comboBox);
            lookAndFeel.addStyles("ComboBox",
                                  jive::Styles{}
                                      .withBackground(juce::Colours::red)
                                      .withFontFamily("Georgia"));

            const auto styles = lookAndFeel.findPopupItemStyles(comboBox, juce::PopupMenu::Item{ "One" }, {});
            expect(!styles.background.hasValue());
            expectEquals(styles.fontFamily.get(), juce::String{ "Georgia" });
        }

        beginTest("combo box popups / items are measured with their own fonts");
        {
            juce::Component parent;
            jive::LookAndFeel lookAndFeel{ parent };
            jive::ComboBoxComponent comboBox;
            parent.addAndMakeVisible(comboBox);
            comboBox.addItem("One", 1);
            comboBox.addItem("Two", 2);
            jive::ComboBoxComponent::Item standIn{ 1 };
            standIn.getProperties().set("class", "large");
            comboBox.getPopup().addChildComponent(standIn);
            lookAndFeel.addStyles(".large", jive::Styles{}.withFontPointSize(40.0f));

            const auto options = juce::PopupMenu::Options{}.withTargetComponent(&comboBox);
            auto largeWidth = 0;
            auto largeHeight = 0;
            lookAndFeel.getIdealPopupMenuItemSizeWithOptions("One", false, 0, largeWidth, largeHeight, options);
            auto regularWidth = 0;
            auto regularHeight = 0;
            lookAndFeel.getIdealPopupMenuItemSizeWithOptions("Two", false, 0, regularWidth, regularHeight, options);

            expectGreaterThan(largeWidth, regularWidth);
            expectGreaterThan(largeHeight, regularHeight);
        }
    }

    void testShadowShapes()
    {
        beginTest("shadows / check-boxes cast their shadow from their rounded box, in their own coordinates");
        {
            juce::Component parent;
            parent.setSize(200, 100);
            jive::LookAndFeel lookAndFeel{ parent };

            juce::ToggleButton checkbox;
            checkbox.setBounds(40, 30, 100, 30);
            parent.addAndMakeVisible(checkbox);

            juce::ignoreUnused(checkbox.createComponentSnapshot(checkbox.getLocalBounds()));

            const auto styles = lookAndFeel.findMostApplicableStyles(checkbox);
            const auto boxBounds = lookAndFeel.getToggleButtonCheckboxBounds(checkbox, styles);
            const auto radius = jive::getBorderRadius(checkbox, styles);
            expect(radius.topLeft > 0.0f);
            expectEquals(checkbox.getProperties()[jive::ids::shadowPath()].toString(),
                         jive::getShape(radius, boxBounds.toFloat()).toString());
        }

        beginTest("shadows / group components cast their shadow from their rounded outline, in their own coordinates");
        {
            juce::Component parent;
            parent.setSize(200, 100);
            jive::LookAndFeel lookAndFeel{ parent };

            juce::GroupComponent group{ {}, "Title" };
            group.setBounds(40, 30, 100, 60);
            parent.addAndMakeVisible(group);
            lookAndFeel.addStyles(jive::StyleSelector{ "*" },
                                  jive::Styles{}
                                      .withBorderRadius(jive::BorderRadii<float>{ 5.0f })
                                      .withShadow(jive::Shadow{}.setColour(juce::Colours::black)));

            juce::ignoreUnused(group.createComponentSnapshot(group.getLocalBounds()));

            const auto styles = lookAndFeel.findMostApplicableStyles(group);
            expectEquals(group.getProperties()[jive::ids::shadowPath()].toString(),
                         jive::getShape(jive::getBorderRadius(group, styles),
                                        jive::getGroupBounds(group, styles))
                             .toString());
        }
    }

    // A component's shadow is a child of one of its ancestors, which may be
    // painting its children when the component asks for the shadow.
    void testShadowsWaitForTheNextFrame()
    {
        juce::Component parent;
        parent.setComponentID("parent");
        parent.setSize(200, 100);
        jive::LookAndFeel lookAndFeel{ parent };
        lookAndFeel.applyTheme(jive::Theme::juce);
        lookAndFeel.addStyles("#parent", jive::Styles{}.withBackground(juce::Colours::black));

        juce::TextButton button;
        button.setComponentID("button");
        button.setBounds(40, 30, 100, 30);
        parent.addAndMakeVisible(button);

        beginTest("shadows / painting a component adds its shadow on the next frame");
        {
            const auto shadowStyles = lookAndFeel.addStyles("#button",
                                                            jive::Styles{}.withShadow(jive::Shadow{}.setColour(juce::Colours::black)));

            juce::ignoreUnused(parent.createComponentSnapshot(parent.getLocalBounds()));
            expectEquals(parent.getNumChildComponents(), 1);

            lookAndFeel.simulateVBlank();
            expectEquals(parent.getNumChildComponents(), 2);

            lookAndFeel.removeStyles(shadowStyles);
        }

        beginTest("shadows / painting a component that no longer has a shadow removes it on the next frame");
        {
            juce::ignoreUnused(parent.createComponentSnapshot(parent.getLocalBounds()));
            expectEquals(parent.getNumChildComponents(), 2);

            lookAndFeel.simulateVBlank();
            expectEquals(parent.getNumChildComponents(), 1);
        }
    }

    void testValueDirections()
    {
        beginTest("value directions / vertical sliders have their minimum at the bottom");
        {
            juce::Component parent;
            parent.setSize(100, 200);
            jive::LookAndFeel lookAndFeel{ parent };
            lookAndFeel.applyTheme(jive::Theme::juce);

            juce::Slider slider{ juce::Slider::LinearVertical, juce::Slider::NoTextBox };
            slider.setComponentID("slider");
            slider.setBounds(0, 0, 30, 150);
            parent.addAndMakeVisible(slider);
            lookAndFeel.addStyles("#slider",
                                  jive::Styles{}
                                      .withBackground(juce::Colours::blue)
                                      .withTrack(juce::Colours::green)
                                      .withThumb(juce::Colours::red));

            slider.setValue(slider.getMinimum());
            auto snapshot = slider.createComponentSnapshot(slider.getLocalBounds());
            expect(snapshot.getPixelAt(15, 138) == juce::Colours::red);
            expect(snapshot.getPixelAt(15, 12) == juce::Colours::blue);

            slider.setValue(slider.getMaximum());
            snapshot = slider.createComponentSnapshot(slider.getLocalBounds());
            expect(snapshot.getPixelAt(15, 12) == juce::Colours::red);
            expect(snapshot.getPixelAt(15, 75) == juce::Colours::green);
        }

        beginTest("value directions / bar sliders fill from their minimum");
        {
            juce::Component parent;
            parent.setSize(200, 200);
            jive::LookAndFeel lookAndFeel{ parent };
            lookAndFeel.applyTheme(jive::Theme::juce);

            juce::Slider slider{ juce::Slider::LinearBar, juce::Slider::NoTextBox };
            slider.setComponentID("slider");
            slider.setBounds(0, 0, 100, 20);
            slider.setRange(0.0, 1.0);
            slider.setValue(0.25);
            parent.addAndMakeVisible(slider);
            lookAndFeel.addStyles("#slider",
                                  jive::Styles{}
                                      .withBackground(juce::Colours::blue)
                                      .withTrack(juce::Colours::green));

            auto snapshot = slider.createComponentSnapshot(slider.getLocalBounds());
            expect(snapshot.getPixelAt(10, 10) == juce::Colours::green);
            expect(snapshot.getPixelAt(50, 10) == juce::Colours::blue);

            slider.setSliderStyle(juce::Slider::LinearBarVertical);
            slider.setBounds(0, 0, 20, 100);
            snapshot = slider.createComponentSnapshot(slider.getLocalBounds());
            expect(snapshot.getPixelAt(10, 90) == juce::Colours::green);
            expect(snapshot.getPixelAt(10, 50) == juce::Colours::blue);
        }

        beginTest("value directions / vertical progress bars fill from the bottom");
        {
            juce::Component parent;
            parent.setSize(100, 200);
            jive::LookAndFeel lookAndFeel{ parent };
            lookAndFeel.applyTheme(jive::Theme::juce);

            juce::ProgressBar progressBar{ progress, juce::ProgressBar::Style::linear };
            progressBar.setComponentID("progress-bar");
            progressBar.setPercentageDisplay(false);
            progressBar.setBounds(0, 0, 20, 100);
            parent.addAndMakeVisible(progressBar);
            lookAndFeel.addStyles("#progress-bar",
                                  jive::Styles{}
                                      .withBackground(juce::Colours::blue)
                                      .withTrack(juce::Colours::green));

            const auto snapshot = progressBar.createComponentSnapshot(progressBar.getLocalBounds());
            expect(snapshot.getPixelAt(10, 75) == juce::Colours::green);
            expect(snapshot.getPixelAt(10, 25) == juce::Colours::blue);
        }
    }

    // Theme changes transition, so this settles them before reading the
    // background.
    static auto settledStyles(const jive::LookAndFeel& lookAndFeel, const juce::Component& component)
    {
        juce::ignoreUnused(lookAndFeel.findMostApplicableStyles(component));
        jive::FakeTime::incrementTime(juce::RelativeTime::seconds(1.0));

        return lookAndFeel.findMostApplicableStyles(component);
    }

    static auto settledBackground(const jive::LookAndFeel& lookAndFeel, const juce::Component& component)
    {
        return settledStyles(lookAndFeel, component)
            .find<jive::Fill>("background")
            .value_or(jive::Fill{});
    }

    static auto settledForeground(const jive::LookAndFeel& lookAndFeel, const juce::Component& component)
    {
        return settledStyles(lookAndFeel, component)
            .find<jive::Fill>("foreground")
            .value_or(jive::Fill{});
    }

    void testThemes()
    {
        beginTest("themes / are named like any other keyword value");
        {
            expect(juce::VariantConverter<jive::Theme>::fromVar("material") == jive::Theme::material);
            expect(juce::VariantConverter<jive::Theme>::fromVar("high-contrast") == jive::Theme::highContrast);
            expectEquals(juce::VariantConverter<jive::Theme>::toVar(jive::Theme::synthwave).toString(), juce::String{ "synthwave" });
        }

        beginTest("themes / applying a theme replaces the previous theme's styles");
        {
            juce::Component parent;
            jive::LookAndFeel lookAndFeel{ parent };
            juce::TextButton button;
            parent.addAndMakeVisible(button);
            expect(settledBackground(lookAndFeel, button) == jive::parseColour(jive::themes::steel::raisinBlack));

            lookAndFeel.applyTheme(jive::Theme::material);
            expect(settledBackground(lookAndFeel, button) == jive::parseColour(jive::themes::material::secondaryContainer));
        }

        beginTest("themes / applying a theme keeps styles that didn't come from a theme");
        {
            juce::Component parent;
            jive::LookAndFeel lookAndFeel{ parent };
            juce::TextButton button;
            button.setComponentID("kept");
            parent.addAndMakeVisible(button);
            lookAndFeel.addStyles("#kept", jive::Styles{}.withBackground(juce::Colours::red));

            lookAndFeel.applyTheme(jive::Theme::synthwave);
            expect(settledBackground(lookAndFeel, button) == juce::Colours::red);
        }

        beginTest("themes / every theme styles windows, text and widgets");
        {
            for (const auto theme : { jive::Theme::steel, jive::Theme::material, jive::Theme::synthwave, jive::Theme::highContrast })
            {
                juce::Component parent;
                jive::LookAndFeel lookAndFeel{ parent };
                lookAndFeel.applyTheme(theme);

                juce::TextButton button;
                juce::ToggleButton checkbox;
                juce::Slider slider;
                juce::Slider knob{ juce::Slider::RotaryVerticalDrag, juce::Slider::NoTextBox };
                juce::ComboBox comboBox;
                parent.addAndMakeVisible(button);
                parent.addAndMakeVisible(checkbox);
                parent.addAndMakeVisible(slider);
                parent.addAndMakeVisible(knob);
                parent.addAndMakeVisible(comboBox);

                for (const auto* component : std::initializer_list<const juce::Component*>{ &button, &checkbox, &slider, &knob, &comboBox })
                {
                    const auto styles = lookAndFeel.findMostApplicableStyles(*component);
                    expect(styles.background.hasValue());
                    expect(styles.foreground.hasValue());
                    expect(styles.fontFamily.hasValue());
                }

                for (const auto* component : std::initializer_list<const juce::Component*>{ &slider, &knob })
                {
                    expect(lookAndFeel.findMostApplicableStyles(*component).thumb.hasValue());
                    expect(lookAndFeel.findMostApplicableStyles(*component).track.hasValue());
                }
            }
        }

        beginTest("themes / every theme underlines hyperlinks in an accent colour that changes on hover and press");
        {
            for (const auto theme : { jive::Theme::steel, jive::Theme::material, jive::Theme::synthwave, jive::Theme::highContrast })
            {
                juce::Component parent;
                jive::LookAndFeel lookAndFeel{ parent };
                lookAndFeel.applyTheme(theme);

                juce::HyperlinkButton hyperlink;
                jive::TextComponent text;
                parent.addAndMakeVisible(hyperlink);
                hyperlink.addAndMakeVisible(text);

                const auto styles = settledStyles(lookAndFeel, text);
                expect(styles.fontStyleFlags.hasValue() && styles.fontStyleFlags.get().count(juce::Font::underlined) > 0);

                const auto atRest = settledForeground(lookAndFeel, text);
                expect(atRest != settledForeground(lookAndFeel, parent));

                hyperlink.getProperties().set("jive::hover", true);
                const auto hovered = settledForeground(lookAndFeel, text);
                expect(hovered != atRest);

                hyperlink.getProperties().set("jive::active", true);
                const auto pressed = settledForeground(lookAndFeel, text);
                expect(pressed != hovered);
                expect(pressed != atRest);
            }
        }

        beginTest("themes / synthwave's button text colour doesn't apply to checkboxes or hyperlinks");
        {
            juce::Component parent;
            jive::LookAndFeel lookAndFeel{ parent };
            lookAndFeel.applyTheme(jive::Theme::synthwave);

            juce::ToggleButton checkbox;
            juce::HyperlinkButton hyperlink;
            jive::TextComponent text;
            parent.addAndMakeVisible(checkbox);
            parent.addAndMakeVisible(hyperlink);
            hyperlink.addAndMakeVisible(text);

            expect(settledForeground(lookAndFeel, checkbox) == jive::parseColour(jive::themes::synthwave::haze));
            expect(settledForeground(lookAndFeel, text) == jive::parseColour(jive::themes::synthwave::neonCyan));
        }

        beginTest("themes / material is flat, with no borders or shadows");
        {
            juce::Component parent;
            jive::LookAndFeel lookAndFeel{ parent };
            lookAndFeel.applyTheme(jive::Theme::material);

            juce::TextButton button;
            juce::ToggleButton checkbox;
            juce::Slider slider;
            juce::ComboBox comboBox;
            juce::ProgressBar progressBar{ progress };
            parent.addAndMakeVisible(button);
            parent.addAndMakeVisible(checkbox);
            parent.addAndMakeVisible(slider);
            parent.addAndMakeVisible(comboBox);
            parent.addAndMakeVisible(progressBar);

            button.setToggleState(true, juce::dontSendNotification);
            checkbox.setToggleState(true, juce::dontSendNotification);

            for (const auto* component : std::initializer_list<const juce::Component*>{ &button, &checkbox, &slider, &comboBox, &progressBar })
            {
                const auto styles = lookAndFeel.findMostApplicableStyles(*component);
                expect(!styles.shadow.hasValue());
                expect(styles.borderWidth.hasValue() && styles.borderWidth.get().isEmpty());
            }
        }
    }

    double progress = 0.5;
};

static LookAndFeelTest lookAndFeelTest;
#endif
