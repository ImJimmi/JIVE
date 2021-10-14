#pragma once

#include <juce_core/juce_core.h>

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    class UnitTest : public juce::UnitTest
    {
    public:
        //==============================================================================================================
        explicit UnitTest(const juce::String& testName)
            : juce::UnitTest{ testName }
        {
        }

    protected:
        //==============================================================================================================
        void GIVEN(const juce::String& message)
        {
            logMessage("    GIVEN       " + message);
        }

        void WHEN(const juce::String& message)
        {
            logMessage("      WHEN      " + message);
        }

        void THEN(const juce::String& message)
        {
            logMessage("        THEN    " + message);
        }
    };
} // namespace jive
