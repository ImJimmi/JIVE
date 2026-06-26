#pragma once

#include "Section.h"

namespace jive_demo
{
    class LayoutsSection : public Section
    {
    public:
        [[nodiscard]] juce::String fileName() const override
        {
            return "sections/layouts.xml";
        }

        void attach(juce::ValueTree root) override
        {
            clearBindings();

            auto flex = jive::findElementWithID(root, "flex-preview");
            auto grid = jive::findElementWithID(root, "grid-preview");
            auto gapLabel = jive::findElementWithID(root, "grid-gap-label");

            bindComboBox(root, "flex-direction", { "row", "column", "row-reverse", "column-reverse" }, [flex](const juce::String& value) mutable {
                flex.setProperty("flex-direction", value, nullptr);
            });
            bindComboBox(root, "flex-justify", { "flex-start", "centre", "flex-end", "space-between", "space-around" }, [flex](const juce::String& value) mutable {
                flex.setProperty("justify-content", value, nullptr);
            });
            bindComboBox(root, "flex-align", { "flex-start", "centre", "flex-end", "stretch" }, [flex](const juce::String& value) mutable {
                flex.setProperty("align-items", value, nullptr);
            });
            bindComboBox(root, "flex-wrap", { "nowrap", "wrap" }, [flex](const juce::String& value) mutable {
                flex.setProperty("flex-wrap", value, nullptr);
            });

            bindComboBox(root, "grid-columns", { "1fr 1fr 1fr", "1fr 1fr", "2fr 1fr", "1fr 2fr 1fr" }, [grid](const juce::String& value) mutable {
                grid.setProperty("grid-template-columns", value, nullptr);
            });
            bindSlider(root, "grid-gap", [grid, gapLabel](double value) mutable {
                const auto gap = juce::roundToInt(value);
                grid.setProperty("gap", gap, nullptr);
                gapLabel.setProperty("text", "gap: " + juce::String{ gap } + "px", nullptr);
            });
        }
    };
} // namespace jive_demo
