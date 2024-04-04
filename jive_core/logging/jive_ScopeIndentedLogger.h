#pragma once

#include <juce_core/juce_core.h>

namespace jive
{
    class ScopeIndentedLogger
    {
    public:
        explicit ScopeIndentedLogger(const juce::String& message);
        ~ScopeIndentedLogger();

    private:
        class Implementation;
    };
} // namespace jive
