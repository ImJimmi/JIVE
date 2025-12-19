#include <jive_layouts/jive_layouts.h>

#if JIVE_ENABLE_MELATONIN_INSPECTOR
    #include <melatonin_inspector/melatonin_inspector.h>
#endif

namespace jive::xmlViews
{
    class App : public juce::JUCEApplication
    {
    public:
        const juce::String getApplicationName() final
        {
            return "XML Views";
        }

        const juce::String getApplicationVersion() final
        {
            return "1.0.0";
        }

        void initialise(const juce::String&) final
        {
            static const juce::File sourceDir{ JIVE_XML_VIEWS_SOURCE_DIR };

            interpreter.addSourceDirectory(sourceDir.getChildFile("views"));
            window = interpreter.interpret(sourceDir.getChildFile("main.xml"));

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
        jive::Interpreter interpreter;
        std::unique_ptr<jive::GuiItem> window;

#if JIVE_ENABLE_MELATONIN_INSPECTOR
        std::unique_ptr<melatonin::Inspector> inspector;
#endif
    };
} // namespace jive::xmlViews

START_JUCE_APPLICATION(jive::xmlViews::App)
