#pragma once

#include "jive_DefaultComponentStyles.h"

#include <jive_core/geometry/jive_BorderRadii.h>
#include <jive_core/geometry/jive_BorderWidth.h>
#include <jive_core/graphics/jive_FontUtilities.h>

#include <juce_gui_basics/juce_gui_basics.h>

namespace jive::default_styles
{
    // The space between a popup's border and its items, and around the content
    // of each item, is based on the items' font so that it scales with their
    // text.
    [[nodiscard]] static auto getPopupMenuPadding(const juce::Font& font)
    {
        return std::round(font.getHeight() * 0.25f);
    }

    [[nodiscard]] static auto getPopupMenuItemPadding(const juce::Font& font)
    {
        const auto vertical = std::round(font.getHeight() * 0.25f);
        const auto horizontal = std::round(font.getHeight() * 0.375f);
        return juce::BorderSize<float>{ vertical, horizontal, vertical, horizontal };
    }

    [[nodiscard]] static auto getPopupMenuItemGap(const juce::Font& font)
    {
        return std::round(font.getHeight() * 0.25f);
    }

    [[nodiscard]] static auto getPopupMenuItemIconSize(const juce::Font& font)
    {
        return std::round(font.getHeight());
    }

    [[nodiscard]] static auto getPopupMenuBorderSize(const juce::PopupMenu& popup,
                                                     const Styles& styles,
                                                     const juce::Font& font)
    {
        const auto borderWidth = getBorderWidth(popup, styles);
        const auto thickestBorder = std::max({ borderWidth.getTop(), borderWidth.getLeft(), borderWidth.getBottom(), borderWidth.getRight() });

        return static_cast<int>(std::ceil(thickestBorder + getPopupMenuPadding(font)));
    }

    // Items are spaced using the popup's font, so their text lines up, but
    // their text is in their own font.
    [[nodiscard]] static auto getIdealPopupMenuItemSize(const juce::String& text,
                                                        const juce::Font& font,
                                                        const juce::Font& textFont)
    {
        const auto padding = getPopupMenuItemPadding(font);
        const auto iconSize = getPopupMenuItemIconSize(font);
        const juce::Rectangle<float> content{
            iconSize + getPopupMenuItemGap(font) + std::ceil(calculateStringWidth(text, textFont)),
            std::max(iconSize, std::ceil(textFont.getHeight())),
        };

        return padding.addedTo(content).withZeroOrigin();
    }

    [[nodiscard]] static auto paintPopupMenu(juce::Graphics& g,
                                             const juce::PopupMenu& popup,
                                             const juce::PopupMenu::Options& options,
                                             juce::Rectangle<float> bounds,
                                             const Styles& styles)
    {
        drawRoundedRectangle(g,
                             bounds,
                             getBorderRadius(popup, styles),
                             getBorderWidth(popup, styles),
                             getBackgroundFill(popup, options, styles),
                             getBorderFill(popup, options, styles));
    }

    [[nodiscard]] static auto paintPopupMenuSeparator(juce::Graphics& g,
                                                      const juce::PopupMenu::Item& item,
                                                      const juce::PopupMenu::Options& options,
                                                      const Styles& styles,
                                                      juce::Rectangle<float> bounds)
    {
        const auto r = bounds.withSizeKeepingCentre(bounds.getWidth(), 1.0f);

        g.setFillType(getBorderFill(item, options, styles, bounds));
        g.fillRect(r.toNearestInt());
    }

