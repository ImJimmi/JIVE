#include <jive_layouts/jive_layouts.h>

namespace jive
{
    CommonGuiItem::CommonGuiItem(std::unique_ptr<GuiItem> itemToDecorate)
        : GuiItemDecorator{ std::move(itemToDecorate) }
        , boxModel{ state }
        , name{ state, "name" }
        , title{ state, "title" }
        , id{ state, "id" }
        , description{ state, "description" }
        , tooltip{ state, "tooltip" }
        , enabled{ state, "enabled", true }
        , visible{ state, "visible", true }
        , alwaysOnTop{ state, "always-on-top" }
        , bufferedToImage{ state, "buffered-to-image" }
        , opaque{ state, "opaque" }
        , focusable{ state, "focusable" }
        , clickingGrabsFocus{ state, "clicking-grabs-focus", true }
        , focusOutline{ state, "focus-outline" }
        , focusOrder{ state, "focus-order" }
        , opacity{ state, "opacity", 1.f }
        , cursor{ state, "cursor", juce::MouseCursor::NormalCursor }
        , display{ state, "display", Display::flex }
        , width{ state, "width", "auto" }
        , height{ state, "height", "auto" }
    {
        name.onValueChange = [this]() {
            component->setName(name);
        };
        component->setName(name);

        title.onValueChange = [this]() {
            component->setTitle(title);
        };
        component->setTitle(title);

        id.onValueChange = [this]() {
            component->setComponentID(id.get().toString());
        };
        component->setComponentID(id.get().toString());

        description.onValueChange = [this]() {
            component->setDescription(description);
        };
        component->setDescription(description);

        tooltip.onValueChange = [this]() {
            component->setHelpText(tooltip);
        };
        component->setHelpText(tooltip);

        enabled.onValueChange = [this]() {
            component->setEnabled(enabled);
        };
        component->setEnabled(enabled);

        visible.onValueChange = [this]() {
            component->setVisible(visible);
        };
        component->setVisible(visible);

        alwaysOnTop.onValueChange = [this]() {
            component->setAlwaysOnTop(alwaysOnTop);
        };
        component->setAlwaysOnTop(alwaysOnTop);

        bufferedToImage.onValueChange = [this]() {
            component->setBufferedToImage(bufferedToImage);
        };
        component->setBufferedToImage(bufferedToImage);

        opaque.onValueChange = [this]() {
            component->setOpaque(opaque);
        };
        component->setOpaque(opaque);

        focusable.onValueChange = [this]() {
            component->setWantsKeyboardFocus(focusable);
        };
        component->setWantsKeyboardFocus(focusable);

        focusOutline.onValueChange = [this]() {
            component->setHasFocusOutline(focusOutline);
        };
        component->setHasFocusOutline(focusOutline);

        clickingGrabsFocus.onValueChange = [this]() {
            component->setMouseClickGrabsKeyboardFocus(clickingGrabsFocus);
        };
        component->setMouseClickGrabsKeyboardFocus(clickingGrabsFocus);

        focusOrder.onValueChange = [this]() {
            component->setExplicitFocusOrder(focusOrder);
        };
        component->setExplicitFocusOrder(focusOrder);

        opacity.onValueChange = [this]() {
            component->setAlpha(opacity);
        };
        component->setAlpha(opacity);

        cursor.onValueChange = [this]() {
            component->setMouseCursor(juce::MouseCursor{ cursor });
        };
        component->setMouseCursor(juce::MouseCursor{ cursor });

        if (isTopLevel())
        {
            // Top-level items must have an explicit size!
            jassert(!boxModel.hasAutoWidth());
            jassert(!boxModel.hasAutoHeight());
        }

        component->setSize(juce::roundToInt(boxModel.getBounds().getWidth()),
                           juce::roundToInt(boxModel.getBounds().getHeight()));

        component->addComponentListener(this);
        boxModel.addListener(*this);
    }

