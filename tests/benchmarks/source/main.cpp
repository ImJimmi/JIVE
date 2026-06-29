#include "FlexStressTest.h"
#include "MinimumViewBenchmark.h"
#include "PageSwitchBenchmark.h"

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

    void initialise(const juce::String&) final
    {
#if PERFETTO
        MelatoninPerfetto tracingSession;
#endif

        MinimumViewBenchmark{}.run();
        FlexStressTest{}.run();
        PageSwitchBenchmark{}.run();
        quit();
    }

    void shutdown() final
    {
    }

private:
};

START_JUCE_APPLICATION(BenchmarkApp)
