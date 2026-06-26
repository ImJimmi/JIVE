#pragma once

#include <jive_layouts/jive_layouts.h>

namespace jive_demo
{
    // A demo section: an XML file under views/sections/ plus the interactive
    // wiring that brings it to life. The DemoController loads the file into the
    // content area and then calls attach() with the resulting (live) value-tree
    // so the section can bind its controls.
    class Section
    {
    public:
        virtual ~Section() = default;

        // Path (relative to the views source directory) of the section's markup.
        [[nodiscard]] virtual juce::String fileName() const = 0;

        // Bind interactive controls within the freshly-loaded section tree.
        virtual void attach(juce::ValueTree root) = 0;

    protected:
        void clearBindings()
        {
            events.clear();
            comboBindings.clear();
            sliderBindings.clear();
            toggleBindings.clear();
        }

        // Calls apply() with the currently-selected option string whenever the
        // ComboBox with the given id changes (and once immediately to sync).
        void bindComboBox(juce::ValueTree root,
                          const juce::Identifier& comboID,
                          juce::StringArray options,
                          std::function<void(const juce::String&)> apply)
        {
            auto combo = jive::findElementWithID(root, comboID);

            if (!combo.isValid())
                return;

            auto property = std::make_unique<jive::Property<int>>(combo, "selected");
            auto* raw = property.get();

            const auto applyCurrent = [raw, options, apply] {
                const auto index = raw->get();

                if (index >= 0 && index < options.size())
                    apply(options[index]);
            };

            property->onValueChange = applyCurrent;
            applyCurrent();

            comboBindings.push_back(std::move(property));
        }

        // Calls apply() with the Slider's value whenever it changes.
        void bindSlider(juce::ValueTree root,
                        const juce::Identifier& sliderID,
                        std::function<void(double)> apply)
        {
            auto slider = jive::findElementWithID(root, sliderID);

            if (!slider.isValid())
                return;

            auto property = std::make_unique<jive::Property<double>>(slider, "value");
            auto* raw = property.get();

            property->onValueChange = [raw, apply] {
                apply(raw->get());
            };
            apply(property->get());

            sliderBindings.push_back(std::move(property));
        }

        // Calls apply() with the toggle state whenever the Checkbox/Button with
        // the given id changes.
        void bindToggle(juce::ValueTree root,
                        const juce::Identifier& toggleID,
                        std::function<void(bool)> apply)
        {
            auto toggle = jive::findElementWithID(root, toggleID);

            if (!toggle.isValid())
                return;

            auto property = std::make_unique<jive::Property<bool>>(toggle, "toggled");
            auto* raw = property.get();

            property->onValueChange = [raw, apply] {
                apply(raw->get());
            };
            apply(property->get());

            toggleBindings.push_back(std::move(property));
        }

        void bindClick(juce::ValueTree root,
                       const juce::Identifier& buttonID,
                       std::function<void()> onClick)
        {
            auto button = jive::findElementWithID(root, buttonID);

            if (!button.isValid())
                return;

            events.push_back(std::make_unique<jive::Event>(button,
                                                           "on-click",
                                                           std::move(onClick)));
        }

    private:
        std::vector<std::unique_ptr<jive::Event>> events;
        std::vector<std::unique_ptr<jive::Property<int>>> comboBindings;
        std::vector<std::unique_ptr<jive::Property<double>>> sliderBindings;
        std::vector<std::unique_ptr<jive::Property<bool>>> toggleBindings;
    };
} // namespace jive_demo
