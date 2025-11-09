#pragma once

namespace jive::default_styles
{
    inline void paintGroupComponent(juce::Graphics& g,
                                    const juce::GroupComponent& group,
                                    const Styles& styles)
    {
        const auto groupBounds = getGroupBounds(group, styles);
        const auto borderRadius = getBorderRadius(group, styles);
        const auto borderWidth = getBorderWidth(group, styles);

        drawRoundedRectangle(g,
                             groupBounds,
                             borderRadius,
                             borderWidth,
                             getBackgroundFill(group, styles),
                             juce::Colours::transparentBlack);

        if (group.getText().isNotEmpty())
        {
            const auto font = getFont(group, styles);
            const auto titleBounds = getTitleBounds(group, styles);

            g.setFont(font);
            g.setFillType(getForegroundFill(group, styles));
            g.drawFittedText(group.getText(),
                             titleBounds.toNearestInt(),
                             juce::Justification::centred,
                             juce::jmax(1, static_cast<int>(titleBounds.getHeight() / font.getHeight())),
                             2.0f / 3.0f);

            g.excludeClipRegion(titleBounds.expanded(5.0f).toNearestInt());
        }

        drawRoundedRectangle(g,
                             groupBounds,
                             borderRadius,
                             borderWidth,
                             juce::Colours::transparentBlack,
                             getBorderFill(group, styles));
    }
} // namespace jive::default_styles
