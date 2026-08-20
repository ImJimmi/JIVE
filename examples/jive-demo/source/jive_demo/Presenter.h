#pragma once

#include <jive_layouts/jive_layouts.h>

#if JIVE_ENABLE_MELATONIN_INSPECTOR
    #include <melatonin_inspector/melatonin_inspector.h>
#endif

namespace jive_demo
{
    class Presenter
    {
    public:
        Presenter()
            : state{
                "Window",
                {
                    { "width", 800 },
                    { "height", 500 },
                },
                {
                    juce::ValueTree{
                        "Component",
                        {
                            { "source", "Main.xml" },
                        },
                    },
                },
            }
        {
            interpreter.addSourceDirectory(juce::File{ JIVE_DEMO_VIEWS_SOURCE_DIRECTORY });
            window = interpreter.interpret(state);

#if JIVE_ENABLE_MELATONIN_INSPECTOR
            inspector = std::make_unique<melatonin::Inspector>(*window->getComponent(), false);
            inspector->setVisible(true);
#endif

            layoutsButtonClicked = std::make_unique<jive::Event>(
                jive::findElementsWithText(jive::findElementsWithClass(state, "nav-button"), "Layouts")[0],
                "on-click",
                [this]() {
                    setDemoPage("Layouts");
                });

            setDemoPage("Layouts");
        }

    private:
        void setDemoPage(const juce::String& name)
        {
            jive::findElementWithID(state, "demo")
                .setProperty("source",
                             "views/demos/" + name + "Demo.xml",
                             nullptr);
        }

        juce::ValueTree state;
        jive::Interpreter interpreter;
        std::unique_ptr<jive::GuiItem> window;

#if JIVE_ENABLE_MELATONIN_INSPECTOR
        std::unique_ptr<melatonin::Inspector> inspector;
#endif

        std::unique_ptr<jive::Event> layoutsButtonClicked;
    };
} // namespace jive_demo
