#include "jive_Button.h"

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

    [[nodiscard]] static juce::Button* findFirstChildButton(const juce::Component& container)
    {
        for (auto* child : container.getChildren())
        {
            if (auto* button = dynamic_cast<juce::Button*>(child))
                return button;
        }

        return nullptr;
    }

    Button::Button(std::unique_ptr<GuiItem> itemToDecorate)
        : GuiItemDecorator{ std::move(itemToDecorate) }
        , toggleable{ state, "toggleable" }
        , toggled{ state, "toggled" }
        , toggleOnClick{ state, "toggle-on-click" }
        , radioGroup{ state, "radio-group" }
        , triggerEvent{ state, "trigger-event" }
        , tooltip{ state, "tooltip" }
        , text{ state, "text" }
        , minWidth{ state, "min-width" }
        , minHeight{ state, "min-height" }
        , focusable{ state, "focusable" }
        , flexJustifyContent{ state, "justify-content" }
        , flexAlignItems{ state, "align-items" }
        , padding{ state, "padding" }
        , onClick{ state, "on-click" }
    {
        const BoxModel::ScopedCallbackLock boxModelLock{ boxModel(*this) };

        if (!triggerEvent.exists())
            triggerEvent = TriggerEvent::mouseUp;
        if (!minWidth.exists())
            minWidth = 50.0f;
        if (!minHeight.exists())
            minHeight = 20.0f;
        if (!focusable.exists())
            focusable = true;
        if (!flexJustifyContent.exists())
            flexJustifyContent = juce::FlexBox::JustifyContent::center;
        if (!flexAlignItems.exists())
        {
            // Check-boxes draw their box at the left of their bounds, so their
            // content is left-aligned rather than centred.
            flexAlignItems = getCheckbox() != nullptr
                               ? juce::FlexBox::AlignItems::flexStart
                               : juce::FlexBox::AlignItems::center;
        }

        hasAutomaticPadding = !padding.exists();

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

        text.onValueChange = [this]() {
            getButton().setTitle(text);
        };
        getButton().setTitle(text);

        onClick.onTrigger = [this]() {
            triggerClick(getButton());
        };
        getButton().addListener(this);
        getButton().addComponentListener(this);
    }

    Button::~Button()
    {
        getButton().removeComponentListener(this);
        getButton().removeListener(this);
    }

#if JIVE_IS_PLUGIN_PROJECT
    void Button::attachToParameter(juce::RangedAudioParameter* parameter, juce::UndoManager* undoManager)
    {
        if (parameter != nullptr)
            parameterAttachment = std::make_unique<juce::ButtonParameterAttachment>(*parameter, getButton(), undoManager);
        else
            parameterAttachment = nullptr;
    }
#endif

    juce::Button& Button::getButton()
    {
        return *dynamic_cast<juce::Button*>(getComponent().get());
    }

    const juce::Button& Button::getButton() const
    {
        return *dynamic_cast<const juce::Button*>(getComponent().get());
    }

    void Button::layOutChildren()
    {
        updateCheckboxTextInset();
        GuiItemDecorator::layOutChildren();
    }

    const juce::ToggleButton* Button::getCheckbox() const
    {
        return dynamic_cast<const juce::ToggleButton*>(&getButton());
    }

    void Button::updateCheckboxTextInset()
    {
        if (!hasAutomaticPadding)
            return;

        const auto* checkbox = getCheckbox();

        if (checkbox == nullptr)
            return;

        if (auto* lookAndFeel = dynamic_cast<LookAndFeel*>(&checkbox->getLookAndFeel()))
        {
            const auto inset = lookAndFeel->getToggleButtonTextInset(*checkbox,
                                                                     lookAndFeel->findMostApplicableStyles(*checkbox));
            padding = juce::BorderSize<float>{ 0.0f, inset, 0.0f, 0.0f };
        }
    }

    void Button::buttonClicked(juce::Button* button)
    {
        jassertquiet(button == &getButton());

        toggled = getButton().getToggleState();
        onClick.triggerWithoutSelfCallback();
    }

    void Button::componentParentHierarchyChanged(juce::Component& comp)
    {
        jassertquiet(&comp == &getButton());

        if (radioGroup.get() != 0)
        {
            if (auto* parentComponent = getButton().getParentComponent())
                toggled = findFirstChildButton(*parentComponent) == &getButton();
        }
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
    #include <jive_layouts/layout/interpreter/jive_Interpreter.h>

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
        testCheckboxTextInset();
        testEvents();
    }

