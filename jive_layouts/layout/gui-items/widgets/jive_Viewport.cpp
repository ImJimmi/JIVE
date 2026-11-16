#include "jive_Viewport.h"

#include <jive_layouts/layout/gui-items/jive_ContainerItem.h>

namespace jive
{
    Viewport::Viewport(std::unique_ptr<GuiItem> itemToDecorate)
        : GuiItemDecorator{ std::move(itemToDecorate) }
    {
        viewport.setScrollBarsShown(true, false);
        viewport.setViewedComponent(&scrolledArea, false);

        getComponent()->addComponentListener(this);
        state.addListener(this);

        if (getComponent()->getParentComponent() != nullptr)
            takePlaceOfContent();
    }

    Viewport::~Viewport()
    {
        state.removeListener(this);
        getComponent()->removeComponentListener(this);
        scrolledArea.removeChildComponent(getComponent().get());
    }

    juce::Viewport& Viewport::getViewport()
    {
        return viewport;
    }

    const juce::Viewport& Viewport::getViewport() const
    {
        return viewport;
    }

    bool Viewport::isInViewport() const
    {
        return getComponent()->getParentComponent() == &scrolledArea;
    }

    void Viewport::componentParentHierarchyChanged(juce::Component& changedComponent)
    {
        if (&changedComponent != getComponent().get()
            || changedComponent.getParentComponent() == nullptr
            || isInViewport())
        {
            return;
        }

        takePlaceOfContent();
    }

    void Viewport::takePlaceOfContent()
    {
        auto& content = *getComponent();
        auto* parentComponent = content.getParentComponent();
        const auto index = parentComponent->getIndexOfChildComponent(&content);

        viewport.setBounds(content.getBounds());
        viewport.setVisible(content.isVisible());
        parentComponent->addChildComponent(viewport, index);
        scrolledArea.addChildComponent(content);
        fitContent();
    }

    void Viewport::componentMovedOrResized(juce::Component& changedComponent, bool wasMoved, bool wasResized)
    {
        if (&changedComponent != getComponent().get() || fittingContent || !isInViewport())
            return;

        if (wasMoved)
            viewport.setTopLeftPosition(changedComponent.getPosition());
        if (wasResized)
            viewport.setSize(changedComponent.getWidth(), changedComponent.getHeight());

        fitContent();
    }

    void Viewport::componentVisibilityChanged(juce::Component& changedComponent)
    {
        if (&changedComponent == getComponent().get())
            viewport.setVisible(changedComponent.isVisible());
    }

    void Viewport::valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& id)
    {
        if (tree == state && id == ids::idealHeight() && !fittingContent && isInViewport())
            fitContent();
    }

    void Viewport::fitContent()
    {
        auto* container = toType<ContainerItem>();

        if (container == nullptr || viewport.getWidth() <= 0)
            return;

        const juce::ScopedValueSetter<bool> fitting{ fittingContent, true };
        const auto visibleHeight = static_cast<float>(viewport.getHeight());
        auto width = viewport.getWidth();
        auto height = container->calculateIdealHeight(static_cast<float>(width));

        if (height > visibleHeight)
        {
            width -= viewport.getScrollBarThickness();
            height = container->calculateIdealHeight(static_cast<float>(width));
        }

        const auto contentHeight = juce::roundToInt(std::ceil(juce::jmax(height, visibleHeight)));
        scrolledArea.setSize(width, contentHeight);
        getComponent()->setBounds(0, 0, width, contentHeight);

        // The content may be resized while its item isn't listening, so its
        // box model is told directly.
        boxModel(*this).setSize(static_cast<float>(width), static_cast<float>(contentHeight));
    }
} // namespace jive

#if JIVE_UNIT_TESTS
    #include <jive_layouts/layout/interpreter/jive_Interpreter.h>

class ViewportTest : public juce::UnitTest
{
public:
    ViewportTest()
        : juce::UnitTest{ "jive::Viewport", "jive" }
    {
    }

    void runTest() final
    {
        testContentScrolls();
        testContentFits();
        testDoesNotGrowParent();
        testTakesContentHeight();
        testShrinksWhenThereIsNoRoom();
        testGridContentFitsBesideScrollBar();
        testChildrenStyledAsChildren();
    }

private:
    struct Fixture
    {
        explicit Fixture(const juce::String& xml)
            : root{ interpreter.interpret(jive::parseXML(xml)) }
        {
            root->getComponent()->setSize(200, 100);
        }

