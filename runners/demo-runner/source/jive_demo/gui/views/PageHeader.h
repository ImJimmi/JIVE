#pragma once

#include "PageLink.h"
#include "Title.h"

#include <jive_demo/gui/tokens/Colours.h>

namespace jive_demo::views
{
    class PageHeader
    {
    public:
        [[nodiscard]] auto& withTitle(const juce::String& pageTitle)
        {
            title = pageTitle;
            return *this;
        }

        [[nodiscard]] auto& withIcon(const juce::ValueTree& pageIcon)
        {
            icon = pageIcon;
            return *this;
        }

        [[nodiscard]] auto operator()() const
        {
            static constexpr auto style = [] {
                return new jive::Object{
                    {
                        "background",
                        new jive::Object{
                            {
                                "stops",
                                new jive::Object{
                                    { "0.0", jive::toVar(colours::blue500) },
                                    { "0.5", jive::toVar(colours::blue600) },
                                    { "1.0", jive::toVar(colours::blue700) },
                                },
                            },
                            { "gradient", "radial" },
                            { "start", "0.0, 0.0" },
                            { "end", "1.0, 1.0" },
                        },
                    },
                };
            };

            auto iconCopy = icon.createCopy();
            static constexpr auto linkHeight = 25;

            return juce::ValueTree{
                "Component",
                {
                    { "flex-direction", "row" },
                    { "padding", 20 },
                    { "style", style() },
                    { "align-items", "centre" },
                },
                {
                    PageLink{}
                        .withIcon(icons::small::back())
                        .withText("Back")
                        .withColour(colours::content)()
                        .setProperty("id", "back", nullptr)
                        .setProperty("height", linkHeight, nullptr)
                        .setProperty("margin", 0, nullptr),
                    juce::ValueTree{
                        "Component",
                        {
                            { "flex-direction", "row" },
                            { "align-items", "centre" },
                            { "justify-content", "centre" },
                            { "flex-grow", 1 },
                        },
                        {
                            iconCopy
                                .setProperty("margin", "0 15 0 0", nullptr),
                            Title{}
                                .withText(title)()
                                .setProperty("margin", "2 0 0 0", nullptr),
                        },
                    },
                    PageLink{}
                        .withText("Next")
                        .withIcon(icons::small::next())
                        .withColour(colours::content)
                        .withIconOnRight()()
                        .setProperty("id", "next", nullptr)
                        .setProperty("height", linkHeight, nullptr)
                        .setProperty("margin", 0, nullptr),
                },
            };
        }

    private:
        juce::String title;
        juce::ValueTree icon;
    };
} // namespace jive_demo::views
