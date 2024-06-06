#pragma once

#include "DemoState.h"

#include <jive_demo/gui/WindowPresenter.h>
#include <jive_demo/gui/tokens/Typography.h>
#include <melatonin_inspector/melatonin_inspector.h>

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

            inspector = std::make_unique<melatonin::Inspector>(*window->getComponent(), false);
            inspector->setVisible(true);
        }

        void shutdown() final
        {
            window = nullptr;
            inspector = nullptr;
        }

    private:
        DemoState state;

        jive::Interpreter interpreter;
        WindowPresenter windowPresenter{ state.getWindowState(), "Window" };
        std::unique_ptr<jive::GuiItem> window;

        std::unique_ptr<melatonin::Inspector> inspector;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Application)
    };
} // namespace jive_demo
