#pragma once

#include <jive_layouts/jive_layouts.h>

namespace jive_examples
{
    template <typename Presenter>
    class Application : public juce::JUCEApplication
    {
    public:
        const juce::String getApplicationName() final
        {
            return JUCE_APPLICATION_NAME;
        }

        const juce::String getApplicationVersion() final
        {
            return JUCE_APPLICATION_VERSION;
        }

        void initialise(const juce::String&) final
        {
            presenter = std::make_unique<Presenter>();
        }

        void shutdown() final
        {
            presenter = nullptr;
        }

    private:
        std::unique_ptr<Presenter> presenter;
    };
} // namespace jive_examples
