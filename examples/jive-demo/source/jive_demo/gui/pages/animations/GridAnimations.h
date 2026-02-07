#pragma once

#include <jive_demo/gui/views/DemoCallToAction.h>

#include <jive_layouts/jive_layouts.h>

namespace jive_demo
{
    [[nodiscard]] static auto calculateRandomGridGap()
    {
        static auto& rng = juce::Random::getSystemRandom();
        return juce::String{ rng.nextInt({ 5, 60 }) } + " " + juce::String{ rng.nextInt({ 5, 60 }) };
    }

    [[nodiscard]] static auto calculateRandomFr(int size)
    {
        static auto& rng = juce::Random::getSystemRandom();

        juce::Array<juce::Grid::TrackInfo> result;

        for (auto i = 0; i < size; i++)
            result.add(juce::Grid::Fr{ rng.nextInt({ 1, 5 }) });

        return result;
    }

    namespace views
    {
        [[nodiscard]] inline auto animatedGridItem()
        {
            return juce::ValueTree{
                "Component",
                {
                    { "border-width", 2 },
                    {
                        "style",
                        new jive::Object{
                            { "background", jive::toVar(colours::blue600) },
                            { "border", jive::toVar(colours::blue400) },
                            { "border-radius", 10 },
                        },
                    },
                },
            };
        }

        [[nodiscard]] inline auto gridAnimationsDemo()
        {
            return juce::ValueTree{
                "Component",
                {
                    { "align-items", "stretch" },
                    { "flex-grow", 1 },
                },
                {
                    juce::ValueTree{
                        "Component",
                        {
                            { "flex-direction", "row" },
                            { "margin", "0 0 20 0" },
                        },
                        {
                            juce::ValueTree{
                                "Text",
                                {
                                    { "text", "Animate:" },
                                    { "margin", "0 15 0 0" },
                                },
                            },
                            DemoCallToAction{}
                                .withText("Gap")()
                                .setProperty("id", "animate-gap-button", nullptr)
                                .setProperty("margin", "0 10 0 0", nullptr),
                            DemoCallToAction{}
                                .withText("Rows")()
                                .setProperty("id", "animate-rows-button", nullptr)
                                .setProperty("margin", "0 10 0 0", nullptr),
                            DemoCallToAction{}
                                .withText("Columns")()
                                .setProperty("id", "animate-columns-button", nullptr)
                                .setProperty("margin", "0 10 0 0", nullptr),
                        },
                    },
                    juce::ValueTree{
                        "Component",
                        {
                            { "id", "grid-container" },
                            { "flex-grow", 1 },
                            { "display", "grid" },
                            { "grid-template-columns", jive::toVar(calculateRandomFr(3)) },
                            { "grid-template-rows", jive::toVar(calculateRandomFr(3)) },
                            { "gap", calculateRandomGridGap() },
                            {
                                "transition",
                                "gap 250ms ease-in-out,"
                                "grid-template-columns 350ms ease-in-out,"
                                "grid-template-rows 350ms ease-in-out",
                            },
                        },
                        {
                            animatedGridItem(),
                            animatedGridItem(),
                            animatedGridItem(),
                            animatedGridItem(),
                            animatedGridItem(),
                            animatedGridItem(),
                            animatedGridItem(),
                            animatedGridItem(),
                            animatedGridItem(),
                        },
                    },
                },
            };
        }
    } // namespace views

    class GridAnimationsPresenter
    {
    public:
        GridAnimationsPresenter()
            : onAnimateGapButtonClicked{
                jive::findElementWithID(view, "animate-gap-button"),
                "on-click",
            }
            , onAnimateRowsButtonClicked{
                jive::findElementWithID(view, "animate-rows-button"),
                "on-click",
            }
            , onAnimateColumnsButtonClicked{
                jive::findElementWithID(view, "animate-columns-button"),
                "on-click",
            }
        {
            onAnimateGapButtonClicked.onTrigger = [this] {
                jive::findElementWithID(view, "grid-container")
                    .setProperty("gap", calculateRandomGridGap(), nullptr);
            };
            onAnimateRowsButtonClicked.onTrigger = [this] {
                jive::findElementWithID(view, "grid-container")
                    .setProperty("grid-template-rows", jive::toVar(calculateRandomFr(3)), nullptr);
            };
            onAnimateColumnsButtonClicked.onTrigger = [this] {
                jive::findElementWithID(view, "grid-container")
                    .setProperty("grid-template-columns", jive::toVar(calculateRandomFr(3)), nullptr);
            };
        }

        [[nodiscard]] auto present() const
        {
            return view;
        }

    private:
        juce::ValueTree view{ views::gridAnimationsDemo() };

        jive::Event onAnimateGapButtonClicked;
        jive::Event onAnimateRowsButtonClicked;
        jive::Event onAnimateColumnsButtonClicked;
    };
} // namespace jive_demo
