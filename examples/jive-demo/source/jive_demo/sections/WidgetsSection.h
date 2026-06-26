#pragma once

#include "Section.h"

namespace jive_demo
{
    class WidgetsSection : public Section
    {
    public:
        [[nodiscard]] juce::String fileName() const override
        {
            return "sections/widgets.xml";
        }

        void attach(juce::ValueTree root) override
        {
            clearBindings();

            auto progress = jive::findElementWithID(root, "demo-progress");
            auto readout = jive::findElementWithID(root, "slider-readout");
            auto target = jive::findElementWithID(root, "toggle-target");

            bindSlider(root, "demo-slider", [progress, readout](double value) mutable {
                const auto normalised = juce::jlimit(0.0, 1.0, value / 100.0);
                progress.setProperty("value", normalised, nullptr);
                readout.setProperty("text", juce::String{ juce::roundToInt(value) } + "%", nullptr);
            });
            bindToggle(root, "enable-toggle", [target](bool on) mutable {
                target.setProperty("enabled", on, nullptr);
            });
        }
    };
} // namespace jive_demo