    [[nodiscard]] static auto paintPopupMenuTextItem(juce::Graphics& g,
                                                     const juce::PopupMenu::Item& item,
                                                     const juce::PopupMenu::Options& options,
                                                     const juce::Component& component,
                                                     const Styles& popupStyles,
                                                     const Styles& itemStyles,
                                                     const InteractionState& interactionState,
                                                     juce::Rectangle<float> bounds)
    {
        g.setFillType(getForegroundFill(item, options, itemStyles, interactionState, bounds));

        const auto font = getFont(juce::PopupMenu::Item{}, component, popupStyles);
        const auto fontAscent = font.getAscent();

        auto r = getPopupMenuItemPadding(font).subtractedFrom(bounds);
        const auto iconSize = getPopupMenuItemIconSize(font);
        const auto iconArea = r.removeFromLeft(iconSize).withSizeKeepingCentre(iconSize, iconSize);
        r.removeFromLeft(getPopupMenuItemGap(font));

        if (item.image != nullptr)
        {
            item.image->drawWithin(g, iconArea, juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize, 1.0f);
        }
        else if (item.isTicked)
        {
            const auto tick = component.getLookAndFeel().getTickShape(1.0f);
            g.fillPath(tick, tick.getTransformToScaleToFit(iconArea.reduced(iconSize * 0.15f), true));
        }

        if (item.subMenu != nullptr)
        {
            const auto arrowH = 0.6f * fontAscent;
            const auto x = r.removeFromRight(std::floor(arrowH)).getX();
            const auto halfH = r.getCentreY();

            juce::Path path;
            path.startNewSubPath(x, halfH - (arrowH * 0.5f));
            path.lineTo(x + (arrowH * 0.6f), halfH);
            path.lineTo(x, halfH + (arrowH * 0.5f));

            g.strokePath(path, juce::PathStrokeType{ 2.0f });
            r.removeFromRight(getPopupMenuItemGap(font));
        }

        g.setFont(getFont(item, component, itemStyles));
        g.drawFittedText(item.text, r.toNearestInt(), juce::Justification::centredLeft, 1);

        if (item.shortcutKeyDescription.isNotEmpty())
        {
            auto f2 = font;
            f2.setHeight(f2.getHeight() * 0.75f);
            f2.setHorizontalScale(0.95f);
            g.setFont(f2);

            g.drawText(item.shortcutKeyDescription, r.toNearestInt(), juce::Justification::centredRight, true);
        }
    }

    [[nodiscard]] static auto paintPopupMenuItem(juce::Graphics& g,
                                                 const juce::PopupMenu::Item& item,
                                                 const juce::PopupMenu& popup,
                                                 const juce::PopupMenu::Options& options,
                                                 const juce::Component& component,
                                                 juce::Rectangle<float> bounds,
                                                 const InteractionState& interactionState,
                                                 const Styles& popupStyles,
                                                 const Styles& itemStyles)
    {
        // JUCE only insets items vertically from the popup's border, so they
        // need insetting horizontally here.
        const auto inset = static_cast<float>(getPopupMenuBorderSize(popup, popupStyles, getFont(juce::PopupMenu::Item{}, component, popupStyles)));
        const auto itemBounds = bounds.reduced(inset, 0.0f);

        drawRoundedRectangle(g,
                             itemBounds,
                             getBorderRadius(item, popup, popupStyles),
                             juce::BorderSize<float>{},
                             getBackgroundFill(item, options, itemStyles, interactionState, itemBounds),
                             juce::Colours::transparentBlack);

        if (item.isSeparator)
        {
            paintPopupMenuSeparator(g, item, options, popupStyles, itemBounds);
        }
        else
        {
            paintPopupMenuTextItem(g, item, options, component, popupStyles, itemStyles, interactionState, itemBounds);
        }
    }

    // The space around a combo box's content, and between its text and its
    // icon, is based on its font so that it scales with its text.
    [[nodiscard]] static auto getComboBoxPadding(const juce::Font& font)
    {
        const auto vertical = std::round(font.getHeight() * 0.125f);
        const auto horizontal = std::round(font.getHeight() * 0.5f);
        return juce::BorderSize<float>{ vertical, horizontal, vertical, horizontal };
    }

    [[nodiscard]] static auto getComboBoxGap(const juce::Font& font)
    {
        return std::round(font.getHeight() * 0.25f);
    }

    // Icons are as tall as the text, unless there isn't room for them.
    [[nodiscard]] static auto getComboBoxIconSize(const juce::Font& font,
                                                  const Styles& styles,
                                                  float maxHeight = std::numeric_limits<float>::max())
    {
        const auto height = std::min(std::round(font.getHeight()), maxHeight);
        const auto aspectRatio = styles.icon.hasValue() ? styles.icon.get().getAspectRatio() : 1.0f;
        return juce::Point{ std::ceil(height * aspectRatio), height };
    }

