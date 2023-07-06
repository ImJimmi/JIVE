#include "jive_ScopeIndentedLogger.h"

namespace jive
{
    class ScopeIndentedLogger::Implementation
        : public juce::Logger
        , private juce::DeletedAtShutdown
    {
    public:
        ~Implementation()
        {
            juce::Logger::setCurrentLogger(nullptr);
            clearSingletonInstance();
        }

        void incrementIndent()
        {
            indentLevel++;
        }

        void decrementIndent()
        {
            indentLevel = juce::jmax(0, indentLevel - 1);
        }

        JUCE_DECLARE_SINGLETON(Implementation, false)

    protected:
        void logMessage(const juce::String& message) final
        {
            for (auto indent = 0; indent < indentLevel; indent++)
                std::cout << "  ";

            std::cout << message << "\n";
        }

    private:
        juce::String previousMessage;
        int indentLevel = 0;
    };

    JUCE_IMPLEMENT_SINGLETON(ScopeIndentedLogger::Implementation)

    ScopeIndentedLogger::ScopeIndentedLogger(const juce::String& message)
    {
        juce::Logger::setCurrentLogger(Implementation::getInstance());
        juce::Logger::writeToLog(message);
        Implementation::getInstance()->incrementIndent();
    }

    ScopeIndentedLogger::~ScopeIndentedLogger()
    {
        Implementation::getInstance()->decrementIndent();
    }
} // namespace jive
