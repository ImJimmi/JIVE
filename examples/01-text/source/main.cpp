#include "Presenter.h"

namespace jive_examples::text
{
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
} // namespace jive_examples::text

START_JUCE_APPLICATION(jive_examples::text::Application)