    CommonGuiItem::~CommonGuiItem()
    {
        component->removeComponentListener(this);
    }

    void CommonGuiItem::componentMovedOrResized(juce::Component& componentThatWasMovedOrResized,
                                                bool /*wasMoved*/,
                                                bool wasResized)
    {
        if (&componentThatWasMovedOrResized != component.get())
            return;

        if (!wasResized)
            return;

        const auto componentBounds = component->getBounds().toFloat();
        boxModel.setSize(componentBounds.getWidth(),
                         componentBounds.getHeight());
    }

    void CommonGuiItem::componentVisibilityChanged(juce::Component& componentThatChangedVisiblity)
    {
        if (&componentThatChangedVisiblity != component.get())
            return;

        visible = component->isVisible();
    }

    void CommonGuiItem::componentNameChanged(juce::Component& componentThatChangedName)
    {
        if (&componentThatChangedName != component.get())
            return;

        name = component->getName();
    }

    void CommonGuiItem::componentEnablementChanged(juce::Component& componentThatChangedEnablement)
    {
        if (&componentThatChangedEnablement != component.get())
            return;

        enabled = component->isEnabled();
    }

    void CommonGuiItem::componentChildrenChanged(juce::Component& componentThatsChildrenChanged)
    {
        if (&componentThatsChildrenChanged != component.get())
            return;

        getTopLevelDecorator().layOutChildren();
    }

    void CommonGuiItem::boxModelChanged(BoxModel& boxModelThatChanged)
    {
        jassertquiet(&boxModelThatChanged == &boxModel);

        component->setSize(juce::roundToInt(boxModel.getWidth()),
                           juce::roundToInt(boxModel.getHeight()));
        getTopLevelDecorator().layOutChildren();
    }
} // namespace jive

#if JIVE_UNIT_TESTS
class CommonGuiItemUnitTest : public juce::UnitTest
{
public:
    CommonGuiItemUnitTest()
        : juce::UnitTest{ "jive::CommonGuiItem", "jive" }
    {
    }

    void runTest() final
    {
        testWidthAndHeight();
        testTopLevelSize();

        testName();
        testTitle();
        testID();
        testDescription();
        testTooltip();

        testVisibility();
        testAlwaysOnTop();
        testBufferedToImage();
        testOpaque();
        testFocusOrder();
        testFocusable();
        testClickingGrabsFocus();
        testFocusOutline();
        testEnablement();
        testOpacity();
        testCursor();
    }

private:
    void testWidthAndHeight()
    {
        beginTest("width and height");

        {
            juce::ValueTree state{
                "Component",
                {
                    { "width", 222 },
                    { "height", 333 },
                },
            };
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(state);
            expect(!jive::BoxModel{ item->state }.hasAutoWidth());
            expect(!jive::BoxModel{ item->state }.hasAutoHeight());
            expectEquals(item->getComponent()->getWidth(), 222);
            expectEquals(item->getComponent()->getHeight(), 333);

            state.setProperty("width", 100.11f, nullptr);
            expectEquals(item->getComponent()->getWidth(), 100);

            state.setProperty("height", 50.55f, nullptr);
            expectEquals(item->getComponent()->getHeight(), 51);
        }
    }

    void testTopLevelSize()
    {
        beginTest("top-level item's size");

        juce::ValueTree state{
            "Component",
            {
                { "width", 200 },
                { "height", 150 },
            },
        };
        jive::Interpreter interpreter;
        auto item = interpreter.interpret(state);
        expect(item->getComponent()->getWidth() == 200);
        expect(item->getComponent()->getHeight() == 150);

        item->getComponent()->setSize(400, 300);
        expectEquals(jive::BoxModel{ item->state }.getWidth(), 400.f);
        expectEquals(jive::BoxModel{ item->state }.getHeight(), 300.f);
    }

