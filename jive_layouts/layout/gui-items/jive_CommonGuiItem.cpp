#include "jive_CommonGuiItem.h"

namespace jive
{
    CommonGuiItem::CommonGuiItem(std::unique_ptr<GuiItem> itemToDecorate)
        : GuiItemDecorator{ std::move(itemToDecorate) }
        , boxModel{ state }
#if !JIVE_GUI_ITEMS_HAVE_STYLE_SHEETS
        , interactionState{ *getComponent(), state }
#endif
        , name{ state, "name" }
        , title{ state, "title" }
        , id{ state, "id" }
        , description{ state, "description" }
        , tooltip{ state, "tooltip" }
        , enabled{ state, "enabled" }
        , accessible{ state, "accessible" }
        , visibility{ state, "visibility" }
        , alwaysOnTop{ state, "always-on-top" }
        , bufferedToImage{ state, "buffered-to-image" }
        , opaque{ state, "opaque" }
        , focusable{ state, "focusable" }
        , clickingGrabsFocus{ state, "clicking-grabs-focus" }
        , focusOutline{ state, "focus-outline" }
        , focusOrder{ state, "focus-order" }
        , opacity{ state, "opacity" }
        , cursor{ state, "cursor" }
        , display{ state, "display" }
        , width{ state, "width" }
        , height{ state, "height" }
    {
        if (!enabled.exists())
            enabled = true;
        if (!accessible.exists())
            accessible = true;
        if (!visibility.exists())
            visibility = true;
        if (!clickingGrabsFocus.exists())
            clickingGrabsFocus = true;
        if (!focusOrder.exists())
            focusOrder = state.getParent().indexOf(state) + 1;
        if (!opacity.exists())
            opacity = 1.0f;
        if (!cursor.exists())
            cursor = juce::MouseCursor::NormalCursor;
        if (!display.exists())
            display = Display::flex;

        getComponent()->addComponentListener(this);

        name.onValueChange = [this]() {
            getComponent()->setName(name);
        };
        getComponent()->setName(name);

        title.onValueChange = [this]() {
            getComponent()->setTitle(title);
        };
        getComponent()->setTitle(title);

        id.onValueChange = [this]() {
            getComponent()->setComponentID(id.get().toString());
        };
        getComponent()->setComponentID(id.get().toString());

        description.onValueChange = [this]() {
            getComponent()->setDescription(description);
        };
        getComponent()->setDescription(description);

        tooltip.onValueChange = [this]() {
            getComponent()->setHelpText(tooltip);
        };
        getComponent()->setHelpText(tooltip);

        enabled.onValueChange = [this]() {
            getComponent()->setEnabled(enabled);
        };
        getComponent()->setEnabled(enabled);

        accessible.onValueChange = [this]() {
            getComponent()->setAccessible(accessible);
        };
        getComponent()->setAccessible(accessible);

        visibility.onValueChange = [this]() {
            getComponent()->setVisible(visibility);
        };
        getComponent()->setVisible(visibility);

        alwaysOnTop.onValueChange = [this]() {
            getComponent()->setAlwaysOnTop(alwaysOnTop);
        };
        getComponent()->setAlwaysOnTop(alwaysOnTop);

        bufferedToImage.onValueChange = [this]() {
            getComponent()->setBufferedToImage(bufferedToImage);
        };
        getComponent()->setBufferedToImage(bufferedToImage);

        opaque.onValueChange = [this]() {
            getComponent()->setOpaque(opaque);
        };
        getComponent()->setOpaque(opaque);

        focusable.onValueChange = [this]() {
            getComponent()->setWantsKeyboardFocus(focusable);
        };
        getComponent()->setWantsKeyboardFocus(focusable);

        focusOutline.onValueChange = [this]() {
            getComponent()->setHasFocusOutline(focusOutline);
        };
        getComponent()->setHasFocusOutline(focusOutline);

        clickingGrabsFocus.onValueChange = [this]() {
            getComponent()->setMouseClickGrabsKeyboardFocus(clickingGrabsFocus);
        };
        getComponent()->setMouseClickGrabsKeyboardFocus(clickingGrabsFocus);

        focusOrder.onValueChange = [this]() {
            getComponent()->setExplicitFocusOrder(focusOrder);
        };
        getComponent()->setExplicitFocusOrder(focusOrder);

        const auto updateOpacity = [this] {
            if (auto* transition = opacity.getTransition())
                getComponent()->setAlpha(transition->calculateCurrent<float>());
            else
                getComponent()->setAlpha(opacity);
        };
        opacity.onValueChange = updateOpacity;
        opacity.onTransitionProgressed = updateOpacity;
        getComponent()->setAlpha(opacity);

        cursor.onValueChange = [this]() {
            getComponent()->setMouseCursor(juce::MouseCursor{ cursor });
        };
        getComponent()->setMouseCursor(juce::MouseCursor{ cursor });

        if (isTopLevel())
        {
            // Top-level items must have an explicit size!
            jassert(!boxModel.hasAutoWidth());
            jassert(!boxModel.hasAutoHeight());
        }

        getComponent()->setSize(juce::roundToInt(boxModel.getOuterBounds().getWidth()),
                                juce::roundToInt(boxModel.getOuterBounds().getHeight()));

        boxModel.addListener(*this);
    }

    CommonGuiItem::~CommonGuiItem()
    {
        getComponent()->removeComponentListener(this);
    }

