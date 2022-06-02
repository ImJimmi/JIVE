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
        , triggerEvent{ tree, "trigger-event", TriggerEvent::mouseUp }
        , tooltip{ tree, "tooltip" }
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

        triggerEvent.onValueChange = [this]() {
            getButton().setTriggeredOnMouseDown(triggerEvent == TriggerEvent::mouseDown);
        };
        getButton().setTriggeredOnMouseDown(triggerEvent == TriggerEvent::mouseDown);

        tooltip.onValueChange = [this]() {
            getButton().setTooltip(tooltip);
        };
        getButton().setTooltip(tooltip);
    }

    //==================================================================================================================
    float Button::getWidth() const
    {
        if (!hasAutoWidth())
            return GuiItemDecorator::getWidth();

        const auto textWidth = getFont().getStringWidthFloat(getText());
        const auto borderWidth = getBoxModel().getBorder().getLeftAndRight();

        return juce::jmax(textWidth + borderWidth, 20.f);
    }

    float Button::getHeight() const
    {
        if (!hasAutoWidth())
            return GuiItemDecorator::getHeight();

        const auto textHeight = getFont().getHeight();
        const auto borderHeight = getBoxModel().getBorder().getTopAndBottom();

        return juce::jmax(textHeight + borderHeight, 20.f);
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
        testTooltip();
        testAutoSize();
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

    void testTriggerEvent()
    {
        beginTest("trigger-event");

        {
            juce::ValueTree tree{ "Button" };
            auto item = createButton(tree);
            expect(!item->getButton().getTriggeredOnMouseDown());

            tree.setProperty("trigger-event", "mouse-down", nullptr);
            expect(item->getButton().getTriggeredOnMouseDown());
        }
        {
            juce::ValueTree tree{
                "Button",
                {
                    { "trigger-event", "mouse-down" },
                },
            };
            auto item = createButton(tree);
            expect(item->getButton().getTriggeredOnMouseDown());

            tree.setProperty("trigger-event", "mouse-up", nullptr);
            expect(!item->getButton().getTriggeredOnMouseDown());
        }
    }

    void testTooltip()
    {
        beginTest("tooltip");

        {
            juce::ValueTree tree{ "Button" };
            auto item = createButton(tree);
            expect(item->getButton().getTooltip().isEmpty());

            tree.setProperty("tooltip", "Click me!", nullptr);
            expectEquals(item->getButton().getTooltip(), juce::String{ "Click me!" });
        }
        {
            juce::ValueTree tree{
                "Button",
                {
                    { "tooltip", "OneTwoThree" },
                },
            };
            auto item = createButton(tree);
            expectEquals(item->getButton().getTooltip(), juce::String{ "OneTwoThree" });
        }
    }

    void testAutoSize()
    {
        beginTest("auto size");

        juce::ValueTree tree{ "Button" };
        auto button = createButton(tree);

        expectEquals(button->getWidth(), 20.f);
        expectEquals(button->getHeight(), 20.f);

        tree.setProperty("text", "Some text", nullptr);
        tree.setProperty("border-width", 30, nullptr);
        tree.setProperty("padding", 5, nullptr);

        const auto boxModel = button->getBoxModel();
        const auto borderWidth = boxModel.getBorder().getLeftAndRight();
        const auto textWidth = button->getFont().getStringWidthFloat(button->getText());
        const auto expectedWidth = borderWidth + textWidth;
        expect(button->getWidth() == expectedWidth);

        const auto borderHeight = boxModel.getBorder().getTopAndBottom();
        const auto textHeight = button->getFont().getHeight();
        const auto expectedHeight = borderHeight + textHeight;
        expect(button->getHeight() == expectedHeight);
    }
};

static ButtonTest buttonTest;
#endif
