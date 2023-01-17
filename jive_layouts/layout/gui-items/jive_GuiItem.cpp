#include <jive_layouts/jive_layouts.h>

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    GuiItem::GuiItem(std::shared_ptr<juce::Component> comp,
                     GuiItem* parentItem,
#if JIVE_GUI_ITEMS_HAVE_STYLE_SHEETS
                     std::unique_ptr<StyleSheet> sheet,
#endif
                     juce::ValueTree stateSource)
        : state{ stateSource }
        , boxModel{ stateSource }
        , component{ comp }
        , parent{ parentItem }
#if JIVE_GUI_ITEMS_HAVE_STYLE_SHEETS
        , styleSheet{ std::move(sheet) }
#endif
        , name{ stateSource, "name" }
        , title{ stateSource, "title" }
        , id{ stateSource, "id" }
        , description{ stateSource, "description" }
        , tooltip{ stateSource, "tooltip" }
        , enabled{ stateSource, "enabled", true }
        , visible{ stateSource, "visible", true }
        , alwaysOnTop{ stateSource, "always-on-top" }
        , bufferedToImage{ stateSource, "buffered-to-image" }
        , opaque{ stateSource, "opaque" }
        , focusable{ stateSource, "focusable" }
        , clickingGrabsFocus{ stateSource, "clicking-grabs-focus", true }
        , focusOutline{ stateSource, "focus-outline" }
        , focusOrder{ stateSource, "focus-order" }
        , opacity{ stateSource, "opacity", 1.f }
        , cursor{ stateSource, "cursor", juce::MouseCursor::NormalCursor }
        , display{ stateSource, "display", Display::flex }
        , width{ stateSource, "width", "auto" }
        , height{ stateSource, "height", "auto" }
        , componentWidth{ stateSource, "component-width" }
        , componentHeight{ stateSource, "component-height" }
    {
        jassert(component != nullptr);

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

        componentWidth.onValueChange = [this]() {
            component->setSize(juce::roundToInt(boxModel.getBounds().getWidth()),
                               juce::roundToInt(boxModel.getBounds().getHeight()));
        };
        component->setSize(juce::roundToInt(boxModel.getBounds().getWidth()),
                           component->getHeight());

        componentHeight.onValueChange = [this]() {
            component->setSize(juce::roundToInt(boxModel.getBounds().getWidth()),
                               juce::roundToInt(boxModel.getBounds().getHeight()));
        };
        component->setSize(component->getWidth(),
                           juce::roundToInt(boxModel.getBounds().getHeight()));

        component->addComponentListener(this);
        state.addListener(this);
        boxModel.addListener(*this);
    }

    GuiItem::GuiItem(std::unique_ptr<juce::Component> comp,
                     juce::ValueTree sourceState,
#if JIVE_GUI_ITEMS_HAVE_STYLE_SHEETS
                     std::unique_ptr<StyleSheet> sheet,
#endif
                     GuiItem* parentItem)
        : GuiItem
    {
        std::shared_ptr<juce::Component>{ std::move(comp) },
            parentItem,
#if JIVE_GUI_ITEMS_HAVE_STYLE_SHEETS
            std::move(sheet),
#endif
            sourceState,
    }
    {
    }

    GuiItem::GuiItem(const GuiItem& other)
        : GuiItem
    {
        other.component,
            other.parent,
#if JIVE_GUI_ITEMS_HAVE_STYLE_SHEETS
            nullptr,
#endif
            other.state,
    }
    {
    }

    GuiItem::~GuiItem()
    {
        component->removeComponentListener(this);
    }

    //==================================================================================================================
    const std::shared_ptr<const juce::Component> GuiItem::getComponent() const
    {
        return component;
    }

    const std::shared_ptr<juce::Component> GuiItem::getComponent()
    {
        return component;
    }

    void GuiItem::addChild(std::unique_ptr<GuiItem> child)
    {
        auto* newlyAddedChild = children.add(std::move(child));
        component->addChildComponent(*newlyAddedChild->getComponent());
        newlyAddedChild->boxModel.addListener(*this);
    }

    int GuiItem::getNumChildren() const
    {
        return children.size();
    }

    GuiItem& GuiItem::getChild(int index) const
    {
        jassert(index < children.size());
        return *children[index];
    }

    const GuiItem* GuiItem::getParent() const
    {
        return parent;
    }

    GuiItem* GuiItem::getParent()
    {
        return parent;
    }

    bool GuiItem::isTopLevel() const
    {
        return getParent() == nullptr;
    }

    bool GuiItem::isContainer() const
    {
        return true;
    }

    bool GuiItem::isContent() const
    {
        return false;
    }

    //==================================================================================================================
    GuiItem::Iterator GuiItem::begin()
    {
        return Iterator{ *this, false };
    }

    const GuiItem::Iterator GuiItem::begin() const
    {
        return Iterator{ *this, false };
    }

    GuiItem::Iterator GuiItem::end()
    {
        return Iterator{ *this, true };
    }

    const GuiItem::Iterator GuiItem::end() const
    {
        return Iterator{ *this, true };
    }

    //==================================================================================================================
    GuiItem* findChildWithState(juce::ValueTree& state, GuiItem& parentItem)
    {
        for (auto& child : parentItem)
        {
            if (child.state == state)
                return &child;
        }

        return nullptr;
    }

    void GuiItem::valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property)
    {
        if (tree == state)
        {
            if (property == juce::Identifier{ "padding" }
                || property == juce::Identifier{ "border-width" })
            {
                layOutChildren();
            }
        }

        if (auto* child = findChildWithState(tree, *this))
        {
            if ((property == componentWidth.id && !child->boxModel.hasAutoWidth())
                && (property == componentHeight.id && !child->boxModel.hasAutoHeight()))
            {
                layOutChildren();
            }
        }
    }

    void GuiItem::componentMovedOrResized(juce::Component& componentThatWasMovedOrResized,
                                          bool /*wasMoved*/,
                                          bool wasResized)
    {
        if (&componentThatWasMovedOrResized != component.get())
            return;

        if (!wasResized)
            return;

        const auto componentBounds = component->getBounds().toFloat();
        boxModel.setWidth(componentBounds.getWidth());
        boxModel.setHeight(componentBounds.getHeight());
        layOutChildren();
    }

    void GuiItem::componentVisibilityChanged(juce::Component& componentThatChangedVisiblity)
    {
        if (&componentThatChangedVisiblity != component.get())
            return;

        visible = component->isVisible();
    }

    void GuiItem::componentNameChanged(juce::Component& componentThatChangedName)
    {
        if (&componentThatChangedName != component.get())
            return;

        name = component->getName();
    }

    void GuiItem::componentEnablementChanged(juce::Component& componentThatChangedEnablement)
    {
        if (&componentThatChangedEnablement != component.get())
            return;

        enabled = component->isEnabled();
    }

    void GuiItem::componentChildrenChanged(juce::Component& componentThatsChildrenChanged)
    {
        if (&componentThatsChildrenChanged != component.get())
            return;

        layOutChildren();
    }

    void GuiItem::boxModelChanged(BoxModel& boxModelThatChanged)
    {
        if (&boxModelThatChanged == &boxModel)
        {
            component->setSize(juce::roundToInt(boxModel.getWidth()),
                               juce::roundToInt(boxModel.getHeight()));
        }

        layOutChildren();
    }

    //==================================================================================================================
    GuiItem::Iterator::Iterator(const GuiItem& guiItem, bool isEnd)
        : item{ isEnd ? guiItem.children.end() : guiItem.children.begin() }
    {
    }

    GuiItem::Iterator& GuiItem::Iterator::operator++()
    {
        item++;
        return *this;
    }

    bool GuiItem::Iterator::operator==(const Iterator& other) const
    {
        return item == other.item;
    }

    bool GuiItem::Iterator::operator!=(const Iterator& other) const
    {
        return item != other.item;
    }

    GuiItem& GuiItem::Iterator::operator*()
    {
        return *(*item);
    }

    const GuiItem& GuiItem::Iterator::operator*() const
    {
        return *(*item);
    }
} // namespace jive