private:
    jive::Interpreter interpreter;

    [[nodiscard]] auto& getButton(jive::GuiItem& item)
    {
        return dynamic_cast<jive::GuiItemDecorator&>(item).toType<jive::Button>()->getButton();
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
            auto item = interpreter.interpret(tree);
            auto& button = getButton(*item);

            expect(!button.isToggleable());

            tree.setProperty("toggleable", true, nullptr);

            expect(button.isToggleable());
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
            auto item = interpreter.interpret(tree);
            auto& button = getButton(*item);

            expect(button.isToggleable());
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
            auto item = interpreter.interpret(tree);
            auto& button = getButton(*item);

            expect(!button.getToggleState());

            tree.setProperty("toggled", true, nullptr);

            expect(button.getToggleState());
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
            auto item = interpreter.interpret(tree);
            auto& button = getButton(*item);

            expect(button.getToggleState());
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
            auto item = interpreter.interpret(tree);
            auto& button = getButton(*item);
            expect(!button.getClickingTogglesState());

            tree.setProperty("toggle-on-click", true, nullptr);
            expect(button.getClickingTogglesState());
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
            auto item = interpreter.interpret(tree);
            auto& button = getButton(*item);
            expect(button.getClickingTogglesState());
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
            auto item = interpreter.interpret(tree);
            auto& button = getButton(*item);
            expectEquals(button.getRadioGroupId(), 0);

            tree.setProperty("radio-group", 100, nullptr);
            expectEquals(button.getRadioGroupId(), 100);
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
            auto item = interpreter.interpret(tree);
            auto& button = getButton(*item);
            expectEquals(button.getRadioGroupId(), 12);
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
            auto item = interpreter.interpret(tree);
            auto& button = getButton(*item);
            expect(!button.getTriggeredOnMouseDown());

            tree.setProperty("trigger-event", "mouse-down", nullptr);
            expect(button.getTriggeredOnMouseDown());
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
            auto item = interpreter.interpret(tree);
            auto& button = getButton(*item);
            expect(button.getTriggeredOnMouseDown());

            tree.setProperty("trigger-event", "mouse-up", nullptr);
            expect(!button.getTriggeredOnMouseDown());
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
            auto item = interpreter.interpret(tree);
            auto& button = getButton(*item);
            expect(button.getTooltip().isEmpty());

            tree.setProperty("tooltip", "Click me!", nullptr);
            expectEquals(button.getTooltip(), juce::String{ "Click me!" });
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
            auto item = interpreter.interpret(tree);
            auto& button = getButton(*item);
            expectEquals(button.getTooltip(), juce::String{ "OneTwoThree" });
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
        auto parent = interpreter.interpret(parentState);
        auto& button = *dynamic_cast<jive::GuiItemDecorator&>(*parent->getChildren()[0])
                            .toType<jive::Button>();
        const auto& boxModel = jive::boxModel(button);
        expectEquals(boxModel.getWidth(), 50.0f);
        expectEquals(boxModel.getHeight(), 20.0f);
    }

    void testCheckboxTextInset()
    {
        beginTest("check-boxes leave room for their box");

        juce::Component root;
        root.setSize(200, 100);
        jive::LookAndFeel lookAndFeel{ root };

        auto item = interpreter.interpret(jive::parseXML(R"(
            <Component width="200" height="100">
                <Checkbox>Some text</Checkbox>
            </Component>
        )"));
        auto* parentComponent = item->getComponent().get();
        root.addAndMakeVisible(*parentComponent);
        parentComponent->setBounds(0, 0, 200, 100);

        auto& checkbox = *item->getChildren()[0];
        const auto& toggleButton = dynamic_cast<const juce::ToggleButton&>(*checkbox.getComponent());
        const auto styles = lookAndFeel.findMostApplicableStyles(toggleButton);
        const auto boxBounds = lookAndFeel.getToggleButtonCheckboxBounds(toggleButton, styles);
        const auto* textComponent = checkbox.getChildren()[0]->getComponent().get();

        expectGreaterOrEqual(textComponent->getX(), boxBounds.getRight());
        expectWithinAbsoluteError(static_cast<float>(textComponent->getX()),
                                  lookAndFeel.getToggleButtonTextInset(toggleButton, styles),
                                  1.0f);

        root.removeChildComponent(parentComponent);
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
