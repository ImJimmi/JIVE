#pragma once

#include "jive_DefaultComponentStyles.h"

#include <jive_core/geometry/jive_BorderRadii.h>
#include <jive_core/geometry/jive_BorderWidth.h>
#include <jive_core/graphics/jive_FontUtilities.h>

#include <juce_gui_basics/juce_gui_basics.h>

namespace jive::default_styles
{
    [[nodiscard]] static auto paintPopupMenu(juce::Graphics& g,
                                             const juce::PopupMenu& popup,
                                             const juce::PopupMenu::Options& options,
                                             const juce::Rectangle<float>& bounds,
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
                                                      const juce::Rectangle<float>& bounds)
    {
        auto r = bounds.reduced(5.0f, 0.0f);
        r.removeFromTop(std::round((r.getHeight() * 0.5f) - 0.5f));

        g.setFillType(getBorderFill(item, options, styles, bounds));
        g.fillRect(r.toNearestInt().removeFromTop(1));
    }

    [[nodiscard]] static auto paintPopupMenuTextItem(juce::Graphics& g,
                                                     const juce::PopupMenu::Item& item,
                                                     const juce::PopupMenu::Options& options,
                                                     const juce::Component& component,
                                                     const Styles& styles,
                                                     const InteractionState& interactionState,
                                                     const juce::Rectangle<float>& bounds)
    {
        g.setFillType(getForegroundFill(item, options, styles, interactionState, bounds));
        auto r = bounds.reduced(juce::jmin(5.0f, bounds.getWidth() / 20.0f) + 1.0f, 1.0f);

        auto font = getFont(item, component, styles);
        const auto fontAscent = font.getAscent();
        const auto maxFontHeight = r.getHeight() / 1.3f;
        font.setHeight(juce::jmin(font.getHeight(), maxFontHeight));
        g.setFont(font);

        const auto iconArea = r.removeFromLeft(std::round(maxFontHeight));

        if (item.image != nullptr)
        {
            item.image->drawWithin(g, iconArea, juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize, 1.0f);
            r.removeFromLeft(std::round(maxFontHeight * 0.5f));
        }
        else if (item.isTicked)
        {
            const auto tick = component.getLookAndFeel().getTickShape(1.0f);
            g.fillPath(tick, tick.getTransformToScaleToFit(iconArea.reduced(iconArea.getWidth() / 5, 0).toFloat(), true));
        }

        if (item.subMenu != nullptr)
        {
            const auto arrowH = 0.6f * fontAscent;
            const auto x = r.removeFromRight(std::floor(arrowH)).getX();
            const auto halfH = r.getCentreY();

            juce::Path path;
            path.startNewSubPath(x, halfH - arrowH * 0.5f);
            path.lineTo(x + arrowH * 0.6f, halfH);
            path.lineTo(x, halfH + arrowH * 0.5f);

            g.strokePath(path, juce::PathStrokeType{ 2.0f });
        }

        r.removeFromRight(3.0f);
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
                                                 const juce::PopupMenu&,
                                                 const juce::PopupMenu::Options& options,
                                                 const juce::Component& component,
                                                 const juce::Rectangle<float>& bounds,
                                                 const InteractionState& interactionState,
                                                 const Styles& styles)
    {
        drawRoundedRectangle(g,
                             bounds,
                             getBorderRadius(item, styles),
                             getBorderWidth(item, styles),
                             getBackgroundFill(item, options, styles, interactionState, bounds),
                             getBorderFill(item, options, styles, bounds));

        if (item.isSeparator)
        {
            paintPopupMenuSeparator(g, item, options, styles, bounds);
        }
        else
        {
            paintPopupMenuTextItem(g, item, options, component, styles, interactionState, bounds);
        }
    }

    [[nodiscard]] static auto paintComboBox(juce::Graphics& g,
                                            const juce::ComboBox& comboBox,
                                            const Styles& styles)
    {
        paintComponent(g, comboBox, styles);

        const auto arrowBounds = comboBox
                                     .getLocalBounds()
                                     .removeFromRight(juce::roundToInt(comboBox.getHeight() * 1.2))
                                     .toFloat()
                                     .withSizeKeepingCentre(11.0f, 11.0f);
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
