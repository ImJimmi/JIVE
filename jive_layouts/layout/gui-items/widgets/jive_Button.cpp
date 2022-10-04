#include <jive_layouts/jive_layouts.h>

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    Button::Button(std::unique_ptr<GuiItem> itemToDecorate)
        : GuiItemDecorator{ std::move(itemToDecorate) }
        , toggleable{ tree, "toggleable" }
        , toggled{ tree, "toggled" }
        , toggleOnClick{ tree, "toggle-on-click" }
        , radioGroup{ tree, "radio-group" }
        , triggerEvent{ tree, "trigger-event", TriggerEvent::mouseUp }
        , tooltip{ tree, "tooltip" }
    {
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
    bool Button::isContainer() const
    {
        return false;
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

    //==================================================================================================================
    void Button::contentChanged()
    {
        GuiItemDecorator::contentChanged();

        if (auto* text = findFirstTextContent(*this))
            getButton().setButtonText(text->getTextComponent().getText());
    }
} // namespace jive

//======================================================================================================================
namespace juce
{
    //==================================================================================================================
    const Array<var> VariantConverter<jive::Button::TriggerEvent>::options = {
        "mouse-up",
        "mouse-down",
    };

    jive::Button::TriggerEvent VariantConverter<jive::Button::TriggerEvent>::fromVar(const var& v)
    {
        jassert(options.contains(v));
        return static_cast<jive::Button::TriggerEvent>(options.indexOf(v));
    }

    var VariantConverter<jive::Button::TriggerEvent>::toVar(const jive::Button::TriggerEvent& event)
    {
        const auto index = static_cast<int>(event);

        jassert(options.size() >= index);
        return options[index];
    }
} // namespace juce

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
        testGuiItem();
        testToggleable();
        testClickingTogglesState();
        testRadioGroup();
        testTooltip();
        testContentChanged();
    }

private:
    std::unique_ptr<jive::Button> createButton(juce::ValueTree tree)
    {
        jive::Interpreter interpreter;

        return std::make_unique<jive::Button>(interpreter.interpret(tree));
    }

    void testGuiItem()
    {
        beginTest("gui-item");

        auto item = createButton(juce::ValueTree{ "Button" });
        expect(!item->isContainer());
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

    void testContentChanged()
    {
        beginTest("content-changed");

        juce::ValueTree tree{
            "Button",
            {},
            {
                juce::ValueTree{
                    "Text",
                    {
                        { "text", "Some text..." },
                    },
                },
            },
        };
        auto button = createButton(tree);
        expectEquals<juce::String>(button->getButton().getButtonText(), "Some text...");

        tree.getChild(0).setProperty("text", "Some different text!", nullptr);
        expectEquals<juce::String>(button->getButton().getButtonText(), "Some different text!");
    }
};

static ButtonTest buttonTest;
#endif
