#pragma once

#include "Fonts.h"

#include <jive_demo/sections/AnimationsSection.h>
#include <jive_demo/sections/LayoutsSection.h>
#include <jive_demo/sections/StylesSection.h>
#include <jive_demo/sections/WidgetsSection.h>

#include <jive_layouts/jive_layouts.h>

namespace jive_demo
{
    // Drives the whole demo: interprets the top-level view from disk (so the
    // markup and styles hot-reload while running), wires the sidebar navigation,
    // and hands each section its live value-tree so it can bind its controls.
    class DemoController
    {
    public:
        [[nodiscard]] std::unique_ptr<jive::GuiItem> create(const juce::String& topLevelFile,
                                                            juce::AudioProcessor* processor = nullptr)
        {
            const juce::File viewsDirectory{ JIVE_DEMO_VIEWS_SOURCE_DIRECTORY };
            interpreter.addSourceDirectory(viewsDirectory);

            view = jive::parseXML(viewsDirectory
                                      .getChildFile(topLevelFile)
                                      .loadFileAsString());
            auto item = interpreter.interpret(view, processor);

            content = jive::findElementWithID(view, "content");

            wireNavigation();
            showSection(0);

            return item;
        }

    private:
        void wireNavigation()
        {
            static const juce::StringArray navIDs{
                "nav-layouts",
                "nav-styles",
                "nav-animations",
                "nav-widgets",
            };

            navEvents.clear();

            for (auto i = 0; i < navIDs.size(); i++)
            {
                auto button = jive::findElementWithID(view, navIDs[i]);

                if (button.isValid())
                {
                    navEvents.push_back(std::make_unique<jive::Event>(button,
                                                                      "on-click",
                                                                      [this, i] {
                                                                          showSection(i);
                                                                      }));
                }
            }
        }

        void showSection(int index)
        {
            if (!content.isValid() || index < 0 || index >= static_cast<int>(sections.size()))
                return;

            auto* section = sections[static_cast<std::size_t>(index)].get();

            content.removeAllChildren(nullptr);

            juce::ValueTree node{
                "Component",
                {
                    { "source", section->fileName() },
                    { "flex-grow", 1 },
                    { "align-items", "stretch" },
                },
            };
            content.appendChild(node, nullptr);

            // appendChild drives the interpreter synchronously, so by now the
            // section's markup has been merged into `node` and we can bind it.
            section->attach(node);
        }

        jive::Interpreter interpreter;
        Fonts fonts;

        juce::ValueTree view;
        juce::ValueTree content;

        std::array<std::unique_ptr<Section>, 4> sections{
            std::make_unique<LayoutsSection>(),
            std::make_unique<StylesSection>(),
            std::make_unique<AnimationsSection>(),
            std::make_unique<WidgetsSection>(),
        };
        std::vector<std::unique_ptr<jive::Event>> navEvents;
    };
} // namespace jive_demo
