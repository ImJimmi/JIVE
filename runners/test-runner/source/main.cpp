#include <juce_gui_basics/juce_gui_basics.h>

//======================================================================================================================
class TestRunner
    : public juce::JUCEApplication
    , private juce::UnitTestRunner
{
public:
    //==================================================================================================================
    TestRunner()
    {
        setPassesAreLogged(false);
    }

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
        runTestsInCategory("jive");

        setApplicationReturnValue(getNumFailures());
        quit();
    }

    void shutdown() final
    {
    }

    bool moreThanOneInstanceAllowed() final
    {
        return false;
    }

private:
    //==================================================================================================================
    int getNumFailures() const
    {
        auto numFailures = 0;

        for (auto i = 0; i < getNumResults(); i++)
        {
            if (auto* result = getResult(i))
                numFailures += result->failures;
        }

        return numFailures;
    }

    //==================================================================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TestRunner)
};

//======================================================================================================================
START_JUCE_APPLICATION(TestRunner)
