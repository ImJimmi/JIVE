#include <jive_layouts/jive_layouts.h>

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    GuiItem::GuiItem(std::shared_ptr<juce::Component> comp, GuiItem* parentItem, juce::ValueTree stateSource)
        : state{ stateSource }
        , boxModel{ stateSource }
        , component{ comp }
        , parent{ parentItem }
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

        if (!isTopLevel())
        {
            componentWidth.onValueChange = [this]() {
                component->setSize(juce::roundToInt(boxModel.getBorderBounds().getWidth()),
                                   component->getHeight());
            };
            componentHeight.onValueChange = [this]() {
                component->setSize(component->getWidth(),
                                   juce::roundToInt(boxModel.getBorderBounds().getHeight()));
            };
        }

        component->setSize(juce::roundToInt(boxModel.getBorderBounds().getWidth()),
                           component->getHeight());
        component->setSize(component->getWidth(),
                           juce::roundToInt(boxModel.getBorderBounds().getHeight()));

        component->addComponentListener(this);
        state.addListener(this);
    }

    GuiItem::GuiItem(std::unique_ptr<juce::Component> comp, juce::ValueTree sourceState, GuiItem* parentItem)
        : GuiItem{ std::shared_ptr<juce::Component>{ std::move(comp) }, parentItem, sourceState }
    {
    }

    GuiItem::GuiItem(const GuiItem& other)
        : GuiItem{ other.component, other.parent, other.state }
    {
    }

    GuiItem::~GuiItem()
    {
        component->removeComponentListener(this);
    }

    //==================================================================================================================
    void GuiItem::updateLayout()
    {
    }

    void GuiItem::updateSize()
    {
    }

    void GuiItem::updatePosition()
    {
    }

    void GuiItem::informContentChanged()
    {
        contentChanged();
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

    bool GuiItem::hasAutoWidth() const
    {
        return width.isAuto();
    }

    bool GuiItem::hasAutoHeight() const
    {
        return height.isAuto();
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
    void GuiItem::componentMovedOrResized(juce::Component& componentThatWasMovedOrResized,
                                          bool /*wasMoved*/,
                                          bool wasResized)
    {
        jassertquiet(&componentThatWasMovedOrResized == component.get());

        if (!wasResized)
            return;
        if (!isTopLevel())
            return;

        boxModel.setWidth(static_cast<float>(component->getWidth()));
        boxModel.setHeight(static_cast<float>(component->getHeight()));
    }

    void GuiItem::componentVisibilityChanged(juce::Component& componentThatChangedVisiblity)
    {
        jassertquiet(&componentThatChangedVisiblity == component.get());
        visible = component->isVisible();
    }

    void GuiItem::componentNameChanged(juce::Component& componentThatChangedName)
    {
        jassertquiet(&componentThatChangedName == component.get());
        name = component->getName();
    }

    void GuiItem::componentEnablementChanged(juce::Component& componentThatChangedEnablement)
    {
        jassertquiet(&componentThatChangedEnablement == component.get());
        enabled = component->isEnabled();
    }

    //==================================================================================================================
    void GuiItem::contentChanged()
    {
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
    std::unique_ptr<jive::GuiItem> createGuiItem(juce::ValueTree tree) const
    {
        jive::Interpreter interpreter;
        return interpreter.interpret(tree);
    }

    void testChildren()
    {
        beginTest("children");

        auto item = createGuiItem(juce::ValueTree{ "Component" });
        expect(item->getNumChildren() == 0);
        expect(item->getComponent()->getNumChildComponents() == item->getNumChildren());

        item->addChild(createGuiItem(juce::ValueTree{ "Component" }));
        expect(item->getNumChildren() == 1);
        expect(item->getComponent()->getNumChildComponents() == item->getNumChildren());

        item->addChild(createGuiItem(juce::ValueTree{ "Component" }));
        item->addChild(createGuiItem(juce::ValueTree{ "Component" }));
        expect(item->getNumChildren() == 3);
        expect(item->getComponent()->getNumChildComponents() == item->getNumChildren());
    }

    void testWidthAndHeight()
    {
        beginTest("width and height");

        {
            juce::ValueTree tree{ "Component" };
            auto item = createGuiItem(tree);
            expect(item->hasAutoWidth());
            expect(item->hasAutoHeight());
            expectEquals(item->getComponent()->getWidth(), 0);
            expectEquals(item->getComponent()->getHeight(), 0);

            tree.setProperty("width", 100.11f, nullptr);
            tree.setProperty("height", 50.55f, nullptr);
            expect(!item->hasAutoWidth());
            expect(!item->hasAutoHeight());
            expectEquals(item->getComponent()->getWidth(), 100);
            expectEquals(item->getComponent()->getHeight(), 51);
        }
        {
            juce::ValueTree tree{
                "Component",
                {
                    { "width", 100.0f },
                    { "height", 389.0f },
                },
            };
            auto item = createGuiItem(tree);
            expect(!item->hasAutoWidth());
            expect(!item->hasAutoHeight());
            expectEquals(item->getComponent()->getWidth(), 100);
            expectEquals(item->getComponent()->getHeight(), 389);

            item->getComponent()->setSize(312, 1203);
            expectEquals(tree["width"], juce::var{ 100.0f });
            expectEquals(tree["height"], juce::var{ 389.0f });
        }
    }

    void testTopLevelSize()
    {
        beginTest("top-level item's size");

        juce::ValueTree tree{
            "Component",
            {
                { "width", 200 },
                { "height", 150 },
            },
        };
        auto item = createGuiItem(tree);
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
            juce::ValueTree tree{ "Component" };
            auto item = createGuiItem(tree);
            expect(item->getComponent()->getName().isEmpty());

            tree.setProperty("name", "Zaphod Beeblebrox", nullptr);
            expectEquals<juce::String>(item->getComponent()->getName(), "Zaphod Beeblebrox");
        }
        {
            juce::ValueTree tree{
                "Component",
                {
                    { "name", "Ford Prefect" },
                },
            };
            auto item = createGuiItem(tree);
            expectEquals<juce::String>(item->getComponent()->getName(), "Ford Prefect");
        }
    }

    void testTitle()
    {
        beginTest("title");

        {
            juce::ValueTree tree{ "Component" };
            auto item = createGuiItem(tree);
            expect(item->getComponent()->getTitle().isEmpty());

            tree.setProperty("title", "Captain", nullptr);
            expectEquals<juce::String>(item->getComponent()->getTitle(), "Captain");
        }
        {
            juce::ValueTree tree{
                "Component",
                {
                    { "title", "Lord" },
                },
            };
            auto item = createGuiItem(tree);
            expectEquals<juce::String>(item->getComponent()->getTitle(), "Lord");
        }
    }

    void testID()
    {
        beginTest("ID");

        {
            juce::ValueTree tree{ "Component" };
            auto item = createGuiItem(tree);

            expect(item->getComponent()->getComponentID().isEmpty());

            tree.setProperty("id", "comp123", nullptr);

            expectEquals<juce::String>(item->getComponent()->getComponentID(), "comp123");
        }
        {
            juce::ValueTree tree{
                "Component",
                {
                    { "id", "specified" },
                },
            };
            auto item = createGuiItem(tree);

            expectEquals<juce::String>(item->getComponent()->getComponentID(), "specified");
        }
    }

    void testDescription()
    {
        beginTest("description");

        {
            juce::ValueTree tree{ "Component" };
            auto item = createGuiItem(tree);

            expect(item->getComponent()->getDescription().isEmpty());

            tree.setProperty("description", "Don't panic!", nullptr);

            expectEquals<juce::String>(item->getComponent()->getDescription(), "Don't panic!");
        }
        {
            juce::ValueTree tree{
                "Component",
                {
                    { "description", "You live and learn. At any rate, you live." },
                },
            };
            auto item = createGuiItem(tree);

            expectEquals<juce::String>(item->getComponent()->getDescription(),
                                       "You live and learn. At any rate, you live.");
        }
    }

    void testTooltip()
    {
        beginTest("tooltip");

        {
            juce::ValueTree tree{ "Component" };
            auto item = createGuiItem(tree);

            expect(item->getComponent()->getHelpText().isEmpty());

            tree.setProperty("tooltip",
                             "I love deadlines. I love the whooshing noise they make as they go by.",
                             nullptr);

            expectEquals<juce::String>(item->getComponent()->getHelpText(),
                                       "I love deadlines. I love the whooshing noise they make as they go by.");
        }
        {
            juce::ValueTree tree{
                "Component",
                {
                    { "tooltip",
                      "I may not have gone where I intended to go, but I think I have ended up where I needed to be." },
                },
            };
            auto item = createGuiItem(tree);

            expectEquals<juce::String>(item->getComponent()->getHelpText(),
                                       "I may not have gone where I intended to go, but I think I have ended up where I needed to be.");
        }
    }

    void testVisibility()
    {
        beginTest("visibility");

        {
            juce::ValueTree tree{ "Component" };
            auto item = createGuiItem(tree);

            expect(item->getComponent()->isVisible());

            tree.setProperty("visible", false, nullptr);
            expect(!item->getComponent()->isVisible());

            juce::ValueTree childTree1{
                "Component",
                {
                    { "visible", true },
                },
            };
            item->addChild(createGuiItem(childTree1));
            expect(item->getChild(0).getComponent()->isVisible());

            juce::ValueTree childTree2{
                "Component",
                {
                    { "visible", false },
                },
            };
            item->addChild(createGuiItem(childTree2));
            expect(!item->getChild(1).getComponent()->isVisible());
        }
        {
            juce::ValueTree tree{
                "Component",
                {
                    { "visible", false },
                },
            };
            auto item = createGuiItem(tree);

            expect(!item->getComponent()->isVisible());
        }
    }

    void testAlwaysOnTop()
    {
        beginTest("always-on-top");

        {
            juce::ValueTree tree{ "Component" };
            auto item = createGuiItem(tree);
            expect(!item->getComponent()->isAlwaysOnTop());

            tree.setProperty("always-on-top", true, nullptr);
            expect(item->getComponent()->isAlwaysOnTop());
        }
        {
            juce::ValueTree tree{
                "Component",
                {
                    { "always-on-top", true },
                },
            };
            auto item = createGuiItem(tree);
            expect(item->getComponent()->isAlwaysOnTop() == static_cast<bool>(tree["always-on-top"]));
        }
    }

    void testBufferedToImage()
    {
        beginTest("buffered-to-image");

        {
            juce::ValueTree tree{ "Component" };
            auto item = createGuiItem(tree);
            expect(item->getComponent()->getCachedComponentImage() == nullptr);

            tree.setProperty("buffered-to-image", true, nullptr);
            expect(item->getComponent()->getCachedComponentImage() != nullptr);
        }
        {
            juce::ValueTree tree{
                "Component",
                {
                    { "buffered-to-image", true },
                },
            };
            auto item = createGuiItem(tree);
            expect(item->getComponent()->getCachedComponentImage() != nullptr);
        }
    }

    void testOpaque()
    {
        beginTest("opaque");

        {
            juce::ValueTree tree{ "Component" };
            auto item = createGuiItem(tree);
            expect(!item->getComponent()->isOpaque());

            tree.setProperty("opaque", true, nullptr);
            expect(item->getComponent()->isOpaque());
        }
        {
            juce::ValueTree tree{
                "Component",
                {
                    { "opaque", true },
                },
            };
            auto item = createGuiItem(tree);
            expect(item->getComponent()->isOpaque());
        }
    }

    void testFocusOrder()
    {
        beginTest("focus-order");

        {
            juce::ValueTree tree{ "Component" };
            auto item = createGuiItem(tree);
            expect(item->getComponent()->getExplicitFocusOrder() == 0);

            tree.setProperty("focus-order", 12, nullptr);
            expect(item->getComponent()->getExplicitFocusOrder() == 12);
        }
        {
            juce::ValueTree tree{
                "Component",
                {
                    { "focus-order", 4 },
                },
            };
            auto item = createGuiItem(tree);
            expect(item->getComponent()->getExplicitFocusOrder() == 4);
        }
    }

    void testFocusable()
    {
        beginTest("focusable");

        {
            juce::ValueTree tree{ "Component" };
            auto item = createGuiItem(tree);
            expect(!item->getComponent()->getWantsKeyboardFocus());

            tree.setProperty("focusable", true, nullptr);
            expect(item->getComponent()->getWantsKeyboardFocus());
        }
        {
            juce::ValueTree tree{
                "Component",
                {
                    { "focusable", true },
                },
            };
            auto item = createGuiItem(tree);
            expect(item->getComponent()->getWantsKeyboardFocus());
        }
    }

    void testClickingGrabsFocus()
    {
        beginTest("clicking-grabs-focus");

        {
            juce::ValueTree tree{ "Component" };
            auto item = createGuiItem(tree);
            expect(item->getComponent()->getMouseClickGrabsKeyboardFocus());

            tree.setProperty("clicking-grabs-focus", false, nullptr);
            expect(!item->getComponent()->getMouseClickGrabsKeyboardFocus());
        }
        {
            juce::ValueTree tree{
                "Component",
                {
                    { "clicking-grabs-focus", false },
                },
            };
            auto item = createGuiItem(tree);
            expect(!item->getComponent()->getMouseClickGrabsKeyboardFocus());
        }
    }

    void testFocusOutline()
    {
        beginTest("focus-outline");

        {
            juce::ValueTree tree{ "Component" };
            auto item = createGuiItem(tree);
            expect(!item->getComponent()->hasFocusOutline());

            tree.setProperty("focus-outline", true, nullptr);
            expect(item->getComponent()->hasFocusOutline());
        }
        {
            juce::ValueTree tree{
                "Component",
                {
                    { "focus-outline", true },
                },
            };
            auto item = createGuiItem(tree);
            expect(item->getComponent()->hasFocusOutline());
        }
    }

    void testEnablement()
    {
        beginTest("enablement");

        {
            juce::ValueTree tree{ "Component" };
            auto item = createGuiItem(tree);
            expect(item->getComponent()->isEnabled());

            tree.setProperty("enabled", false, nullptr);
            expect(!item->getComponent()->isEnabled());
        }
        {
            juce::ValueTree tree{
                "Component",
                {
                    { "enabled", false },
                },
            };
            auto item = createGuiItem(tree);
            expect(!item->getComponent()->isEnabled());
        }
    }

    void testOpacity()
    {
        beginTest("opacity");

        {
            juce::ValueTree tree{ "Component" };
            auto item = createGuiItem(tree);
            expect(item->getComponent()->getAlpha() == 1.f);

            tree.setProperty("opacity", 0.42f, nullptr);
            expectWithinAbsoluteError(item->getComponent()->getAlpha(), 0.42f, 1.f / 256.f);
        }
        {
            juce::ValueTree tree{
                "Component",
                {
                    { "opacity", 0.123f },
                },
            };
            auto item = createGuiItem(tree);
            expectWithinAbsoluteError(item->getComponent()->getAlpha(), 0.123f, 1.f / 256.f);
        }
    }

    void testCursor()
    {
        beginTest("cursor");

        {
            juce::ValueTree tree{ "Component" };
            auto item = createGuiItem(tree);
            expect(item->getComponent()->getMouseCursor() == juce::MouseCursor::NormalCursor);

            tree.setProperty("cursor", "left-right", nullptr);
            expect(item->getComponent()->getMouseCursor() == juce::MouseCursor::LeftRightResizeCursor);
        }
        {
            juce::ValueTree tree{
                "Component",
                {
                    { "cursor", "wait" },
                },
            };
            auto item = createGuiItem(tree);
            expect(item->getComponent()->getMouseCursor() == juce::MouseCursor::WaitCursor);
        }
    }
};

static GuiItemUnitTest guiItemUnitTest;
#endif
