#pragma once

#include <juce_core/juce_core.h>

namespace jive
{
    class ScopeIndentedLogger
    {
    public:
        explicit ScopeIndentedLogger(const juce::String& message);
        JUCE_DECLARE_NON_COPYABLE(ScopeIndentedLogger)
        JUCE_DECLARE_NON_MOVEABLE(ScopeIndentedLogger)
        ~ScopeIndentedLogger();

    private:
        class Implementation;
    };
} // namespace jive
