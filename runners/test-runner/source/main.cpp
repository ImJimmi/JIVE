#include <juce_gui_basics/juce_gui_basics.h>

class TestRunner
    : public juce::JUCEApplication
    , private juce::UnitTestRunner
{
public:
    TestRunner()
    {
        setPassesAreLogged(false);
        setAssertOnFailure(false);
    }

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
        auto elapsed = 0.0;

        {
            juce::ScopedTimeMeasurement timer{ elapsed };
            runTestsInCategory("jive");
        }

        logSuccessOrFailure(juce::RelativeTime::seconds(elapsed));
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
    void logMessage(const juce::String& message) final
    {
        if (message.contains("Starting test"))
        {
            static constexpr auto includeSubstring = false;
            static constexpr auto ignoreCase = true;
            latestTestName = message.fromFirstOccurrenceOf(": ", includeSubstring, ignoreCase);
            latestTestName = latestTestName.upToLastOccurrenceOf("...", includeSubstring, ignoreCase);

            return;
        }

        if (message.contains("-----------------------------------------------------------------")
            || message.contains("All tests completed successfully")
            || message.contains("FAILED!!")
            || message.trim().isEmpty()
            || message.contains("Completed test"))
        {
            return;
        }

        std::cout << "[" << latestTestName << "] " << message.fromLastOccurrenceOf("!!! ", false, true) << "\n";
    }

private:
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

    void logSuccessOrFailure(juce::RelativeTime elapsed)
    {
        std::cout << "\n================================\n";

        if (getNumFailures() == 0)
            std::cout << "ALL TESTS PASSED!\n";
        else
            std::cout << juce::String{ getNumFailures() } << " tests failed!\n";

        std::cout << "Took " << juce::String{ static_cast<double>(elapsed.inMilliseconds()) / 1000.0 } << "s\n";
        std::cout << "================================\n\n";
    }

    juce::String latestTestName;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TestRunner)
};

START_JUCE_APPLICATION(TestRunner)
