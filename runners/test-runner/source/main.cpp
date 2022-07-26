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
        setAssertOnFailure(false);
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
        logSuccessOrFailure();
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

protected:
    //==================================================================================================================
    void logMessage(const juce::String& message) final
    {
        if (message.contains("Starting test: "))
        {
            static constexpr auto includeSubstring = false;
            static constexpr auto ignoreCase = true;
            latestTestName = message.fromFirstOccurrenceOf("Starting test: ", includeSubstring, ignoreCase);
            latestTestName = latestTestName.upToLastOccurrenceOf("...", includeSubstring, ignoreCase);

            return;
        }

        if (message.contains("-----------------------------------------------------------------")
            || message.contains("All tests completed successfully")
            || message.contains("FAILED!!")
            || message.trim().isEmpty())
        {
            return;
        }

        DBG("[" << latestTestName << "] " << message.fromLastOccurrenceOf("!!! ", false, true));
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

    void logSuccessOrFailure()
    {
        DBG("\n================================");

        if (getNumFailures() == 0)
            DBG("ALL TESTS PASSED!");
        else
            DBG(juce::String{ getNumFailures() } << " tests failed!");

        DBG("================================\n");
    }

    //==================================================================================================================
    juce::String latestTestName;

    //==================================================================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TestRunner)
};

//======================================================================================================================
START_JUCE_APPLICATION(TestRunner)
