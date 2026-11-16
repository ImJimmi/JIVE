#pragma once

#include <juce_core/juce_core.h>

namespace jive
{
    // Captures everything logged while it exists, for testing log messages.
    struct CapturingLogger : juce::Logger
    {
        CapturingLogger()
            : previousLogger{ juce::Logger::getCurrentLogger() }
        {
            juce::Logger::setCurrentLogger(this);
        }

        ~CapturingLogger() override
        {
            juce::Logger::setCurrentLogger(previousLogger);
        }

        void logMessage(const juce::String& message) final
        {
            messages.add(message);
        }

        [[nodiscard]] juce::String getLog() const
        {
            return messages.joinIntoString("\n");
        }

        juce::Logger* previousLogger;
        juce::StringArray messages;

        JUCE_DECLARE_NON_COPYABLE(CapturingLogger)
    };
} // namespace jive
