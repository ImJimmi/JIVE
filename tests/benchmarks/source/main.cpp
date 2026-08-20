#include "FlexStressTest.h"
#include "MinimumViewBenchmark.h"
#include "PageSwitchBenchmark.h"
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
        if (shouldRun("page-switch"))
            PageSwitchBenchmark{}.run();
        if (shouldRun("text-resize"))
        {
            TextResizeBenchmark{ false }.run();
            TextResizeBenchmark{ true }.run();
        }

        quit();
    }

    void shutdown() final
    {
    }

private:
};

START_JUCE_APPLICATION(BenchmarkApp)