    void CommonGuiItem::componentMovedOrResized(juce::Component& componentThatWasMovedOrResized,
                                                bool /*wasMoved*/,
                                                bool wasResized)
    {
        if (&componentThatWasMovedOrResized != getComponent().get())
            return;

        if (!wasResized)
            return;

        const auto componentBounds = getComponent()->getBounds().toFloat();

        if (!width.isTransitioning())
            boxModel.setWidth(componentBounds.getWidth());
        if (!height.isTransitioning())
            boxModel.setHeight(componentBounds.getHeight());
    }

    void CommonGuiItem::componentVisibilityChanged(juce::Component& componentThatChangedVisiblity)
    {
        if (&componentThatChangedVisiblity != getComponent().get())
            return;

        visibility = getComponent()->isVisible();
    }

    void CommonGuiItem::componentNameChanged(juce::Component& componentThatChangedName)
    {
        if (&componentThatChangedName != getComponent().get())
            return;

        name = getComponent()->getName();
    }

    void CommonGuiItem::componentEnablementChanged(juce::Component& componentThatChangedEnablement)
    {
        if (&componentThatChangedEnablement != getComponent().get())
            return;

        enabled = getComponent()->isEnabled();
    }

    [[nodiscard]] static auto hasWidgetRole(const juce::Component& component)
    {
        if (auto* handler = const_cast<juce::Component*>(&component)->getAccessibilityHandler())
        {
            switch (handler->getRole())
            {
            case juce::AccessibilityRole::button:
            case juce::AccessibilityRole::toggleButton:
            case juce::AccessibilityRole::radioButton:
            case juce::AccessibilityRole::comboBox:
            case juce::AccessibilityRole::image:
            case juce::AccessibilityRole::slider:
            case juce::AccessibilityRole::label:
            case juce::AccessibilityRole::staticText:
            case juce::AccessibilityRole::editableText:
            case juce::AccessibilityRole::hyperlink:
            case juce::AccessibilityRole::progressBar:
            case juce::AccessibilityRole::scrollBar:
            case juce::AccessibilityRole::tooltip:
                return true;

            case juce::AccessibilityRole::menuItem:
            case juce::AccessibilityRole::menuBar:
            case juce::AccessibilityRole::popupMenu:
            case juce::AccessibilityRole::table:
            case juce::AccessibilityRole::tableHeader:
            case juce::AccessibilityRole::column:
            case juce::AccessibilityRole::row:
            case juce::AccessibilityRole::cell:
            case juce::AccessibilityRole::list:
            case juce::AccessibilityRole::listItem:
            case juce::AccessibilityRole::tree:
            case juce::AccessibilityRole::treeItem:
            case juce::AccessibilityRole::group:
            case juce::AccessibilityRole::dialogWindow:
            case juce::AccessibilityRole::window:
            case juce::AccessibilityRole::splashScreen:
            case juce::AccessibilityRole::ignored:
            case juce::AccessibilityRole::unspecified:
            default:
                return false;
            }
        }

        return false;
    }

    void CommonGuiItem::componentParentHierarchyChanged(juce::Component& componentThatsParentChanged)
    {
        if (&componentThatsParentChanged != getComponent().get())
            return;

        if (auto* parentComponent = getComponent()->getParentComponent())
        {
            if (hasWidgetRole(*parentComponent))
                getComponent()->setAccessible(false);
            else
                getComponent()->setAccessible(accessible);
        }
    }

    void CommonGuiItem::boxModelChanged(BoxModel& boxModelThatChanged)
    {
        jassertquiet(&boxModelThatChanged == &boxModel);

        getComponent()->removeComponentListener(this);
        getComponent()->setSize(juce::roundToInt(boxModel.getWidth()),
                                juce::roundToInt(boxModel.getHeight()));
        getComponent()->addComponentListener(this);
        getTopLevelDecorator().callLayoutChildrenWithRecursionLock();
    }

    void CommonGuiItem::childrenChanged()
    {
        getTopLevelDecorator().callLayoutChildrenWithRecursionLock();
    }
} // namespace jive

#if JIVE_UNIT_TESTS
    #include <jive_layouts/layout/jive_Interpreter.h>

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
            const auto& boxModel = jive::boxModel(*item);
            expect(!boxModel.hasAutoWidth());
            expect(!boxModel.hasAutoHeight());
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
        const auto& boxModel = jive::boxModel(*item);
        expectEquals(boxModel.getWidth(), 400.f);
        expectEquals(boxModel.getHeight(), 300.f);
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

            state.setProperty("visibility", false, nullptr);
            expect(!item->getComponent()->isVisible());

            ;
            state.appendChild(juce::ValueTree{
                                  "Component",
                                  {
                                      { "width", 222 },
                                      { "height", 333 },
                                      { "visibility", true },
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
                    { "visibility", false },
                },
            };
            item->insertChild(std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(),
                                                              childTree2,
                                                              item.get()),
                              -1);
            expect(!item->getChildren()[1]->getComponent()->isVisible());
        }
        {
            juce::ValueTree state{
                "Component",
                {
                    { "width", 222 },
                    { "height", 333 },
                    { "visibility", false },
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

            state.setProperty("transition", "opacity 10s", nullptr);
            state.setProperty("opacity", 0.92f, nullptr);
            expectWithinAbsoluteError(item->getComponent()->getAlpha(), 0.42f, 1.f / 256.f);
            jive::FakeTime::incrementTime(juce::RelativeTime::seconds(4.0));
            expectWithinAbsoluteError(item->getComponent()->getAlpha(), 0.62f, 1.f / 256.f);
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
