#pragma once

#include <jive_layouts/jive_layouts.h>

namespace boabass
{
    // Draws an ADSR envelope's shape. Times are drawn on a square-root scale
    // so short attacks stay visible next to long releases. Its box is drawn
    // like any other element's, and the curve from its styles:
    //  - fill: the area under the curve
    //  - stroke: the curve itself
    //  - thumb: the dots at each stage
    class EnvelopeView : public juce::Component
    {
    public:
        struct Envelope
        {
            float attack = 0.0f;
            float decay = 0.0f;
            float sustain = 1.0f;
            float release = 0.0f;
        };

        void setEnvelope(const Envelope& newEnvelope)
        {
            envelope = newEnvelope;
            repaint();
        }

        void paint(juce::Graphics& g) final
        {
            auto* lookAndFeel = dynamic_cast<jive::LookAndFeel*>(&getLookAndFeel());

            if (lookAndFeel == nullptr)
                return;

            lookAndFeel->drawComponent(g, *this);

            const auto styles = lookAndFeel->findMostApplicableStyles(*this);
            const auto area = getLocalBounds().toFloat().reduced(8.0f, 10.0f);
            const auto points = calculatePoints(area);

            juce::Path curve;
            curve.startNewSubPath(points.front());

            for (auto point = std::next(points.begin()); point != points.end(); point++)
                curve.lineTo(*point);

            auto filled = curve;
            filled.lineTo(area.getBottomLeft());
            filled.closeSubPath();

            g.setFillType(jive::getFill(*this, styles, juce::Colours::transparentBlack));
            g.fillPath(filled);

            g.setFillType(jive::getStroke(*this, styles));
            g.strokePath(curve, juce::PathStrokeType{ 2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded });

            const auto thumb = styles.find<jive::Fill>("thumb").value_or(juce::Colours::white);

            for (auto point = std::next(points.begin()); point != std::prev(points.end()); point++)
            {
                const auto dot = juce::Rectangle{ 6.0f, 6.0f }.withCentre(*point);
                g.setFillType(thumb.toJuceFillType(dot));
                g.fillEllipse(dot);
            }
        }

    private:
        [[nodiscard]] std::array<juce::Point<float>, 5> calculatePoints(juce::Rectangle<float> area) const
        {
            static constexpr auto hold = 0.6f;

            const auto attack = std::sqrt(envelope.attack);
            const auto decay = std::sqrt(envelope.decay);
            const auto release = std::sqrt(envelope.release);
            const auto total = juce::jmax(0.001f, attack + decay + hold + release);
            const auto x = [&area, total](float time) {
                return area.getX() + area.getWidth() * time / total;
            };
            const auto sustainY = area.getBottom() - area.getHeight() * envelope.sustain;

            return {
                area.getBottomLeft(),
                juce::Point{ x(attack), area.getY() },
                juce::Point{ x(attack + decay), sustainY },
                juce::Point{ x(attack + decay + hold), sustainY },
                area.getBottomRight(),
            };
        }

        Envelope envelope;
    };

    class EnvelopeViewItem : public jive::GuiItemDecorator
    {
    public:
        explicit EnvelopeViewItem(std::unique_ptr<jive::GuiItem> itemToDecorate)
            : jive::GuiItemDecorator{ std::move(itemToDecorate) }
        {
            for (auto* property : { &attack, &decay, &sustain, &release })
            {
                property->onValueChange = [this] {
                    update();
                };
            }

            update();
        }

    private:
        void update()
        {
            dynamic_cast<EnvelopeView&>(*getComponent()).setEnvelope({
                attack.get(),
                decay.get(),
                sustain.get(),
                release.get(),
            });
        }

        jive::Property<float> attack{ state, "attack" };
        jive::Property<float> decay{ state, "decay" };
        jive::Property<float> sustain{ state, "sustain" };
        jive::Property<float> release{ state, "release" };
    };
} // namespace boabass
