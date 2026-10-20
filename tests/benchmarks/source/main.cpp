#include "CardsBenchmark.h"
#include "FlexStressTest.h"
#include "MinimumViewBenchmark.h"
#include "TextResizeBenchmark.h"

class BenchmarkApp : public juce::JUCEApplication
{
public:
    BenchmarkApp() = default;

    const juce::String getApplicationName() final
    {
        return "JIVE Benchmarking";
    }

    const juce::String getApplicationVersion() final
    {
        return "1.0.0";
    }

    void initialise(const juce::String& commandLine) final
    {
#if PERFETTO
        MelatoninPerfetto tracingSession;
#endif

        const auto shouldRun = [&commandLine](const juce::String& name) {
            return commandLine.isEmpty() || commandLine.containsIgnoreCase(name);
        };

        if (shouldRun("minimum"))
            MinimumViewBenchmark{}.run();
        if (shouldRun("flex"))
            FlexStressTest{}.run();
        if (shouldRun("text-resize"))
        {
            TextResizeBenchmark{ false }.run();
            TextResizeBenchmark{ true }.run();
        }
        if (shouldRun("cards"))
        {
            CardsBenchmark{ CardsBenchmark::Scenario::build }.run();
            CardsBenchmark{ CardsBenchmark::Scenario::cardTextChange }.run();
            CardsBenchmark{ CardsBenchmark::Scenario::headingTextChange }.run();
        }

        quit();
    }

    void shutdown() final
    {
    }

private:
};

START_JUCE_APPLICATION(BenchmarkApp)
