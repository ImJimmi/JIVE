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
            : uiState{
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
            interpreter.onExternalSourceLoaded = [this](const jive::SourceFile&) {
                // A reloaded source file is swapped in as a brand-new tree, so
                // anything listening to the old one needs reassigning.
                if (window != nullptr)
                    refreshBindings();
            };
            window = interpreter.interpret(uiState);

#if JIVE_ENABLE_MELATONIN_INSPECTOR
            inspector = std::make_unique<melatonin::Inspector>(*window->getComponent(), false);
            inspector->setVisible(true);
#endif

            refreshBindings();
        }

    private:
        void refreshBindings()
        {
            const auto layoutsButtons = jive::findElementsWithText(jive::findElementsWithClass(uiState, "nav-button"),
                                                                   "Layouts");
            layoutsButtonClicked = layoutsButtons.empty()
                                     ? nullptr
                                     : std::make_unique<jive::Event>(layoutsButtons[0],
                                                                     "on-click",
                                                                     [this]() {
                                                                         setDemoPage("Layouts");
                                                                     });

            setDemoPage(currentDemoPage);
        }

        void setDemoPage(const juce::String& name)
        {
            currentDemoPage = name;
            demoButtonToggles.clear();

            if (name == "Layouts")
                presentLayoutsDemo();
        }

        void presentLayoutsDemo()
        {
            jive::findElementWithID(uiState, "demo")
                .setProperty("source",
                             "views/demos/LayoutsDemo.xml",
                             nullptr);

            static const std::unordered_map<juce::Identifier, juce::String> flexDirectionButtons{
                { "flex-direction-row", "row" },
                { "flex-direction-column", "column" },
                { "flex-direction-row-reverse", "row-reverse" },
                { "flex-direction-column-reverse", "column-reverse" },
            };

            for (const auto& [button, direction] : flexDirectionButtons)
            {
                onDemoButtonToggledOn(button, [this, dir = direction]() {
                    jive::findElementWithID(uiState, "flex-demo-container")
                        .setProperty("flex-direction", dir, nullptr);

                    jive::findElementWithID(uiState, "flex-direction-label")
                        .setProperty("text", dir, nullptr);
                });
            }
        }

        void onDemoButtonToggledOn(const juce::Identifier& buttonID, std::function<void()> callback)
        {
            auto& toggled = *demoButtonToggles.emplace_back(
                std::make_unique<jive::Property<bool>>(jive::findElementWithID(uiState, buttonID),
                                                       "toggled"));

            toggled.onValueChange = [&toggled, onToggledOn = std::move(callback)]() {
                if (toggled.get())
                    onToggledOn();
            };
        }

        juce::ValueTree uiState;
        jive::Interpreter interpreter;
        std::unique_ptr<jive::GuiItem> window;

#if JIVE_ENABLE_MELATONIN_INSPECTOR
        std::unique_ptr<melatonin::Inspector> inspector;
#endif

        juce::String currentDemoPage = "Layouts";
        std::unique_ptr<jive::Event> layoutsButtonClicked;
        std::vector<std::unique_ptr<jive::Property<bool>>> demoButtonToggles;
    };
} // namespace jive_demo
