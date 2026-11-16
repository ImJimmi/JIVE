#pragma once

#include "Benchmark.h"

class CardsBenchmark : public Benchmark
{
public:
    enum class Scenario
    {
        build,
        cardTextChange,
        headingTextChange,
    };

    explicit CardsBenchmark(Scenario benchmarkScenario)
        : Benchmark{
            describe(benchmarkScenario),
            juce::RelativeTime::seconds(8.0),
        }
        , scenario{ benchmarkScenario }
    {
    }

protected:
    void prepare(jive::Interpreter& interpreter) final
    {
        if (scenario != Scenario::build)
            page = interpreter.interpret(buildPage());
    }

    void doIteration(jive::Interpreter& interpreter) final
    {
        if (scenario == Scenario::build)
        {
            page = interpreter.interpret(buildPage());
            return;
        }

        static constexpr auto shortText = "Short.";
        static constexpr auto longText = "Considerably longer, so that it wraps onto a few more lines than it did before.";

        const auto main = page->state.getChild(1);
        auto text = scenario == Scenario::headingTextChange
                      ? main.getChild(0)
                      : main.getChild(1).getChild(card).getChild(1);
        text.setProperty("text", text["text"] == shortText ? longText : shortText, nullptr);

        card = (card + 1) % numCards;
    }

private:
    static constexpr auto numCards = 24;

    [[nodiscard]] static juce::String describe(Scenario scenario)
    {
        switch (scenario)
        {
        case Scenario::build:
            return "Build a page of cards";
        case Scenario::cardTextChange:
            return "Change the text of a card";
        case Scenario::headingTextChange:
            return "Change the heading above the cards";
        }

        return {};
    }

    [[nodiscard]] static juce::ValueTree buildPage()
    {
        juce::String sidebar;

        for (auto i = 0; i < 8; i++)
            sidebar << "<Button padding=\"6 10\"><Text>Navigation item " << i << "</Text></Button>";

        juce::String cards;

        for (auto i = 0; i < numCards; i++)
        {
            cards << "<Component width=\"260\" padding=\"12\" gap=\"8\" border-width=\"1\">"
                  << "    <Component flex-direction=\"row\" gap=\"8\" align-items=\"centre\">"
                  << "        <Component width=\"24\" height=\"24\" />"
                  << "        <Text>Card number " << i << "</Text>"
                  << "    </Component>"
                  << "    <Text>Some body text that is long enough that it needs to wrap onto more than one line.</Text>"
                  << "    <Component flex-direction=\"row\" gap=\"8\" justify-content=\"flex-end\">"
                  << "        <Button padding=\"4 8\"><Text>Cancel</Text></Button>"
                  << "        <Button padding=\"4 8\"><Text>OK</Text></Button>"
                  << "    </Component>"
                  << "</Component>";
        }

        return jive::parseXML("<Component width=\"1200\" height=\"900\" flex-direction=\"row\" padding=\"16\" gap=\"16\">"
                              "    <Component gap=\"8\" padding=\"8\">"
                              + sidebar
                              + "    </Component>"
                                "    <Component flex-grow=\"1\" gap=\"16\">"
                                "        <Text>Heading</Text>"
                                "        <Component flex-direction=\"row\" flex-wrap=\"wrap\" gap=\"16\">"
                              + cards
                              + "        </Component>"
                                "    </Component>"
                                "</Component>");
    }

    const Scenario scenario;
    std::unique_ptr<jive::GuiItem> page;
    int card = 0;
};