    void testName()
    {
        beginTest("name");

        {
            juce::ValueTree state{
                "Component",
                {
                    { "width", 222 },
                    { "height", 333 },
                },
            };
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(state);
            expect(item->getComponent()->getName().isEmpty());

            state.setProperty("name", "Zaphod Beeblebrox", nullptr);
            expectEquals<juce::String>(item->getComponent()->getName(), "Zaphod Beeblebrox");
        }
        {
            juce::ValueTree state{
                "Component",
                {
                    { "width", 222 },
                    { "height", 333 },
                    { "name", "Ford Prefect" },
                },
            };
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(state);
            expectEquals<juce::String>(item->getComponent()->getName(), "Ford Prefect");
        }
    }

    void testTitle()
    {
        beginTest("title");

        {
            juce::ValueTree state{
                "Component",
                {
                    { "width", 222 },
                    { "height", 333 },
                },
            };
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(state);
            expect(item->getComponent()->getTitle().isEmpty());

            state.setProperty("title", "Captain", nullptr);
            expectEquals<juce::String>(item->getComponent()->getTitle(), "Captain");
        }
        {
            juce::ValueTree state{
                "Component",
                {
                    { "width", 222 },
                    { "height", 333 },
                    { "title", "Lord" },
                },
            };
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(state);
            expectEquals<juce::String>(item->getComponent()->getTitle(), "Lord");
        }
    }

    void testID()
    {
        beginTest("ID");

        {
            juce::ValueTree state{
                "Component",
                {
                    { "width", 222 },
                    { "height", 333 },
                },
            };
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(state);
            expect(item->getComponent()->getComponentID().isEmpty());

            state.setProperty("id", "comp123", nullptr);
            expectEquals<juce::String>(item->getComponent()->getComponentID(), "comp123");
        }
        {
            juce::ValueTree state{
                "Component",
                {
                    { "width", 222 },
                    { "height", 333 },
                    { "id", "specified" },
                },
            };
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(state);
            expectEquals<juce::String>(item->getComponent()->getComponentID(), "specified");
        }
    }

    void testDescription()
    {
        beginTest("description");

        {
            juce::ValueTree state{
                "Component",
                {
                    { "width", 222 },
                    { "height", 333 },
                },
            };
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(state);
            expect(item->getComponent()->getDescription().isEmpty());

            state.setProperty("description", "Don't panic!", nullptr);
            expectEquals<juce::String>(item->getComponent()->getDescription(), "Don't panic!");
        }
        {
            juce::ValueTree state{
                "Component",
                {
                    { "width", 222 },
                    { "height", 333 },
                    { "description", "You live and learn. At any rate, you live." },
                },
            };
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(state);
            expectEquals<juce::String>(item->getComponent()->getDescription(),
                                       "You live and learn. At any rate, you live.");
        }
    }

    void testTooltip()
    {
        beginTest("tooltip");

        {
            juce::ValueTree state{
                "Component",
                {
                    { "width", 222 },
                    { "height", 333 },
                },
            };
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(state);
            expect(item->getComponent()->getHelpText().isEmpty());

            state.setProperty("tooltip",
                              "I love deadlines. I love the whooshing noise they make as they go by.",
                              nullptr);
            expectEquals<juce::String>(item->getComponent()->getHelpText(),
                                       "I love deadlines. I love the whooshing noise they make as they go by.");
        }
        {
            juce::ValueTree state{
                "Component",
                {
                    { "width", 222 },
                    { "height", 333 },
                    {
                        "tooltip",
                        "I may not have gone where I intended to go, but I think I have ended up where I needed to be.",
                    },
                },
            };
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(state);
            expectEquals<juce::String>(item->getComponent()->getHelpText(),
                                       "I may not have gone where I intended to go, but I think I have ended up where I needed to be.");
        }
    }

