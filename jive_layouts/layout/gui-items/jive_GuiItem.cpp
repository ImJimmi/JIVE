#include <jive_layouts/jive_layouts.h>

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    GuiItem::GuiItem(std::shared_ptr<juce::Component> comp,
                     std::shared_ptr<juce::Viewport> vp,
                     juce::ValueTree valueTree,
                     GuiItem* parentItem)
        : tree{ valueTree }
        , component{ std::move(comp) }
        , viewport{ std::move(vp) }
        , parent{ parentItem }
        , name{ tree, "name" }
        , id{ tree, "id" }
        , description{ tree, "description" }
        , tooltip{ tree, "tooltip" }
        , enabled{ tree, "enabled", true }
        , visible{ tree, "visible", true }
        , alwaysOnTop{ tree, "always-on-top" }
        , bufferedToImage{ tree, "buffered-to-image" }
        , opaque{ tree, "opaque" }
        , focusable{ tree, "focusable" }
        , clickingGrabsFocus{ tree, "clicking-grabs-focus", true }
        , focusOutline{ tree, "focus-outline" }
        , focusOrder{ tree, "focus-order" }
        , opacity{ tree, "opacity", 1.f }
        , cursor{ tree, "cursor", juce::MouseCursor::NormalCursor }
        , width{ tree, "width", Width::fromPixels(-1.f) }
        , height{ tree, "height", Height::fromPixels(-1.f) }
        , display{ tree, "display", Display::flex }
        , overflow{ tree, "overflow", Overflow::hidden }
    {
        jassert(component != nullptr);
        jassert(viewport != nullptr);

        static constexpr auto takeOwnership = false;
        viewport->setViewedComponent(&getComponent(), takeOwnership);
        viewport->addComponentListener(this);

        name.onValueChange = [this]() {
            getComponent().setName(name);
            getComponent().setTitle(name);
        };
        getComponent().setName(name);
        getComponent().setTitle(name);

        id.onValueChange = [this]() {
            getComponent().setComponentID(id.get().toString());
        };
        getComponent().setComponentID(id.get().toString());

        description.onValueChange = [this]() {
            getComponent().setDescription(description);
        };
        getComponent().setDescription(description);

        tooltip.onValueChange = [this]() {
            getComponent().setHelpText(tooltip);
        };
        getComponent().setHelpText(tooltip);

        enabled.onValueChange = [this]() {
            getComponent().setEnabled(enabled);
        };
        getComponent().setEnabled(enabled);

        visible.onValueChange = [this]() {
            getViewport().setVisible(visible);
        };
        getViewport().setVisible(visible);

        alwaysOnTop.onValueChange = [this]() {
            getViewport().setAlwaysOnTop(alwaysOnTop);
        };
        getViewport().setAlwaysOnTop(alwaysOnTop);

        bufferedToImage.onValueChange = [this]() {
            getComponent().setBufferedToImage(bufferedToImage);
        };
        getComponent().setBufferedToImage(bufferedToImage);

        opaque.onValueChange = [this]() {
            getComponent().setOpaque(opaque);
        };
        getComponent().setOpaque(opaque);

        focusable.onValueChange = [this]() {
            getComponent().setWantsKeyboardFocus(focusable);
        };
        getComponent().setWantsKeyboardFocus(focusable);

        focusOutline.onValueChange = [this]() {
            getViewport().setHasFocusOutline(focusOutline);
        };
        getViewport().setHasFocusOutline(focusOutline);

        clickingGrabsFocus.onValueChange = [this]() {
            getComponent().setMouseClickGrabsKeyboardFocus(clickingGrabsFocus);
        };
        getComponent().setMouseClickGrabsKeyboardFocus(clickingGrabsFocus);

        focusOrder.onValueChange = [this]() {
            getViewport().setExplicitFocusOrder(focusOrder);
        };
        getViewport().setExplicitFocusOrder(focusOrder);

        opacity.onValueChange = [this]() {
            getViewport().setAlpha(opacity);
        };
        getViewport().setAlpha(opacity);

        cursor.onValueChange = [this]() {
            getComponent().setMouseCursor(juce::MouseCursor{ cursor });
        };
        getComponent().setMouseCursor(juce::MouseCursor{ cursor });

        width.onValueChange = [this]() {
            updateViewportSize();
        };
        height.onValueChange = [this]() {
            updateViewportSize();
        };
        overflow.onValueChange = [this]() {
            updateComponentSize();
        };

        component->addComponentListener(this);
        updateViewportSize();

        tree.addListener(this);
    }

    GuiItem::GuiItem(std::unique_ptr<juce::Component> comp, juce::ValueTree valueTree, GuiItem* parentItem)
        : GuiItem{ std::shared_ptr<juce::Component>{ std::move(comp) },
                   std::make_shared<juce::Viewport>(),
                   valueTree,
                   parentItem }
    {
    }

    GuiItem::GuiItem(const GuiItem& other)
        : GuiItem{ other.component, other.viewport, other.tree, other.parent }
    {
    }

    GuiItem::~GuiItem()
    {
        viewport->removeComponentListener(this);
        component->removeComponentListener(this);
    }

    //==================================================================================================================
    void GuiItem::updateLayout()
    {
    }

    void GuiItem::updateSize()
    {
        updateViewportSize();
    }

    void GuiItem::updatePosition()
    {
    }

    void GuiItem::informContentChanged()
    {
        contentChanged();
    }

    //==================================================================================================================
    juce::ValueTree GuiItem::getState()
    {
        return tree;
    }

    const juce::Component& GuiItem::getComponent() const
    {
        return *component;
    }

    juce::Component& GuiItem::getComponent()
    {
        return *component;
    }

    const juce::Viewport& GuiItem::getViewport() const
    {
        return *viewport;
    }

    juce::Viewport& GuiItem::getViewport()
    {
        return *viewport;
    }

    void GuiItem::addChild(std::unique_ptr<GuiItem> child)
    {
        auto* newlyAddedChild = children.add(std::move(child));
        component->addChildComponent(newlyAddedChild->getViewport());
        updateComponentSize();
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

    BoxModel GuiItem::getBoxModel() const
    {
        return boxModel;
    }

    float GuiItem::getWidth() const
    {
        if (hasAutoWidth())
            return -1.0f;

        auto length = width.get();
        length.setCorrespondingGuiItem(*this);

        return length;
    }

    float GuiItem::getHeight() const
    {
        if (hasAutoHeight())
            return -1.0f;

        auto length = height.get();
        length.setCorrespondingGuiItem(*this);

        return length;
    }

    GuiItem::Display GuiItem::getDisplay() const
    {
        return display;
    }

    bool GuiItem::hasAutoWidth() const
    {
        if (width.get().isPercent())
            return false;

        return static_cast<int>(width.get()) == -1;
    }

    bool GuiItem::hasAutoHeight() const
    {
        if (height.get().isPercent())
            return false;

        return static_cast<int>(height.get()) == -1;
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
        jassertquiet(&componentThatWasMovedOrResized == component.get()
                     || &componentThatWasMovedOrResized == viewport.get());

        if (!wasResized)
            return;

        if (isTopLevel())
        {
            const auto bounds = getViewport().getLocalBounds();
            const auto w = juce::roundToInt(getWidth());
            const auto h = juce::roundToInt(getHeight());

            if (w != bounds.getWidth())
                width = Width::fromPixels(static_cast<float>(bounds.getWidth()));

            if (h != bounds.getHeight())
                height = Height::fromPixels(static_cast<float>(bounds.getHeight()));
        }

        if (&componentThatWasMovedOrResized == component.get())
            updateLayout();

        updateComponentSize();
    }

    void GuiItem::componentVisibilityChanged(juce::Component& componentThatChangedVisiblity)
    {
        jassertquiet(&componentThatChangedVisiblity == component.get()
                     || &componentThatChangedVisiblity == viewport.get());

        visible = componentThatChangedVisiblity.isVisible();
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
    void GuiItem::updateViewportSize()
    {
        const auto borderBounds = boxModel.getBorderBounds();
        getViewport().setSize(juce::roundToInt(borderBounds.getWidth()),
                              juce::roundToInt(borderBounds.getHeight()));
    }

    void GuiItem::updateComponentSize()
    {
        auto newWidth = getViewport().getWidth();
        auto newHeight = getViewport().getHeight();

        switch (overflow.get())
        {
        case Overflow::hidden:
            break;
        case Overflow::scroll:
        {
            if (getViewport().isVerticalScrollBarShown())
                newWidth -= getViewport().getScrollBarThickness();

            if (getViewport().isHorizontalScrollBarShown())
                newHeight -= getViewport().getScrollBarThickness();

            updateLayout();

            for (auto& child : *this)
            {
                const auto childViewportRight = child.getViewport().getRight();
                const auto requiredWidth = static_cast<int>(std::ceil(childViewportRight
                                                                      + child.getBoxModel().getMargin().getRight()
                                                                      + getBoxModel().getPadding().getRight()));
                newWidth = juce::jmax(newWidth, requiredWidth);

                const auto childViewportBottom = child.getViewport().getBottom();
                const auto requiredHeight = static_cast<int>(std::ceil(childViewportBottom
                                                                       + child.getBoxModel().getMargin().getBottom()
                                                                       + getBoxModel().getPadding().getBottom()));
                newHeight = juce::jmax(newHeight, requiredHeight);
            }

            break;
        }
        default:
            jassertfalse;
            break;
        }

        component->setSize(newWidth, newHeight);
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
        testOverflow();
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
        expect(item->getComponent().getNumChildComponents() == item->getNumChildren());

        item->addChild(createGuiItem(juce::ValueTree{ "Component" }));

        expect(item->getNumChildren() == 1);
        expect(item->getComponent().getNumChildComponents() == item->getNumChildren());

        item->addChild(createGuiItem(juce::ValueTree{ "Component" }));
        item->addChild(createGuiItem(juce::ValueTree{ "Component" }));

        expect(item->getNumChildren() == 3);
        expect(item->getComponent().getNumChildComponents() == item->getNumChildren());
    }

    void testWidthAndHeight()
    {
        beginTest("width and height");

        {
            juce::ValueTree tree{ "Component" };
            auto item = createGuiItem(tree);

            expect(item->hasAutoWidth());
            expect(item->hasAutoHeight());
            expect(item->getViewport().getWidth() == 0);
            expect(item->getViewport().getHeight() == 0);

            tree.setProperty("width", 100.11f, nullptr);
            tree.setProperty("height", 50.55f, nullptr);

            expect(item->getWidth() == 100.11f);
            expect(item->getHeight() == 50.55f);
            expect(item->getViewport().getWidth() == 100);
            expect(item->getViewport().getHeight() == 51);
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
            expectEquals(item->getViewport().getWidth(), 100);
            expectEquals(item->getViewport().getHeight(), 389);
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

        expect(item->getViewport().getWidth() == 200);
        expect(item->getViewport().getHeight() == 150);

        item->getViewport().setSize(400, 300);

        expectEquals(item->getWidth(), 400.f);
        expectEquals(item->getHeight(), 300.f);
    }

    void testName()
    {
        beginTest("name");

        {
            juce::ValueTree tree{ "Component" };
            auto item = createGuiItem(tree);

            expect(item->getComponent().getName().isEmpty());
            expect(item->getComponent().getTitle().isEmpty());

            tree.setProperty("name", "Zaphod Beeblebrox", nullptr);

            expectEquals<juce::String>(item->getComponent().getName(), "Zaphod Beeblebrox");
            expectEquals<juce::String>(item->getComponent().getTitle(), "Zaphod Beeblebrox");
        }
        {
            juce::ValueTree tree{
                "Component",
                {
                    { "name", "Ford Prefect" },
                },
            };
            auto item = createGuiItem(tree);

            expectEquals<juce::String>(item->getComponent().getName(), "Ford Prefect");
            expectEquals<juce::String>(item->getComponent().getTitle(), "Ford Prefect");
        }
    }

    void testID()
    {
        beginTest("ID");

        {
            juce::ValueTree tree{ "Component" };
            auto item = createGuiItem(tree);

            expect(item->getComponent().getComponentID().isEmpty());

            tree.setProperty("id", "comp123", nullptr);

            expectEquals<juce::String>(item->getComponent().getComponentID(), "comp123");
        }
        {
            juce::ValueTree tree{
                "Component",
                {
                    { "id", "specified" },
                },
            };
            auto item = createGuiItem(tree);

            expectEquals<juce::String>(item->getComponent().getComponentID(), "specified");
        }
    }

    void testDescription()
    {
        beginTest("description");

        {
            juce::ValueTree tree{ "Component" };
            auto item = createGuiItem(tree);

            expect(item->getComponent().getDescription().isEmpty());

            tree.setProperty("description", "Don't panic!", nullptr);

            expectEquals<juce::String>(item->getComponent().getDescription(), "Don't panic!");
        }
        {
            juce::ValueTree tree{
                "Component",
                {
                    { "description", "You live and learn. At any rate, you live." },
                },
            };
            auto item = createGuiItem(tree);

            expectEquals<juce::String>(item->getComponent().getDescription(),
                                       "You live and learn. At any rate, you live.");
        }
    }

    void testTooltip()
    {
        beginTest("tooltip");

        {
            juce::ValueTree tree{ "Component" };
            auto item = createGuiItem(tree);

            expect(item->getComponent().getHelpText().isEmpty());

            tree.setProperty("tooltip",
                             "I love deadlines. I love the whooshing noise they make as they go by.",
                             nullptr);

            expectEquals<juce::String>(item->getComponent().getHelpText(),
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

            expectEquals<juce::String>(item->getComponent().getHelpText(),
                                       "I may not have gone where I intended to go, but I think I have ended up where I needed to be.");
        }
    }

    void testVisibility()
    {
        beginTest("visibility");

        {
            juce::ValueTree tree{ "Component" };
            auto item = createGuiItem(tree);

            expect(item->getViewport().isVisible());

            tree.setProperty("visible", false, nullptr);
            expect(!item->getViewport().isVisible());

            juce::ValueTree childTree1{
                "Component",
                {
                    { "visible", true },
                },
            };
            item->addChild(createGuiItem(childTree1));
            expect(item->getChild(0).getViewport().isVisible());

            juce::ValueTree childTree2{
                "Component",
                {
                    { "visible", false },
                },
            };
            item->addChild(createGuiItem(childTree2));
            expect(!item->getChild(1).getViewport().isVisible());
        }
        {
            juce::ValueTree tree{
                "Component",
                {
                    { "visible", false },
                },
            };
            auto item = createGuiItem(tree);

            expect(!item->getViewport().isVisible());
        }
    }

    void testAlwaysOnTop()
    {
        beginTest("always-on-top");

        {
            juce::ValueTree tree{ "Component" };
            auto item = createGuiItem(tree);
            expect(!item->getViewport().isAlwaysOnTop());

            tree.setProperty("always-on-top", true, nullptr);
            expect(item->getViewport().isAlwaysOnTop());
        }
        {
            juce::ValueTree tree{
                "Component",
                {
                    { "always-on-top", true },
                },
            };
            auto item = createGuiItem(tree);
            expect(item->getViewport().isAlwaysOnTop() == static_cast<bool>(tree["always-on-top"]));
        }
    }

    void testBufferedToImage()
    {
        beginTest("buffered-to-image");

        {
            juce::ValueTree tree{ "Component" };
            auto item = createGuiItem(tree);
            expect(item->getComponent().getCachedComponentImage() == nullptr);

            tree.setProperty("buffered-to-image", true, nullptr);
            expect(item->getComponent().getCachedComponentImage() != nullptr);
        }
        {
            juce::ValueTree tree{
                "Component",
                {
                    { "buffered-to-image", true },
                },
            };
            auto item = createGuiItem(tree);
            expect(item->getComponent().getCachedComponentImage() != nullptr);
        }
    }

    void testOpaque()
    {
        beginTest("opaque");

        {
            juce::ValueTree tree{ "Component" };
            auto item = createGuiItem(tree);
            expect(!item->getComponent().isOpaque());

            tree.setProperty("opaque", true, nullptr);
            expect(item->getComponent().isOpaque());
        }
        {
            juce::ValueTree tree{
                "Component",
                {
                    { "opaque", true },
                },
            };
            auto item = createGuiItem(tree);
            expect(item->getComponent().isOpaque());
        }
    }

    void testFocusOrder()
    {
        beginTest("focus-order");

        {
            juce::ValueTree tree{ "Component" };
            auto item = createGuiItem(tree);
            expect(item->getViewport().getExplicitFocusOrder() == 0);

            tree.setProperty("focus-order", 12, nullptr);
            expect(item->getViewport().getExplicitFocusOrder() == 12);
        }
        {
            juce::ValueTree tree{
                "Component",
                {
                    { "focus-order", 4 },
                },
            };
            auto item = createGuiItem(tree);
            expect(item->getViewport().getExplicitFocusOrder() == 4);
        }
    }

    void testFocusable()
    {
        beginTest("focusable");

        {
            juce::ValueTree tree{ "Component" };
            auto item = createGuiItem(tree);
            expect(!item->getComponent().getWantsKeyboardFocus());

            tree.setProperty("focusable", true, nullptr);
            expect(item->getComponent().getWantsKeyboardFocus());
        }
        {
            juce::ValueTree tree{
                "Component",
                {
                    { "focusable", true },
                },
            };
            auto item = createGuiItem(tree);
            expect(item->getComponent().getWantsKeyboardFocus());
        }
    }

    void testClickingGrabsFocus()
    {
        beginTest("clicking-grabs-focus");

        {
            juce::ValueTree tree{ "Component" };
            auto item = createGuiItem(tree);
            expect(item->getComponent().getMouseClickGrabsKeyboardFocus());

            tree.setProperty("clicking-grabs-focus", false, nullptr);
            expect(!item->getComponent().getMouseClickGrabsKeyboardFocus());
        }
        {
            juce::ValueTree tree{
                "Component",
                {
                    { "clicking-grabs-focus", false },
                },
            };
            auto item = createGuiItem(tree);
            expect(!item->getComponent().getMouseClickGrabsKeyboardFocus());
        }
    }

    void testFocusOutline()
    {
        beginTest("focus-outline");

        {
            juce::ValueTree tree{ "Component" };
            auto item = createGuiItem(tree);
            expect(!item->getViewport().hasFocusOutline());

            tree.setProperty("focus-outline", true, nullptr);
            expect(item->getViewport().hasFocusOutline());
        }
        {
            juce::ValueTree tree{
                "Component",
                {
                    { "focus-outline", true },
                },
            };
            auto item = createGuiItem(tree);
            expect(item->getViewport().hasFocusOutline());
        }
    }

    void testEnablement()
    {
        beginTest("enablement");

        {
            juce::ValueTree tree{ "Component" };
            auto item = createGuiItem(tree);
            expect(item->getComponent().isEnabled());

            tree.setProperty("enabled", false, nullptr);
            expect(!item->getComponent().isEnabled());
        }
        {
            juce::ValueTree tree{
                "Component",
                {
                    { "enabled", false },
                },
            };
            auto item = createGuiItem(tree);
            expect(!item->getComponent().isEnabled());
        }
    }

    void testOpacity()
    {
        beginTest("opacity");

        {
            juce::ValueTree tree{ "Component" };
            auto item = createGuiItem(tree);
            expect(item->getViewport().getAlpha() == 1.f);

            tree.setProperty("opacity", 0.42f, nullptr);
            expectWithinAbsoluteError(item->getViewport().getAlpha(), 0.42f, 1.f / 256.f);
        }
        {
            juce::ValueTree tree{
                "Component",
                {
                    { "opacity", 0.123f },
                },
            };
            auto item = createGuiItem(tree);
            expectWithinAbsoluteError(item->getViewport().getAlpha(), 0.123f, 1.f / 256.f);
        }
    }

    void testCursor()
    {
        beginTest("cursor");

        {
            juce::ValueTree tree{ "Component" };
            auto item = createGuiItem(tree);
            expect(item->getComponent().getMouseCursor() == juce::MouseCursor::NormalCursor);

            tree.setProperty("cursor", "left-right", nullptr);
            expect(item->getComponent().getMouseCursor() == juce::MouseCursor::LeftRightResizeCursor);
        }
        {
            juce::ValueTree tree{
                "Component",
                {
                    { "cursor", "wait" },
                },
            };
            auto item = createGuiItem(tree);
            expect(item->getComponent().getMouseCursor() == juce::MouseCursor::WaitCursor);
        }
    }

    void testOverflow()
    {
        beginTest("overflow");

        {
            juce::ValueTree tree{
                "Component",
                {
                    { "width", 120 },
                    { "height", 357 },
                    { "padding", 12 },
                },
                {
                    juce::ValueTree{
                        "Component",
                        {
                            { "width", 200 },
                            { "height", 550 },
                            { "margin", 19 },
                        },
                    },
                },
            };
            auto item = createGuiItem(tree);
            expectEquals(item->getViewport().getBounds(), { 0, 0, 120, 357 });
            expectEquals(item->getViewport().getViewArea(), item->getComponent().getLocalBounds());

            tree.setProperty("overflow", "scroll", nullptr);

            expectEquals(item->getViewport().getBounds(), { 0, 0, 120, 357 });
            expectEquals(item->getViewport().getViewArea(),
                         {
                             0,
                             0,
                             120 - item->getViewport().getScrollBarThickness(),
                             357 - item->getViewport().getScrollBarThickness(),
                         });
            expect(item->getViewport().canScrollHorizontally());
            expect(!item->getViewport().canScrollVertically());
            expectEquals(item->getComponent().getBounds(), { 0, 0, 262, 357 });
        }
        {
            juce::ValueTree tree{
                "Component",
                {
                    { "width", 120 },
                    { "height", 957 },
                    { "padding", 12 },
                    { "overflow", "scroll" },
                },
                {
                    juce::ValueTree{
                        "Component",
                        {
                            { "width", 200 },
                            { "height", 50 },
                            { "margin", 19 },
                        },
                    },
                },
            };
            auto item = createGuiItem(tree);
            expectEquals(item->getViewport().getBounds(), { 0, 0, 120, 957 });
            expectEquals(item->getViewport().getViewArea(),
                         {
                             0,
                             0,
                             120,
                             957 - item->getViewport().getScrollBarThickness(),
                         });
            expect(item->getViewport().canScrollHorizontally());
            expect(!item->getViewport().canScrollVertically());
            expectEquals(item->getComponent().getBounds(),
                         {
                             0,
                             0,
                             262,
                             957 - item->getViewport().getScrollBarThickness(),
                         });
        }
    }
};

static GuiItemUnitTest guiItemUnitTest;
#endif
