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
};

static ButtonTest buttonTest;
#endif
