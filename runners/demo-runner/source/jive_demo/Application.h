#pragma once

#include "AppState.h"

#include <jive_demo/gui/WindowPresenter.h>
#include <jive_demo/gui/tokens/Typography.h>

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
            window = interpreter.interpret(windowPresenter.present());
            interpreter.listenTo(*window);
        }

        void shutdown() final
        {
            window = nullptr;
        }

    private:
        AppState state;

        jive::Interpreter interpreter;
        WindowPresenter windowPresenter{ state.getWindowState() };
        std::unique_ptr<jive::GuiItem> window;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Application)
    };
} // namespace jive_demo
