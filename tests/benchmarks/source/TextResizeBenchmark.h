#pragma once

#include "Benchmark.h"

class TextResizeBenchmark : public Benchmark
{
public:
    explicit TextResizeBenchmark(bool shouldPaint)
        : Benchmark{
            shouldPaint ? "Resize 01-text (layout + paint)" : "Resize 01-text (layout)",
            juce::RelativeTime::seconds(8.0),
        }
        , paint{ shouldPaint }
    {
    }

protected:
    void prepare(jive::Interpreter& interpreter) final
    {
        const juce::File viewsDirectory{ viewsPath };
        interpreter.addSourceDirectory(viewsDirectory);

        for (const auto* font : { "rubik.ttf", "rubik-mono-one.ttf", "source-code-pro.ttf" })
        {
            juce::MemoryBlock data;
            viewsDirectory.getChildFile("../../../resources").getChildFile(font).loadFileAsData(data);
            fonts.push_back(juce::Typeface::createSystemTypefaceFor(data.getData(), data.getSize()));
        }

        const auto xml = viewsDirectory
                             .getChildFile("Main.xml")
                             .loadFileAsString()
                             .replace("<Window", "<Component")
                             .replace("</Window>", "</Component>");
        window = interpreter.interpret(jive::parseXML(xml), nullptr);
    }

    void doIteration(jive::Interpreter&) final
    {
        static constexpr auto minWidth = 800;
        static constexpr auto maxWidth = 1000;
        static constexpr auto step = 4;

        width += direction * step;

        if (width <= minWidth || width >= maxWidth)
            direction = -direction;

        auto& component = *window->getComponent();
        component.setSize(width, 780);

        if (paint)
        {
            juce::Image image{ juce::Image::ARGB, component.getWidth(), component.getHeight(), true };
            juce::Graphics g{ image };
            component.paintEntireComponent(g, true);
        }
    }

private:
    static constexpr auto viewsPath = JIVE_EXAMPLE_01_TEXT_VIEWS_DIRECTORY;

    const bool paint;
    std::vector<juce::Typeface::Ptr> fonts;
    std::unique_ptr<jive::GuiItem> window;
    int width = 1000;
    int direction = -1;
};
