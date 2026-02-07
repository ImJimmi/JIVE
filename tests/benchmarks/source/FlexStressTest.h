#pragma once

#include "Benchmark.h"

class FlexStressTest : public Benchmark
{
public:
    FlexStressTest()
        : Benchmark{
            "jive::FlexContainer Stress Test",
            juce::RelativeTime::seconds(30.0),
        }
    {
    }

protected:
    void doIteration(jive::Interpreter& interpreter) final
    {
        static constexpr auto button = [] {
            static auto id = -1;
            id++;
            return juce::ValueTree{
                "Button",
                {
                    { "id", id },
                },
                {
                    juce::ValueTree{
                        "Component",
                        {
                            { "id", id },
                            { "width", 20 },
                            { "height", 50 },
                        },
                    },
                    juce::ValueTree{
                        "Text",
                        {
                            { "id", id },
                            { "text", "Quisque hendrerit pharetra libero, nec eleifend felis blandit sed. Etiam in ligula efficitur, volutpat turpis luctus, dictum mauris. Proin ipsum nunc, lobortis in suscipit et, elementum eget felis. Morbi nec nisi lorem. Nulla laoreet elit tortor, a aliquam ante rhoncus in. Sed blandit arcu nibh, at congue arcu ornare id. Nunc vulputate tempor egestas. Cras rhoncus enim vel nisl convallis iaculis. Praesent felis nibh, lobortis id enim et, ultricies tempus urna. Curabitur sed felis et metus convallis varius. Duis eget lacus lectus. Proin et dolor scelerisque, hendrerit leo eget, posuere magna. Aenean dolor augue, efficitur in mi ac, lobortis tristique eros. Vivamus auctor purus et nisi consectetur, a porttitor elit scelerisque." },
                        },
                    },
                },
            };
        };
        juce::ValueTree view{
            "Component",
            {
                { "id", "top-level" },
                { "width", 540 },
                { "height", 360 },
                { "flex-wrap", "wrap" },
            },
            {
                button(),
                button(),
                button(),
                button(),
                button(),
                button(),
                button(),
                button(),
                button(),
            },
        };
        const auto item = interpreter.interpret(view);

        view.setProperty("align-content", "flex-start", nullptr);
        view.setProperty("align-content", "flex-end", nullptr);
        view.setProperty("align-content", "centre", nullptr);
        view.setProperty("align-content", "space-between", nullptr);
        view.setProperty("align-content", "space-around", nullptr);
        view.setProperty("align-content", "stretch", nullptr);
        view.setProperty("align-items", "flex-start", nullptr);
        view.setProperty("align-items", "flex-end", nullptr);
        view.setProperty("align-items", "centre", nullptr);
        view.setProperty("align-items", "stretch", nullptr);
        view.setProperty("flex-direction", "row-reverse", nullptr);
        view.setProperty("flex-direction", "column", nullptr);
        view.setProperty("flex-direction", "column-reverse", nullptr);
        view.setProperty("flex-direction", "row", nullptr);
        view.setProperty("justify-content", "flex-end", nullptr);
        view.setProperty("justify-content", "centre", nullptr);
        view.setProperty("justify-content", "space-between", nullptr);
        view.setProperty("justify-content", "space-around", nullptr);
        view.setProperty("justify-content", "flex-start", nullptr);
        view.setProperty("flex-wrap", "nowrap", nullptr);
        view.setProperty("flex-wrap", "wrap-reverse", nullptr);
        view.setProperty("flex-wrap", "wrap", nullptr);

        for (auto child : view)
        {
            child.setProperty("align-self", "flex-start", nullptr);
            child.setProperty("align-self", "flex-end", nullptr);
            child.setProperty("align-self", "centre", nullptr);
            child.setProperty("align-self", "stretch", nullptr);
            child.setProperty("align-self", "auto", nullptr);
        }
    }

private:
};
