#include <jive_layouts/jive_layouts.h>

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    Button::Button(std::unique_ptr<GuiItem> itemToDecorate)
        : GuiItemDecorator{ std::move(itemToDecorate) }
        , TextWidget{ tree }
    {
        onTextChanged = [this]() {
            getButton().setButtonText(getText());
        };
        getButton().setButtonText(getText());

        onFontChanged = [this]() {
            getComponent().getProperties().set("font", juce::VariantConverter<juce::Font>::toVar(getFont()));
        };
        getComponent().getProperties().set("font", juce::VariantConverter<juce::Font>::toVar(getFont()));
    }

    //==================================================================================================================
    juce::Button& Button::getButton()
    {
        return *dynamic_cast<juce::Button*>(&getComponent());
    }

    const juce::Button& Button::getButton() const
    {
        return *dynamic_cast<const juce::Button*>(&getComponent());
    }
} // namespace jive

//======================================================================================================================
#if JIVE_UNIT_TESTS
class ButtonTest : public juce::UnitTest
{
public:
    ButtonTest()
        : juce::UnitTest{ "jive::Button", "jive" }
    {
    }

    void runTest() final
    {
        testText();
        testFont();
    }

private:
    std::unique_ptr<jive::Button> createButton(juce::ValueTree tree)
    {
        struct ConcreteButton : public juce::Button
        {
            ConcreteButton()
                : juce::Button{ "" }
            {
            }

            void paintButton(juce::Graphics&, bool, bool) final {}
        };

        return std::make_unique<jive::Button>(std::make_unique<jive::GuiItem>(std::make_unique<ConcreteButton>(),
                                                                              tree));
    }

    void testText()
    {
        beginTest("text");

        {
            juce::ValueTree tree{ "Button" };
            auto item = createButton(tree);

            expect(item->getButton().getButtonText().isEmpty());

            tree.setProperty("text", "Some text", nullptr);
            expectEquals(item->getButton().getButtonText(), juce::String{ "Some text" });
        }
        {
            juce::ValueTree tree{
                "Button",
                {
                    { "text", "Bacon sandwich" },
                },
            };
            auto item = createButton(tree);

            expectEquals(item->getButton().getButtonText(), juce::String{ "Bacon sandwich" });
        }
    }

    void testFont()
    {
        beginTest("font");

        {
            juce::ValueTree tree{ "Button" };
            auto item = createButton(tree);

            expect(item->getComponent().getProperties().contains("font"));

            juce::Font font{ "Helvetica", 16.f, 0 };
            tree.setProperty("typeface-name", font.getTypefaceName(), nullptr);
            tree.setProperty("font-weight", font.getTypefaceStyle(), nullptr);
            tree.setProperty("font-height", font.getHeightInPoints(), nullptr);
            expectEquals(item->getComponent().getProperties()["font"].toString(), font.toString());
        }
        {
            juce::Font font{ "Arial", 48.f, 0 };
            juce::ValueTree tree{
                "Button",
                {
                    { "typeface-name", font.getTypefaceName() },
                    { "font-weight", font.getTypefaceStyle() },
                    { "font-height", font.getHeightInPoints() },
                },
            };
            auto item = createButton(tree);

            expectEquals(item->getComponent().getProperties()["font"].toString(), font.toString());
        }
    }
};

static ButtonTest buttonTest;
#endif