    void testVisibility()
    {
        beginTest("visibility");

        {
            juce::ValueTree state{
                "Component",
                {
                    { "width", 222 },
                    { "height", 333 },
                },
            };
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(state);
            expect(item->getComponent()->isVisible());

            state.setProperty("visible", false, nullptr);
            expect(!item->getComponent()->isVisible());

            ;
            state.appendChild(juce::ValueTree{
                                  "Component",
                                  {
                                      { "width", 222 },
                                      { "height", 333 },
                                      { "visible", true },
                                  },
                              },
                              nullptr);
            item = interpreter.interpret(state);
            expect(item->getChildren()[0]->getComponent()->isVisible());

            juce::ValueTree childTree2{
                "Component",
                {
                    { "width", 222 },
                    { "height", 333 },
                    { "visible", false },
                },
            };
            item->addChild(std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(),
                                                           childTree2,
                                                           item.get()));
            expect(!item->getChildren()[1]->getComponent()->isVisible());
        }
        {
            juce::ValueTree state{
                "Component",
                {
                    { "width", 222 },
                    { "height", 333 },
                    { "visible", false },
                },
            };
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(state);
            expect(!item->getComponent()->isVisible());
        }
    }

    void testAlwaysOnTop()
    {
        beginTest("always-on-top");

        {
            juce::ValueTree state{
                "Component",
                {
                    { "width", 222 },
                    { "height", 333 },
                },
            };
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(state);
            expect(!item->getComponent()->isAlwaysOnTop());

            state.setProperty("always-on-top", true, nullptr);
            expect(item->getComponent()->isAlwaysOnTop());
        }
        {
            juce::ValueTree state{
                "Component",
                {
                    { "width", 222 },
                    { "height", 333 },
                    { "always-on-top", true },
                },
            };
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(state);
            expect(item->getComponent()->isAlwaysOnTop() == static_cast<bool>(state["always-on-top"]));
        }
    }

    void testBufferedToImage()
    {
        beginTest("buffered-to-image");

        {
            juce::ValueTree state{
                "Component",
                {
                    { "width", 222 },
                    { "height", 333 },
                },
            };
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(state);
            expect(item->getComponent()->getCachedComponentImage() == nullptr);

            state.setProperty("buffered-to-image", true, nullptr);
            expect(item->getComponent()->getCachedComponentImage() != nullptr);
        }
        {
            juce::ValueTree state{
                "Component",
                {
                    { "width", 222 },
                    { "height", 333 },
                    { "buffered-to-image", true },
                },
            };
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(state);
            expect(item->getComponent()->getCachedComponentImage() != nullptr);
        }
    }

    void testOpaque()
    {
        beginTest("opaque");

        {
            juce::ValueTree state{
                "Component",
                {
                    { "width", 222 },
                    { "height", 333 },
                },
            };
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(state);
            expect(!item->getComponent()->isOpaque());

            state.setProperty("opaque", true, nullptr);
            expect(item->getComponent()->isOpaque());
        }
        {
            juce::ValueTree state{
                "Component",
                {
                    { "width", 222 },
                    { "height", 333 },
                    { "opaque", true },
                },
            };
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(state);
            expect(item->getComponent()->isOpaque());
        }
    }

    void testFocusOrder()
    {
        beginTest("focus-order");

        {
            juce::ValueTree state{
                "Component",
                {
                    { "width", 222 },
                    { "height", 333 },
                },
            };
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(state);
            expect(item->getComponent()->getExplicitFocusOrder() == 0);

            state.setProperty("focus-order", 12, nullptr);
            expect(item->getComponent()->getExplicitFocusOrder() == 12);
        }
        {
            juce::ValueTree state{
                "Component",
                {
                    { "width", 222 },
                    { "height", 333 },
                    { "focus-order", 4 },
                },
            };
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(state);
            expect(item->getComponent()->getExplicitFocusOrder() == 4);
        }
    }

    void testFocusable()
    {
        beginTest("focusable");

        {
            juce::ValueTree state{
                "Component",
                {
                    { "width", 222 },
                    { "height", 333 },
                },
            };
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(state);
            expect(!item->getComponent()->getWantsKeyboardFocus());

            state.setProperty("focusable", true, nullptr);
            expect(item->getComponent()->getWantsKeyboardFocus());
        }
        {
            juce::ValueTree state{
                "Component",
                {
                    { "width", 222 },
                    { "height", 333 },
                    { "focusable", true },
                },
            };
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(state);
            expect(item->getComponent()->getWantsKeyboardFocus());
        }
    }

    void testClickingGrabsFocus()
    {
        beginTest("clicking-grabs-focus");

        {
            juce::ValueTree state{
                "Component",
                {
                    { "width", 222 },
                    { "height", 333 },
                },
            };
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(state);
            expect(item->getComponent()->getMouseClickGrabsKeyboardFocus());

            state.setProperty("clicking-grabs-focus", false, nullptr);
            expect(!item->getComponent()->getMouseClickGrabsKeyboardFocus());
        }
        {
            juce::ValueTree state{
                "Component",
                {
                    { "width", 222 },
                    { "height", 333 },
                    { "clicking-grabs-focus", false },
                },
            };
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(state);
            expect(!item->getComponent()->getMouseClickGrabsKeyboardFocus());
        }
    }

    void testFocusOutline()
    {
        beginTest("focus-outline");

        {
            juce::ValueTree state{
                "Component",
                {
                    { "width", 222 },
                    { "height", 333 },
                },
            };
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(state);
            expect(!item->getComponent()->hasFocusOutline());

            state.setProperty("focus-outline", true, nullptr);
            expect(item->getComponent()->hasFocusOutline());
        }
        {
            juce::ValueTree state{
                "Component",
                {
                    { "width", 222 },
                    { "height", 333 },
                    { "focus-outline", true },
                },
            };
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(state);
            expect(item->getComponent()->hasFocusOutline());
        }
    }

    void testEnablement()
    {
        beginTest("enablement");

        {
            juce::ValueTree state{
                "Component",
                {
                    { "width", 222 },
                    { "height", 333 },
                },
            };
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(state);
            expect(item->getComponent()->isEnabled());

            state.setProperty("enabled", false, nullptr);
            expect(!item->getComponent()->isEnabled());
        }
        {
            juce::ValueTree state{
                "Component",
                {
                    { "width", 222 },
                    { "height", 333 },
                    { "enabled", false },
                },
            };
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(state);
            expect(!item->getComponent()->isEnabled());
        }
    }

    void testOpacity()
    {
        beginTest("opacity");

        {
            juce::ValueTree state{
                "Component",
                {
                    { "width", 222 },
                    { "height", 333 },
                },
            };
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(state);
            expect(item->getComponent()->getAlpha() == 1.f);

            state.setProperty("opacity", 0.42f, nullptr);
            expectWithinAbsoluteError(item->getComponent()->getAlpha(), 0.42f, 1.f / 256.f);
        }
        {
            juce::ValueTree state{
                "Component",
                {
                    { "width", 222 },
                    { "height", 333 },
                    { "opacity", 0.123f },
                },
            };
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(state);
            expectWithinAbsoluteError(item->getComponent()->getAlpha(), 0.123f, 1.f / 256.f);
        }
    }

    void testCursor()
    {
        beginTest("cursor");

        {
            juce::ValueTree state{
                "Component",
                {
                    { "width", 222 },
                    { "height", 333 },
                },
            };
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(state);
            expect(item->getComponent()->getMouseCursor() == juce::MouseCursor::NormalCursor);

            state.setProperty("cursor", "left-right", nullptr);
            expect(item->getComponent()->getMouseCursor() == juce::MouseCursor::LeftRightResizeCursor);
        }
        {
            juce::ValueTree state{
                "Component",
                {
                    { "width", 222 },
                    { "height", 333 },
                    { "cursor", "wait" },
                },
            };
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(state);
            expect(item->getComponent()->getMouseCursor() == juce::MouseCursor::WaitCursor);
        }
    }
};

static CommonGuiItemUnitTest commonGuiItemUnitTest;
#endif
