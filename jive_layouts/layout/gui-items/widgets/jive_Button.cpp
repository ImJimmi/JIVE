#include <jive_layouts/jive_layouts.h>

namespace jive
{
    static void triggerClick(juce::Button& button)
    {
#if JIVE_UNIT_TESTS
        // Pretty horrible hack to have the triggerClick command message
        // handled synchronously for the sake of testing.
        dynamic_cast<juce::Component&>(button)
            .handleCommandMessage(0x2f3f4f99);
#else
        button.triggerClick();
#endif
    }

    Button::Button(std::unique_ptr<GuiItem> itemToDecorate)
        : GuiItemDecorator{ std::move(itemToDecorate) }
        , toggleable{ state, "toggleable" }
        , toggled{ state, "toggled" }
        , toggleOnClick{ state, "toggle-on-click" }
        , radioGroup{ state, "radio-group" }
        , triggerEvent{ state, "trigger-event" }
        , tooltip{ state, "tooltip" }
        , flexDirection{ state, "flex-direction" }
        , justifyContent{ state, "justify-content" }
        , padding{ state, "padding" }
        , minWidth{ state, "min-width" }
        , minHeight{ state, "min-height" }
        , onClick{ state, "on-click" }
    {
        if (!triggerEvent.exists())
            triggerEvent = TriggerEvent::mouseUp;
        if (!padding.exists())
            padding = juce::BorderSize{ 0.0f, 5.0f, 0.0f, 5.0f };
        if (!minWidth.exists())
            minWidth = 50.0f;
        if (!minHeight.exists())
            minHeight = 20.0f;

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

        onClick.onTrigger = [this]() {
            triggerClick(getButton());
        };
        getButton().addListener(this);
    }

    Button::~Button()
    {
        getButton().removeListener(this);
    }

    juce::Button& Button::getButton()
    {
        return *dynamic_cast<juce::Button*>(component.get());
    }

    const juce::Button& Button::getButton() const
    {
        return *dynamic_cast<const juce::Button*>(component.get());
    }

    void Button::buttonClicked(juce::Button* button)
    {
        jassertquiet(button == &getButton());

        toggled = getButton().getToggleState();
        onClick.triggerWithoutSelfCallback();
    }
} // namespace jive

namespace juce
{
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
        testToggleable();
        testClickingTogglesState();
        testRadioGroup();
        testTooltip();
        testDefaultSize();
        testEvents();
    }

private:
    std::unique_ptr<jive::Button> createButton(juce::ValueTree tree)
    {
        jive::Interpreter interpreter;

        return std::make_unique<jive::Button>(interpreter.interpret(tree));
    }

    void testToggleable()
    {
        beginTest("toggleable");

        {
            juce::ValueTree tree{
                "Button",
                {
                    { "width", 222 },
                    { "height", 333 },
                },
            };
            auto item = createButton(tree);

            expect(!item->getButton().isToggleable());

            tree.setProperty("toggleable", true, nullptr);

            expect(item->getButton().isToggleable());
        }
        {
            juce::ValueTree tree{
                "Button",
                {
                    { "width", 222 },
                    { "height", 333 },
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
            juce::ValueTree tree{
                "Button",
                {
                    { "width", 222 },
                    { "height", 333 },
                },
            };
            auto item = createButton(tree);

            expect(!item->getButton().getToggleState());

            tree.setProperty("toggled", true, nullptr);

            expect(item->getButton().getToggleState());
        }
        {
            juce::ValueTree tree{
                "Button",
                {
                    { "width", 222 },
                    { "height", 333 },
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
            juce::ValueTree tree{
                "Button",
                {
                    { "width", 222 },
                    { "height", 333 },
                },
            };
            auto item = createButton(tree);
            expect(!item->getButton().getClickingTogglesState());

            tree.setProperty("toggle-on-click", true, nullptr);
            expect(item->getButton().getClickingTogglesState());
        }
        {
            juce::ValueTree tree{
                "Button",
                {
                    { "width", 222 },
                    { "height", 333 },
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
            juce::ValueTree tree{
                "Button",
                {
                    { "width", 222 },
                    { "height", 333 },
                },
            };
            auto item = createButton(tree);
            expectEquals(item->getButton().getRadioGroupId(), 0);

            tree.setProperty("radio-group", 100, nullptr);
            expectEquals(item->getButton().getRadioGroupId(), 100);
        }
        {
            juce::ValueTree tree{
                "Button",
                {
                    { "width", 222 },
                    { "height", 333 },
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
            juce::ValueTree tree{
                "Button",
                {
                    { "width", 222 },
                    { "height", 333 },
                },
            };
            auto item = createButton(tree);
            expect(!item->getButton().getTriggeredOnMouseDown());

            tree.setProperty("trigger-event", "mouse-down", nullptr);
            expect(item->getButton().getTriggeredOnMouseDown());
        }
        {
            juce::ValueTree tree{
                "Button",
                {
                    { "width", 222 },
                    { "height", 333 },
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
            juce::ValueTree tree{
                "Button",
                {
                    { "width", 222 },
                    { "height", 333 },
                },
            };
            auto item = createButton(tree);
            expect(item->getButton().getTooltip().isEmpty());

            tree.setProperty("tooltip", "Click me!", nullptr);
            expectEquals(item->getButton().getTooltip(), juce::String{ "Click me!" });
        }
        {
            juce::ValueTree tree{
                "Button",
                {
                    { "width", 222 },
                    { "height", 333 },
                    { "tooltip", "OneTwoThree" },
                },
            };
            auto item = createButton(tree);
            expectEquals(item->getButton().getTooltip(), juce::String{ "OneTwoThree" });
        }
    }

    void testDefaultSize()
    {
        beginTest("default size");

        juce::ValueTree parentState{
            "Component",
            {
                { "display", "flex" },
                { "align-items", "flex-start" },
                { "width", 999 },
                { "height", 999 },
            },
            {
                juce::ValueTree{ "Button" },
            },
        };
        jive::Interpreter interpreter;
        auto parent = interpreter.interpret(parentState);
        auto& button = *dynamic_cast<jive::GuiItemDecorator&>(*parent->getChildren()[0])
                            .toType<jive::Button>();
        expectEquals(jive::BoxModel{ button.state }.getWidth(), 50.0f);
        expectEquals(jive::BoxModel{ button.state }.getHeight(), 20.0f);
    }

    void testEvents()
    {
        beginTest("events");

        juce::ValueTree parentState{
            "Component",
            {
                { "width", 100 },
                { "height", 100 },
            },
            {
                juce::ValueTree{ "Button" },
            },
        };
        jive::Interpreter interpreter;
        auto parent = interpreter.interpret(parentState);
        auto& button = *dynamic_cast<jive::GuiItemDecorator&>(*parent->getChildren()[0])
                            .toType<jive::Button>();

        jive::Event clickEvent{ parentState.getChild(0), "on-click" };
        expectEquals(clickEvent.getAssumedTriggerCount(), 0);

        jive::triggerClick(button.getButton());
        expectEquals(clickEvent.getAssumedTriggerCount(), 1);

        bool wasTriggered = false;
        button.getButton().onClick = [&wasTriggered]() {
            wasTriggered = true;
        };
        clickEvent.trigger();
        expect(wasTriggered);
    }
};

static ButtonTest buttonTest;
#endif
