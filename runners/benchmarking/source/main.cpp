#include "FlexStressTest.h"
#include "MinimumViewBenchmark.h"
#include "StyleSheetsBenchmark.h"

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
        StyleSheetsConstructionBenchmark{}.run();
        StyleSheetsQueryingBenchmark{}.run();
        MinimumViewBenchmark{}.run();
        FlexStressTest{}.run();
        quit();
    }

    void shutdown() final
    {
    }

private:
};

START_JUCE_APPLICATION(BenchmarkApp)
