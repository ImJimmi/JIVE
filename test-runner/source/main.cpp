#include <juce_gui_basics/juce_gui_basics.h>

//======================================================================================================================
auto getNumFailures(const juce::UnitTestRunner& runner)
{
    auto numFails = 0;

    for (auto i = 0; i < runner.getNumResults(); i++)
    {
        if (auto* result = runner.getResult(i))
            numFails += result->failures;
    }

    return numFails;
}

//======================================================================================================================
class TestRunner : public juce::JUCEApplication
{
public:
    //==================================================================================================================
    TestRunner() = default;

    //==================================================================================================================
    const juce::String getApplicationName() final
    {
        return "JIVE Test Runner";
    }

    const juce::String getApplicationVersion() final
    {
        return "1.0.0";
    }

    void initialise(const juce::String&) final
    {
        juce::UnitTestRunner runner;
        runner.setAssertOnFailure(false);

        runner.runTestsInCategory("JIVE");

        setApplicationReturnValue(getNumFailures(runner));
        quit();
    }

    void shutdown() final
    {
    }

private:
    //==================================================================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TestRunner)
};

//======================================================================================================================
START_JUCE_APPLICATION(TestRunner)
