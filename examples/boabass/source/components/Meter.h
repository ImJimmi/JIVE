#pragma once

#include <jive_layouts/jive_layouts.h>

namespace boabass
{
    // A column of LEDs, lit from the bottom up to its value. Lit LEDs use the
    // track fill, laid out over the whole column so a gradient reads like a
    // meter's scale. Unlit ones use the background.
    class Meter : public juce::Component
    {
    public:
        void setValue(float newValue)
        {
            value = juce::jlimit(0.0f, 1.0f, newValue);
            repaint();
        }

        void paint(juce::Graphics& g) final
        {
            static constexpr auto ledPitch = 6.0f;
            static constexpr auto gap = 2.0f;

            auto* lookAndFeel = dynamic_cast<jive::LookAndFeel*>(&getLookAndFeel());

            if (lookAndFeel == nullptr)
                return;

            const auto styles = lookAndFeel->findMostApplicableStyles(*this);
            const auto bounds = getLocalBounds().toFloat();
            const auto numLeds = juce::jmax(1, static_cast<int>(bounds.getHeight() / ledPitch));
            const auto ledHeight = bounds.getHeight() / static_cast<float>(numLeds);
            const auto litLeds = juce::roundToInt(value * static_cast<float>(numLeds));
            const auto lit = styles.find<jive::Fill>("track").value_or(juce::Colours::limegreen).toJuceFillType(bounds);
            const auto unlit = styles.find<jive::Fill>("background").value_or(juce::Colours::darkgrey).toJuceFillType(bounds);

            for (auto i = 0; i < numLeds; i++)
            {
                const auto led = bounds
                                     .withY(bounds.getBottom() - ledHeight * static_cast<float>(i + 1))
                                     .withHeight(ledHeight)
                                     .reduced(0.0f, gap / 2.0f);

                g.setFillType(i < litLeds ? lit : unlit);
                g.fillRoundedRectangle(led, 1.0f);
            }
        }

    private:
        float value = 0.0f;
    };

    class MeterItem : public jive::GuiItemDecorator
    {
    public:
        explicit MeterItem(std::unique_ptr<jive::GuiItem> itemToDecorate)
            : jive::GuiItemDecorator{ std::move(itemToDecorate) }
        {
            value.onValueChange = [this] {
                getMeter().setValue(value.get());
            };
            getMeter().setValue(value.get());
        }

    private:
        [[nodiscard]] Meter& getMeter()
        {
            return dynamic_cast<Meter&>(*getComponent());
        }

        jive::Property<float> value{ state, "value" };
    };
} // namespace boabass
