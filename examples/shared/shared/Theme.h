#pragma once

#include <jive_layouts/jive_layouts.h>

namespace jive_examples
{
    // Switches the root's theme to whichever one is picked in its
    // theme-combo-box, whose options are named after the themes.
    [[nodiscard]] inline std::unique_ptr<jive::Property<int>> bindThemeComboBox(juce::ValueTree root)
    {
        static const juce::Identifier themeComboBoxID{ "theme-combo-box" };
        auto comboBox = jive::findElementWithID(root, themeComboBoxID);
        comboBox.setProperty(jive::ids::selected(), 0, nullptr);

        auto selected = std::make_unique<jive::Property<int>>(comboBox, jive::ids::selected());
        selected->onValueChange = [root, comboBox, &index = *selected]() mutable {
            if (index.get() < 0)
            {
                jassertfalse;
                return;
            }

            root.setProperty(jive::ids::theme(),
                             comboBox.getChild(index.get())[jive::ids::text()].toString().toLowerCase(),
                             nullptr);
        };

        return selected;
    }
} // namespace jive_examples
