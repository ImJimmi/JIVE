#pragma once

#include "Section.h"

namespace jive_demo
{
    class StylesSection : public Section
    {
    public:
        [[nodiscard]] juce::String fileName() const override
        {
            return "sections/styles.xml";
        }

        void attach(juce::ValueTree root) override
        {
            clearBindings();

            auto preview = jive::findElementWithID(root, "style-preview");

            if (!preview.isValid())
                return;

            style = new jive::Object{
                { "background", "#316DCA" },
                { "foreground", "#0D1117" },
                { "border", "#A9E4EF" },
                { "border-width", 0 },
                { "border-radius", 12 },
                { "font-family", "Rubik Mono One" },
                { "font-size", 22 },
                { "transition", "background 250ms ease-out, border-radius 200ms ease-out, border-width 200ms ease-out" },
            };
            preview.setProperty("style", juce::var{ style.get() }, nullptr);

            auto radiusLabel = jive::findElementWithID(root, "style-radius-label");
            auto borderLabel = jive::findElementWithID(root, "style-border-label");
            auto fontLabel = jive::findElementWithID(root, "style-font-label");

            bindComboBox(root, "style-background", { "Blue", "Green", "Crimson", "Gradient" }, [this](const juce::String& value) {
                if (value == "Gradient")
                    style->setProperty("background", juce::var{ gradient() });
                else
                    style->setProperty("background", solid(value));
            });
            bindSlider(root, "style-radius", [this, radiusLabel](double value) mutable {
                const auto radius = juce::roundToInt(value);
                style->setProperty("border-radius", radius);
                radiusLabel.setProperty("text", "border-radius: " + juce::String{ radius } + "px", nullptr);
            });
            bindSlider(root, "style-border", [this, borderLabel](double value) mutable {
                const auto width = juce::roundToInt(value);
                style->setProperty("border-width", width);
                borderLabel.setProperty("text", "border-width: " + juce::String{ width } + "px", nullptr);
            });
            bindSlider(root, "style-font", [this, fontLabel](double value) mutable {
                const auto size = juce::roundToInt(value);
                style->setProperty("font-size", size);
                fontLabel.setProperty("text", "font-size: " + juce::String{ size } + "px", nullptr);
            });
            bindToggle(root, "style-shadow", [this](bool on) {
                style->setProperty("shadow", on ? juce::String{ "0 10 28 rgba(0,0,0,0.55)" }
                                                : juce::String{ "0 0 0 rgba(0,0,0,0)" });
            });
        }

    private:
        [[nodiscard]] static juce::String solid(const juce::String& name)
        {
            if (name == "Green")
                return "#347D39";
            if (name == "Crimson")
                return "#C93C37";

            return "#316DCA";
        }

        [[nodiscard]] static jive::Object* gradient()
        {
            return new jive::Object{
                { "gradient", "linear" },
                { "orientation", "horizontal" },
                {
                    "stops",
                    new jive::Object{
                        { "0.0", "#8256D0" },
                        { "1.0", "#AE4C82" },
                    },
                },
            };
        }

        jive::Object::ReferenceCountedPointer style;
    };
} // namespace jive_demo
