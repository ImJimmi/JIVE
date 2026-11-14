#pragma once

#include <jive_style_sheets/jive_style_sheets.h>

namespace boabass
{
    // Draws a combo box as a bevelled pill with its arrow in a recessed
    // socket on the right. Colours come from its styles:
    //  - background: the body
    //  - border: the outline and the socket's divider
    //  - track: the socket
    //  - accent: the arrow
    inline void paintComboBox(juce::Component& component, const jive::Styles& styles, juce::Graphics& g)
    {
        auto& comboBox = dynamic_cast<juce::ComboBox&>(component);
        auto& lookAndFeel = dynamic_cast<jive::LookAndFeel&>(comboBox.getLookAndFeel());

        const auto bounds = comboBox.getLocalBounds().toFloat().reduced(0.5f);
        const auto radius = bounds.getHeight() * 0.5f;
        const auto icon = lookAndFeel.getComboBoxIconBounds(comboBox, styles);
        const auto socket = bounds.withLeft(icon.getX() - (bounds.getRight() - icon.getRight()) * 0.5f).reduced(3.0f);

        juce::Path body;
        body.addRoundedRectangle(bounds, radius);

        g.setFillType(jive::getBackgroundFill(comboBox, styles));
        g.fillPath(body);

        g.setFillType(styles.find<jive::Fill>("track").value_or(juce::Colours::black).toJuceFillType(socket));
        g.fillRoundedRectangle(socket, socket.getHeight() * 0.5f);

        g.setColour(juce::Colours::white.withAlpha(0.06f));
        g.drawLine(bounds.getX() + radius, bounds.getY() + 1.5f, socket.getX() - 4.0f, bounds.getY() + 1.5f, 1.0f);

        g.setFillType(jive::getBorderFill(comboBox, styles));
        g.strokePath(body, juce::PathStrokeType{ 1.0f });

        const auto arrowArea = juce::Rectangle{ 9.0f, 5.0f }.withCentre(socket.getCentre());
        juce::Path arrow;
        arrow.startNewSubPath(arrowArea.getTopLeft());
        arrow.lineTo(arrowArea.getCentreX(), arrowArea.getBottom());
        arrow.lineTo(arrowArea.getTopRight());

        g.setFillType(styles.find<jive::Fill>("accent").value_or(juce::Colours::white).toJuceFillType(arrowArea));
        g.strokePath(arrow, juce::PathStrokeType{ 1.8f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded });

        comboBox.getProperties().set(jive::ids::shadowPath(), body.toString());
    }
} // namespace boabass
