#pragma once

#include "Presenter.h"

namespace jive_demo
{
    class Application : public juce::JUCEApplication
    {
    public:
        Application() = default;

        const juce::String getApplicationName() final
        {
            return JUCE_APPLICATION_NAME;
        }

        const juce::String getApplicationVersion() final
        {
            return JUCE_APPLICATION_VERSION;
        }

        void initialise(const juce::String& /*commandLineArguments*/) final
        {
            presenter = std::make_unique<Presenter>();
        }

        void shutdown() final
        {
            presenter = nullptr;
        }

    private:
        std::unique_ptr<Presenter> presenter;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Application)
    };
} // namespace jive_demo
