#pragma once

#include "Benchmark.h"

class MinimumViewBenchmark : public Benchmark
{
public:
    MinimumViewBenchmark()
        : Benchmark{
            "Minimum Possible View",
            juce::RelativeTime::seconds(5.0),
        }
    {
    }

protected:
    void doIteration(jive::Interpreter& interpreter) final
    {
        const auto item = interpreter.interpret(juce::ValueTree{
            "Component",
            {
                { "width", 1 },
                { "height", 1 },
            },
        });
    }
};