        jive::GuiItem& viewportItem()
        {
            return *root->getChildren()[0];
        }

        jive::Viewport& viewport()
        {
            return *dynamic_cast<jive::GuiItemDecorator&>(viewportItem()).toType<jive::Viewport>();
        }

        jive::Interpreter interpreter;
        std::unique_ptr<jive::GuiItem> root;
    };

    void testContentScrolls()
    {
        beginTest("viewport / content taller than the viewport scrolls");

        Fixture fixture{ R"(
            <Component width="200" height="100">
                <Viewport flex-grow="1">
                    <Component height="80" />
                    <Component height="80" />
                    <Component height="80" />
                </Viewport>
            </Component>
        )" };
        auto& viewport = fixture.viewport().getViewport();
        expectEquals(viewport.getHeight(), 100);
        expectEquals(viewport.getWidth(), 200);
        expect(viewport.getParentComponent() == fixture.root->getComponent().get());

        auto& content = *fixture.viewportItem().getComponent();
        expectEquals(content.getHeight(), 240);
        expectEquals(content.getWidth(), 200 - viewport.getScrollBarThickness());
        expectEquals(fixture.viewportItem().getChildren()[2]->getComponent()->getY(), 160);

        viewport.setViewPosition(0, 140);
        expectEquals(viewport.getViewPosition().y, 140);
    }

    void testContentFits()
    {
        beginTest("viewport / content shorter than the viewport fills it, without a scroll bar");

        Fixture fixture{ R"(
            <Component width="200" height="100">
                <Viewport flex-grow="1">
                    <Component height="30" />
                </Viewport>
            </Component>
        )" };
        auto& content = *fixture.viewportItem().getComponent();
        expectEquals(content.getWidth(), 200);
        expectEquals(content.getHeight(), 100);
    }

    void testDoesNotGrowParent()
    {
        beginTest("viewport / its content doesn't make it, or its parent, grow");

        Fixture fixture{ R"(
            <Component width="200" height="100">
                <Viewport flex-grow="1">
                    <Component height="500" />
                </Viewport>
                <Component height="20" />
            </Component>
        )" };
        expectEquals(fixture.viewport().getViewport().getHeight(), 80);
        expectEquals(fixture.root->getChildren()[1]->getComponent()->getY(), 80);
    }

    void testTakesContentHeight()
    {
        beginTest("viewport / it's as tall as its content when there's room");

        Fixture fixture{ R"(
            <Component width="200" height="100">
                <Viewport>
                    <Component height="30" />
                </Viewport>
            </Component>
        )" };
        expectEquals(fixture.viewport().getViewport().getHeight(), 30);
    }

    void testShrinksWhenThereIsNoRoom()
    {
        beginTest("viewport / it shrinks to fit when its content doesn't");

        Fixture fixture{ R"(
            <Component width="200" height="100">
                <Viewport>
                    <Component height="500" />
                </Viewport>
                <Component height="20" />
            </Component>
        )" };
        expectEquals(fixture.viewport().getViewport().getHeight(), 80);
        expectEquals(fixture.viewportItem().getComponent()->getHeight(), 500);
        expectEquals(fixture.viewportItem().getComponent()->getWidth(),
                     200 - fixture.viewport().getViewport().getScrollBarThickness());
    }

    void testGridContentFitsBesideScrollBar()
    {
        beginTest("viewport / grid content is laid out beside the scroll bar, not under it");

        Fixture fixture{ R"(
            <Component width="200" height="100">
                <Viewport display="grid" grid-template-columns="auto 1fr">
                    <Component width="20" height="300" />
                    <Component height="20" />
                </Viewport>
                <Component height="20" />
            </Component>
        )" };
        const auto contentWidth = 200 - fixture.viewport().getViewport().getScrollBarThickness();
        expectEquals(fixture.viewportItem().getComponent()->getWidth(), contentWidth);
        expectEquals(fixture.viewportItem().getChildren()[1]->getComponent()->getRight(), contentWidth);
    }

    void testChildrenStyledAsChildren()
    {
        beginTest("viewport / children are still direct children of the viewport's element");

        Fixture fixture{ R"(
            <Component width="200" height="100">
                <Viewport flex-grow="1">
                    <Component height="30" />
                </Viewport>
            </Component>
        )" };
        auto& content = *fixture.viewportItem().getComponent();
        expect(fixture.viewportItem().getChildren()[0]->getComponent()->getParentComponent() == &content);
    }
};

static ViewportTest viewportTest;
#endif
