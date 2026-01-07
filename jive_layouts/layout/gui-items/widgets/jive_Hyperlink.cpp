#include "jive_Hyperlink.h"

namespace jive
{
    Hyperlink::Hyperlink(std::unique_ptr<GuiItem> itemToDecorate)
        : Button(std::move(itemToDecorate))
        , url{ state, "url" }
    {
        url.onValueChange = [this]() {
            getHyperlink().setURL(url);
        };
        getHyperlink().setURL(url);
    }

    bool Hyperlink::isContainer() const
    {
        return false;
    }

    juce::HyperlinkButton& Hyperlink::getHyperlink()
    {
        return *dynamic_cast<juce::HyperlinkButton*>(&getButton());
    }

    const juce::HyperlinkButton& Hyperlink::getHyperlink() const
    {
        return *dynamic_cast<const juce::HyperlinkButton*>(&getButton());
    }
} // namespace jive

#if JIVE_UNIT_TESTS
    #include <jive_layouts/layout/interpreter/jive_Interpreter.h>

class HyperlinkTest : public juce::UnitTest
{
public:
    HyperlinkTest()
        : juce::UnitTest("jive::Hyperlink", "jive")
    {
    }

    void runTest() final
    {
        testGuiItem();
        testURL();
    }

private:
    jive::Interpreter interpreter;

    [[nodiscard]] auto& getLink(jive::GuiItem& item)
    {
        return dynamic_cast<jive::GuiItemDecorator&>(item).toType<jive::Hyperlink>()->getHyperlink();
    }

    void testGuiItem()
    {
        beginTest("gui-item");

        auto item = interpreter.interpret(juce::ValueTree{
            "Hyperlink",
            {
                { "width", 222 },
                { "height", 333 },
            },
        });
        expect(!item->isContainer());
    }

    void testURL()
    {
        beginTest("url");

        {
            juce::ValueTree tree{
                "Hyperlink",
                {
                    { "width", 222 },
                    { "height", 333 },
                },
            };
            auto item = interpreter.interpret(tree);
            auto& hyperlink = getLink(*item);
            expect(hyperlink.getURL().isEmpty());

            tree.setProperty("url", "example.com", nullptr);
            expect(hyperlink.getURL() == juce::URL{ "example.com" });
        }
        {
            juce::ValueTree tree{
                "Hyperlink",
                {
                    { "width", 222 },
                    { "height", 333 },
                    { "url", "github.com/ImJimmi/JIVE/blob/main/jive_layouts/layout/gui-items/widgets/jive_Hyperlink.cpp" },
                },
            };
            auto item = interpreter.interpret(tree);
            auto& hyperlink = getLink(*item);
            expect(hyperlink.getURL() == juce::URL{ "github.com/ImJimmi/JIVE/blob/main/jive_layouts/layout/gui-items/widgets/jive_Hyperlink.cpp" });
        }
    }
};

static HyperlinkTest hyperlinkTest;
#endif
