#pragma once

#include "DemoState.h"

#include <jive_demo/gui/WindowPresenter.h>
#include <jive_demo/gui/tokens/Typography.h>

#if JIVE_ENABLE_MELATONIN_INSPECTOR
    #include <melatonin_inspector/melatonin_inspector.h>
#endif

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

#if JIVE_ENABLE_MELATONIN_INSPECTOR
            inspector = std::make_unique<melatonin::Inspector>(*window->getComponent(), false);
            inspector->setVisible(true);
#endif
        }

        void shutdown() final
        {
            window = nullptr;

#if JIVE_ENABLE_MELATONIN_INSPECTOR
            inspector = nullptr;
#endif
        }

    private:
        DemoState state;

        jive::Interpreter interpreter;
        WindowPresenter windowPresenter{ state.getWindowState(), "Window" };
        std::unique_ptr<jive::GuiItem> window;

#if JIVE_ENABLE_MELATONIN_INSPECTOR
        std::unique_ptr<melatonin::Inspector> inspector;
#endif

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Application)
    };
} // namespace jive_demo
