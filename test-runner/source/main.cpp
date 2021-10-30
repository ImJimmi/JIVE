#include <catch2/catch_session.hpp>
#include <juce_gui_basics/juce_gui_basics.h>

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
        Catch::Session session;

        const auto returnValue = session.run();

        setApplicationReturnValue(returnValue);
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
