#pragma once

#include <jive_style_sheets/jive_style_sheets.h>

namespace jive_examples::widgets
{
    // A plain juce::Component that knows nothing about JIVE's elements. Its x
    // and y values go from 0 to 1, with y = 0 at the bottom. It draws itself
    // from the style sheet: its box like any element's, and the crosshair and
    // puck from its track and thumb styles.
    class XYPad : public juce::Component
    {
    public:
        [[nodiscard]] juce::Point<float> getValue() const
        {
            return value;
        }

        void setValue(juce::Point<float> newValue, juce::NotificationType notification = juce::sendNotificationSync)
        {
            newValue = {
                juce::jlimit(0.0f, 1.0f, newValue.x),
                juce::jlimit(0.0f, 1.0f, newValue.y),
            };

            if (newValue == value)
                return;

            value = newValue;
            repaint();

            if (notification != juce::dontSendNotification && onValueChange != nullptr)
                onValueChange();
        }

        void paint(juce::Graphics& g) final
        {
            auto* laf = dynamic_cast<jive::LookAndFeel*>(&getLookAndFeel());

            if (laf == nullptr)
                return;

            laf->drawComponent(g, *this);

            const auto styles = laf->findMostApplicableStyles(*this);
            const auto bounds = getLocalBounds().toFloat();
            const auto puck = getPuckArea().getRelativePoint(value.x, 1.0f - value.y);

            if (const auto track = styles.find<jive::Fill>("track"))
            {
                g.setFillType(track->toJuceFillType(bounds));
                g.fillRect(bounds.withY(puck.y - 0.5f).withHeight(1.0f));
                g.fillRect(bounds.withX(puck.x - 0.5f).withWidth(1.0f));
            }

            const auto thumb = juce::Rectangle{ puckSize, puckSize }.withCentre(puck);
            g.setFillType(styles.find<jive::Fill>("thumb").value_or(juce::Colours::white).toJuceFillType(thumb));
            g.fillEllipse(thumb);
        }

        void mouseDown(const juce::MouseEvent& event) final
        {
            setValueFromMouse(event);
        }

        void mouseDrag(const juce::MouseEvent& event) final
        {
            setValueFromMouse(event);
        }

        std::function<void()> onValueChange = nullptr;

    private:
        static constexpr auto puckSize = 16.0f;

        [[nodiscard]] juce::Rectangle<float> getPuckArea() const
        {
            return getLocalBounds().toFloat().reduced(puckSize / 2.0f);
        }

        void setValueFromMouse(const juce::MouseEvent& event)
        {
            const auto area = getPuckArea();
            const auto position = event.position - area.getPosition();

            setValue({
                position.x / area.getWidth(),
                1.0f - position.y / area.getHeight(),
            });
        }

        juce::Point<float> value{ 0.5f, 0.5f };
    };
} // namespace jive_examples::widgets
