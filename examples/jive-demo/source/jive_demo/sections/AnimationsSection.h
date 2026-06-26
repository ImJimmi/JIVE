#pragma once

#include "Section.h"

namespace jive_demo
{
    class AnimationsSection : public Section
    {
    public:
        [[nodiscard]] juce::String fileName() const override
        {
            return "sections/animations.xml";
        }

        void attach(juce::ValueTree root) override
        {
            clearBindings();

            box = jive::findElementWithID(root, "anim-box");
            grown = false;

            auto durationLabel = jive::findElementWithID(root, "ease-duration-label");

            bindComboBox(root, "ease-function", { "linear", "ease-in", "ease-out", "ease-in-out", "cubic-bezier(.68,-.55,.27,1.55)" }, [this](const juce::String& value) {
                easing = value;
            });
            bindSlider(root, "ease-duration", [this, durationLabel](double value) mutable {
                durationMs = juce::roundToInt(value);
                durationLabel.setProperty("text", "duration: " + juce::String{ durationMs } + "ms", nullptr);
            });
            bindClick(root, "ease-play", [this] {
                if (!box.isValid())
                    return;

                box.setProperty("transition",
                                "flex-grow " + juce::String{ durationMs } + "ms " + easing,
                                nullptr);

                grown = !grown;
                box.setProperty("flex-grow", grown ? 9.0 : 0.1, nullptr);
            });
        }

    private:
        juce::ValueTree box;
        juce::String easing{ "ease-in-out" };
        int durationMs{ 800 };
        bool grown{ false };
    };
} // namespace jive_demo
