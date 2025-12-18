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

    [[nodiscard]] static auto toString(const juce::RelativeTime& duration)
    {
        if (duration.inSeconds() >= 60.0)
            return juce::String{ duration.inSeconds() / 60.0 } + "m";
        if (duration.inSeconds() >= 1.0)
            return juce::String{ duration.inSeconds() } + "s";
        if (duration.inSeconds() >= 0.001)
            return juce::String{ duration.inSeconds() * 1000.0 } + "ms";

        return juce::String{ duration.inSeconds() * 1000000.0 } + u8"μs";
    }

    void printResult(std::vector<juce::RelativeTime>& durations)
    {
        std::sort(std::begin(durations), std::end(durations));
        const auto sum = std::accumulate(std::begin(durations),
                                         std::end(durations),
                                         0.0,
                                         [](double total, const juce::RelativeTime& next) {
                                             return total + next.inSeconds();
                                         });
        const juce::RelativeTime mean{ sum / static_cast<double>(std::size(durations)) };

        std::cout
            << "\n\n"
            << "Completed: " << std::size(durations) << " iterations\n\n"
            << "Min:       " << toString(durations.front()) << "\n"
            << "Max:       " << toString(durations.back()) << "\n"
            << "Median:    " << toString(durations.at(std::size(durations) / 2)) << "\n"
            << "Mean:      " << toString(mean) << "\n\n";
    }

    void doTimeboxedRun()
    {
        std::cout << "Duration:   " << duration->getDescription() << "\n\n";

        jive::Interpreter interpreter;
        const auto start = juce::Time::getCurrentTime();
        std::vector<juce::RelativeTime> durations;

        while (juce::Time::getCurrentTime() - start < duration)
        {
            const auto iterationStart = juce::Time::getMillisecondCounterHiRes();
            doIteration(interpreter);
            const auto iterationDurationMS = juce::Time::getMillisecondCounterHiRes() - iterationStart;
            durations.emplace_back(juce::RelativeTime::seconds(iterationDurationMS / 1000.0));

            printProgress(juce::jmin(1.0, (juce::Time::getCurrentTime() - start).inSeconds() / duration->inSeconds()));
        }

        printProgress(1.0);
        printResult(durations);
    }

    void doIterativeRun()
    {
        std::cout << "Iterations: " << *iterations << "\n";

        jive::Interpreter interpreter;

        std::vector<juce::RelativeTime> durations;

        for (auto i = 0; i < *iterations;)
        {
            const auto iterationStart = juce::Time::getCurrentTime();
            doIteration(interpreter);
            const auto iterationDuration = juce::Time::getCurrentTime() - iterationStart;
            durations.emplace_back(iterationDuration);

            i++;
            printProgress(i / static_cast<double>(*iterations));
        }

        printResult(durations);
    }

    const juce::String description;
    const std::optional<juce::RelativeTime> duration;
    const std::optional<int> iterations;

    static constexpr int columnWidth = 50;
};
