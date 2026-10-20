#pragma once

namespace jive::default_styles
{
    template <typename Component>
    inline void paintText(juce::Graphics& g,
                          const Component& component,
                          juce::Rectangle<float> bounds,
                          const Styles& styles,
                          const juce::String& text,
                          juce::Justification justification,
                          float minimumHorizontalScale = 1.0f)
    {
        const auto font = getFont(component, styles);
        const auto textArea = getBorderWidth(component, styles)
                                  .subtractedFrom(bounds)
                                  .toNearestInt();

        g.setFont(font);
        g.setFillType(getForegroundFill(component, styles));
        g.drawFittedText(text,
                         textArea,
                         justification,
                         std::max(1, (int)((float)textArea.getHeight() / font.getHeight())),
                         minimumHorizontalScale);
    }

    inline void paintLabel(juce::Graphics& g,
                           const juce::Label& label,
                           const Styles& styles)
    {
        paintComponent(g, label, styles);

        if (label.isBeingEdited())
            return;

        const auto alpha = label.isEnabled() ? 1.0f : 0.5f;
        const juce::Graphics::ScopedSaveState sss{ g };
        g.setOpacity(alpha);
        paintText(g,
                  label,
                  label.getLocalBounds().toFloat(),
                  styles,
                  label.getText(),
                  label.getJustificationType(),
                  label.getMinimumHorizontalScale());
    }
} // namespace jive::default_styles