    [[nodiscard]] static auto getComboBoxContentBounds(const juce::ComboBox& comboBox,
                                                       const Styles& styles,
                                                       const juce::Font& font)
    {
        return getComboBoxPadding(font).subtractedFrom(getBorderWidth(comboBox, styles)
                                                           .subtractedFrom(comboBox.getLocalBounds().toFloat()));
    }

    [[nodiscard]] static auto getComboBoxIconBounds(const juce::ComboBox& comboBox,
                                                    const Styles& styles)
    {
        const auto font = getFont(comboBox, styles);
        const auto maxHeight = getBorderWidth(comboBox, styles)
                                   .subtractedFrom(comboBox.getLocalBounds().toFloat())
                                   .getHeight();
        const auto size = getComboBoxIconSize(font, styles, maxHeight);

        return getComboBoxContentBounds(comboBox, styles, font)
            .removeFromRight(size.x)
            .withSizeKeepingCentre(size.x, size.y);
    }

    [[nodiscard]] static auto getComboBoxTextBounds(const juce::ComboBox& comboBox,
                                                    const Styles& styles)
    {
        const auto font = getFont(comboBox, styles);
        auto bounds = getComboBoxContentBounds(comboBox, styles, font);
        bounds.removeFromRight(getComboBoxIconBounds(comboBox, styles).getWidth() + getComboBoxGap(font));

        return bounds.withSizeKeepingCentre(bounds.getWidth(), font.getHeight());
    }

    [[nodiscard]] static auto getIdealComboBoxSize(const juce::ComboBox& comboBox,
                                                   const Styles& styles)
    {
        const auto font = getFont(comboBox, styles);
        auto textWidth = calculateStringWidth(comboBox.getTextWhenNothingSelected(), font);

        for (auto i = 0; i < comboBox.getNumItems(); ++i)
            textWidth = std::max(textWidth, calculateStringWidth(comboBox.getItemText(i), font));

        const auto iconSize = getComboBoxIconSize(font, styles);
        const juce::Rectangle<float> content{
            std::ceil(textWidth) + getComboBoxGap(font) + iconSize.x,
            std::max(std::ceil(font.getHeight()), iconSize.y),
        };

        return getBorderWidth(comboBox, styles)
            .addedTo(getComboBoxPadding(font).addedTo(content))
            .withZeroOrigin();
    }

    [[nodiscard]] static auto paintComboBox(juce::Graphics& g,
                                            const juce::ComboBox& comboBox,
                                            const Styles& styles)
    {
        paintComponent(g, comboBox, styles);

        const auto iconBounds = getComboBoxIconBounds(comboBox, styles);

        if (styles.icon.hasValue() && !styles.icon.get().isEmpty())
        {
            styles.icon.get().draw(g,
                                   iconBounds,
                                   getArrowFill(comboBox, styles).colour,
                                   findFill(comboBox, styles),
                                   findStroke(comboBox, styles));
            return;
        }

        const auto arrowBounds = iconBounds.withSizeKeepingCentre(iconBounds.getWidth() * 0.65f,
                                                                  iconBounds.getHeight() * 0.65f);
        juce::Path path;
        path.startNewSubPath(arrowBounds.getX(), arrowBounds.getY() + arrowBounds.proportionOfHeight(0.275f));
        path.lineTo(arrowBounds.getCentreX(), arrowBounds.getBottom() - arrowBounds.proportionOfHeight(0.225f));
        path.lineTo(arrowBounds.getRight(), arrowBounds.getY() + arrowBounds.proportionOfHeight(0.275f));

        g.setFillType(getArrowFill(comboBox, styles));
        g.strokePath(path,
                     juce::PathStrokeType{
                         2.0f,
                         juce::PathStrokeType::mitered,
                         juce::PathStrokeType::rounded,
                     });
    }
} // namespace jive::default_styles