//======================================================================================================================
#if JIVE_UNIT_TESTS
class GuiItemUnitTest : public juce::UnitTest
{
public:
    GuiItemUnitTest()
        : juce::UnitTest{ "jive::GuiItem", "jive" }
    {
    }

    void runTest() final
    {
        testChildren();

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
    void testChildren()
    {
        beginTest("children");

        jive::Interpreter interpreter;
        auto item = interpreter.interpret(juce::ValueTree{
            "Component",
            {
                { "width", 100 },
                { "height", 100 },
            },
        });
        expectEquals(item->getNumChildren(), 0);
        expectEquals(item->getComponent()->getNumChildComponents(), item->getNumChildren());

        item->addChild(std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(),
                                                       juce::ValueTree{ "Component" },
                                                       item.get()));
        expectEquals(item->getNumChildren(), 1);
        expectEquals(item->getComponent()->getNumChildComponents(), item->getNumChildren());

        item->addChild(std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(),
                                                       juce::ValueTree{ "Component" },
                                                       item.get()));
        item->addChild(std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(),
                                                       juce::ValueTree{ "Component" },
                                                       item.get()));
        expectEquals(item->getNumChildren(), 3);
        expectEquals(item->getComponent()->getNumChildComponents(), item->getNumChildren());
    }

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
            expect(!item->boxModel.hasAutoWidth());
            expect(!item->boxModel.hasAutoHeight());
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
        expectEquals(item->boxModel.getWidth(), 400.f);
        expectEquals(item->boxModel.getHeight(), 300.f);
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

            juce::ValueTree childTree1{
                "Component",
                {
                    { "width", 222 },
                    { "height", 333 },
                    { "visible", true },
                },
            };
            item->addChild(std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(),
                                                           childTree1,
                                                           item.get()));
            expect(item->getChild(0).getComponent()->isVisible());

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
            expect(!item->getChild(1).getComponent()->isVisible());
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

static GuiItemUnitTest guiItemUnitTest;
#endif
