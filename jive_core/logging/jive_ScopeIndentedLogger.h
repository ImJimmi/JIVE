#pragma once

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
