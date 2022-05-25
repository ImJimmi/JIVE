#include <jive_layouts/jive_layouts.h>

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    Button::Button(std::unique_ptr<GuiItem> itemToDecorate)
        : GuiItemDecorator{ std::move(itemToDecorate) }
        , TextWidget{ tree }
        , toggleable{ tree, "toggleable" }
        , toggled{ tree, "toggled" }
        , toggleOnClick{ tree, "toggle-on-click" }
        , radioGroup{ tree, "radio-group" }
    {
        onTextChanged = [this]() {
            getButton().setButtonText(getText());
        };
        getButton().setButtonText(getText());

        onFontChanged = [this]() {
            getComponent().getProperties().set("font", juce::VariantConverter<juce::Font>::toVar(getFont()));
        };
        getComponent().getProperties().set("font", juce::VariantConverter<juce::Font>::toVar(getFont()));

        onJustificationChanged = [this]() {
            getComponent().getProperties().set("justification", juce::VariantConverter<juce::Justification>::toVar(getTextJustification()));
        };
        getComponent().getProperties().set("justification", juce::VariantConverter<juce::Justification>::toVar(getTextJustification()));

        toggleable.onValueChange = [this]() {
            getButton().setToggleable(toggleable);
        };
        getButton().setToggleable(toggleable);

        toggled.onValueChange = [this]() {
            getButton().setToggleState(toggled, juce::sendNotification);
        };
        getButton().setToggleState(toggled, juce::sendNotification);

        toggleOnClick.onValueChange = [this]() {
            getButton().setClickingTogglesState(toggleOnClick);
        };
        getButton().setClickingTogglesState(toggleOnClick);

        radioGroup.onValueChange = [this]() {
            getButton().setRadioGroupId(radioGroup);
        };
        getButton().setRadioGroupId(radioGroup);
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
        testJustification();
        testToggleable();
        testClickingTogglesState();
        testRadioGroup();
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

    void testJustification()
    {
        beginTest("justification");

        using Converter = juce::VariantConverter<juce::Justification>;

        {
            juce::ValueTree tree{ "Button" };
            auto item = createButton(tree);

            expect(item->getComponent().getProperties().contains("justification"));

            tree.setProperty("justification",
                             Converter::toVar(juce::Justification::bottomRight),
                             nullptr);
            expect(Converter::fromVar(item->getComponent().getProperties()["justification"]) == juce::Justification::bottomRight);
        }
        {
            juce::ValueTree tree{
                "Button",
                {
                    { "justification", Converter::toVar(juce::Justification::topLeft) },
                },
            };
            auto item = createButton(tree);

            expect(Converter::fromVar(item->getComponent().getProperties()["justification"]) == juce::Justification::topLeft);
        }
    }

    void testToggleable()
    {
        beginTest("toggleable");

        {
            juce::ValueTree tree{ "Button" };
            auto item = createButton(tree);

            expect(!item->getButton().isToggleable());

            tree.setProperty("toggleable", true, nullptr);

            expect(item->getButton().isToggleable());
        }
        {
            juce::ValueTree tree{
                "Button",
                {
                    { "toggleable", true },
                },
            };
            auto item = createButton(tree);

            expect(item->getButton().isToggleable());
        }
    }

    void testToggled()
    {
        beginTest("toggled");

        {
            juce::ValueTree tree{ "Button" };
            auto item = createButton(tree);

            expect(!item->getButton().getToggleState());

            tree.setProperty("toggled", true, nullptr);

            expect(item->getButton().getToggleState());
        }
        {
            juce::ValueTree tree{
                "Button",
                {
                    { "toggled", true },
                },
            };
            auto item = createButton(tree);

            expect(item->getButton().getToggleState());
        }
    }

    void testClickingTogglesState()
    {
        beginTest("toggle-on-click");

        {
            juce::ValueTree tree{ "Button" };
            auto item = createButton(tree);
            expect(!item->getButton().getClickingTogglesState());

            tree.setProperty("toggle-on-click", true, nullptr);
            expect(item->getButton().getClickingTogglesState());
        }
        {
            juce::ValueTree tree{
                "Button",
                {
                    { "toggle-on-click", true },
                },
            };
            auto item = createButton(tree);
            expect(item->getButton().getClickingTogglesState());
        }
    }

    void testRadioGroup()
    {
        beginTest("radio-group");

        {
            juce::ValueTree tree{ "Button" };
            auto item = createButton(tree);
            expectEquals(item->getButton().getRadioGroupId(), 0);

            tree.setProperty("radio-group", 100, nullptr);
            expectEquals(item->getButton().getRadioGroupId(), 100);
        }
        {
            juce::ValueTree tree{
                "Button",
                {
                    { "radio-group", 12 },
                },
            };
            auto item = createButton(tree);
            expectEquals(item->getButton().getRadioGroupId(), 12);
        }
    }
};

static ButtonTest buttonTest;
#endif
