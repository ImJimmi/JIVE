#pragma once

#include "Benchmark.h"

#include <jive_core/jive_core.h>

class PageSwitchBenchmark : public Benchmark
{
public:
    PageSwitchBenchmark()
        : Benchmark{
            "Page Switch (demo sections)",
            juce::RelativeTime::seconds(8.0),
        }
    {
    }

protected:
    void prepare(jive::Interpreter& interpreter) final
    {
        const juce::File viewsDirectory{ viewsPath };
        interpreter.addSourceDirectory(viewsDirectory);

        view = jive::parseXML(viewsDirectory
                                  .getChildFile("app.xml")
                                  .loadFileAsString());
        window = interpreter.interpret(view, nullptr);
        content = jive::findElementWithID(view, "content");

        if (auto* comp = window->getComponent().get())
            comp->setSize(980, 640);
    }

    void doIteration(jive::Interpreter&) final
    {
        static const juce::StringArray sections{
            "sections/layouts.xml",
            "sections/styles.xml",
            "sections/animations.xml",
            "sections/widgets.xml",
        };

        content.removeAllChildren(nullptr);
        content.appendChild(juce::ValueTree{
                                "Component",
                                {
                                    { "source", sections[index % sections.size()] },
                                    { "flex-grow", 1 },
                                    { "align-items", "stretch" },
                                },
                            },
                            nullptr);
        index++;

        if (auto* comp = window->getComponent().get())
        {
            comp->setBounds(comp->getBounds());

            juce::Image image{ juce::Image::ARGB, comp->getWidth(), comp->getHeight(), true };
            juce::Graphics g{ image };
            comp->paintEntireComponent(g, true);
        }
    }

private:
    static constexpr auto viewsPath = JIVE_DEMO_VIEWS_SOURCE_DIRECTORY;

    juce::ValueTree view;
    juce::ValueTree content;
    std::unique_ptr<jive::GuiItem> window;
    int index = 0;
};
