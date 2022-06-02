#include <jive_layouts/jive_layouts.h>

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    Hyperlink::Hyperlink(std::unique_ptr<GuiItem> itemToDecorate)
        : Button(std::move(itemToDecorate))
        , url{ tree, "url" }
        , fontStyle{ tree, "font-style", "underlined" }
    {
        url.onValueChange = [this]() {
            getHyperlink().setURL(url);
        };
        getHyperlink().setURL(url);

        static constexpr auto resizeFontToMatchWidgetHeight = false;
        onFontChanged = [this]() {
            getHyperlink().setFont(getFont(),
                                   resizeFontToMatchWidgetHeight);
        };
        getHyperlink().setFont(getFont(),
                               resizeFontToMatchWidgetHeight);

        onJustificationChanged = [this]() {
            getHyperlink().setJustificationType(getTextJustification());
        };
        getHyperlink().setJustificationType(getTextJustification());
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
        testURL();
        testFont();
        testJustification();
    }

private:
    std::unique_ptr<jive::Hyperlink> createHyperlink(juce::ValueTree tree)
    {
        return std::make_unique<jive::Hyperlink>(std::make_unique<jive::GuiItem>(std::make_unique<juce::HyperlinkButton>(),
                                                                                 tree));
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

    void testFont()
    {
        beginTest("font");

        juce::ValueTree tree{ "Hyperlink" };
        auto hyperlink = createHyperlink(tree);
        expect(hyperlink->getFont().isUnderlined());
    }

    void testJustification()
    {
        beginTest("justification");

        {
            juce::ValueTree tree{ "Hyperlink" };
            auto hyperlink = createHyperlink(tree);
            expect(hyperlink->getHyperlink().getJustificationType() == juce::Justification::centredLeft);

            tree.setProperty("justification", "centred", nullptr);
            expect(hyperlink->getHyperlink().getJustificationType() == juce::Justification::centred);
        }
        {
            juce::ValueTree tree{
                "Hyperlink",
                {
                    { "justification", "centred" },
                },
            };
            auto hyperlink = createHyperlink(tree);
            expect(hyperlink->getHyperlink().getJustificationType() == juce::Justification::centred);
        }
    }
};

static HyperlinkTest hyperlinkTest;
#endif
