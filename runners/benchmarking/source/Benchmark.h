#pragma once

#include <jive_layouts/jive_layouts.h>

class Benchmark
{
public:
    explicit Benchmark(juce::String testDescription,
                       juce::RelativeTime testDuration)
        : description{ testDescription }
        , duration{ testDuration }
    {
    }

    explicit Benchmark(juce::String testDescription, int numIterations)
        : description{ testDescription }
        , iterations{ numIterations }
    {
    }

    void run()
    {
        std::cout << "Test:       " << description << "\n";

        if (duration.has_value())
            doTimeboxedRun();
        if (iterations.has_value())
            doIterativeRun();

        std::cout << juce::String::repeatedString(juce::CharPointer_UTF8{ "\xe2\x95\x90" }, columnWidth) << "\n\n";
    }

protected:
    virtual void doIteration(jive::Interpreter& interpreter) = 0;

private:
    void printProgress(double progressNormalised)
    {
        std::cout << "\r" << jive::buildProgressBar(progressNormalised, columnWidth) << std::flush;
    }

    void printResult(int counter, juce::RelativeTime elapsed)
    {
        std::cout << "\n\n"
                  << "Completed:  " << counter << " iterations\n"
                  << "Average:    " << static_cast<double>(elapsed.inMilliseconds()) / static_cast<double>(counter) << "ms\n\n";
    }

    void doTimeboxedRun()
    {
        std::cout << "Duration:   " << duration->getDescription() << "\n\n";

        jive::Interpreter interpreter;
        auto counter = 0;
        const auto start = juce::Time::getCurrentTime();

        for (auto elapsed = juce::Time::getCurrentTime() - start;
             elapsed <= duration.value();
             elapsed = juce::Time::getCurrentTime() - start)
        {
            doIteration(interpreter);
            counter++;
            printProgress(elapsed.inSeconds() / duration->inSeconds());
        }

        printProgress(1.0);
        printResult(counter, *duration);
    }

    void doIterativeRun()
    {
        std::cout << "Iterations: " << *iterations << "\n";

        jive::Interpreter interpreter;
        const auto start = juce::Time::getCurrentTime();

        for (auto i = 0; i < *iterations;)
        {
            doIteration(interpreter);
            i++;
            printProgress(i / static_cast<double>(*iterations));
        }

        const auto elapsed = juce::Time::getCurrentTime() - start;
        printResult(*iterations, elapsed);
    }

    const juce::String description;
    const std::optional<juce::RelativeTime> duration;
    const std::optional<int> iterations;

    static constexpr int columnWidth = 50;
};
