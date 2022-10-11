#include <jive_layouts/jive_layouts.h>

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    Hyperlink::Hyperlink(std::unique_ptr<GuiItem> itemToDecorate)
        : Button(std::move(itemToDecorate))
        , url{ state, "url" }
        , fontStyle{ state, "font-style", "underlined" }
    {
        url.onValueChange = [this]() {
            getHyperlink().setURL(url);
        };
        getHyperlink().setURL(url);
    }

    //==================================================================================================================
    bool Hyperlink::isContainer() const
    {
        return false;
    }

    //==================================================================================================================
    juce::HyperlinkButton& Hyperlink::getHyperlink()
    {
        return *dynamic_cast<juce::HyperlinkButton*>(&getButton());
    }

    const juce::HyperlinkButton& Hyperlink::getHyperlink() const
    {
        return *dynamic_cast<const juce::HyperlinkButton*>(&getButton());
    }
} // namespace jive

//======================================================================================================================
#if JIVE_UNIT_TESTS
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
    std::unique_ptr<jive::Hyperlink> createHyperlink(juce::ValueTree tree)
    {
        jive::Interpreter interpreter;

        return std::make_unique<jive::Hyperlink>(interpreter.interpret(tree));
    }

    void testGuiItem()
    {
        beginTest("gui-item");

        auto item = createHyperlink(juce::ValueTree{ "Hyperlink" });
        expect(!item->isContainer());
    }

    void testURL()
    {
        beginTest("url");

        {
            juce::ValueTree tree{ "Hyperlink" };
            auto hyperlink = createHyperlink(tree);
            expect(hyperlink->getHyperlink().getURL().isEmpty());

            tree.setProperty("url", "example.com", nullptr);
            expect(hyperlink->getHyperlink().getURL() == juce::URL{ "example.com" });
        }
        {
            juce::ValueTree tree{
                "Hyperlink",
                {
                    { "url", "github.com/ImJimmi/JIVE/blob/main/jive_layouts/layout/gui-items/widgets/jive_Hyperlink.cpp" },
                },
            };
            auto hyperlink = createHyperlink(tree);
            expect(hyperlink->getHyperlink().getURL() == juce::URL{ "github.com/ImJimmi/JIVE/blob/main/jive_layouts/layout/gui-items/widgets/jive_Hyperlink.cpp" });
        }
    }
};

static HyperlinkTest hyperlinkTest;
#endif
