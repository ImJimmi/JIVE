#include <jive_layouts/jive_layouts.h>

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    FlexContainer::FlexContainer(std::unique_ptr<GuiItem> itemToDecorate)
        : ContainerItem{ std::move(itemToDecorate) }
        , flexDirection{ state, "flex-direction", juce::FlexBox::Direction::column }
        , flexWrap{ state, "flex-wrap" }
        , flexJustifyContent{ state, "justify-content" }
        , flexAlignItems{ state, "align-items" }
        , flexAlignContent{ state, "align-content" }
    {
        jassert(state.hasProperty("display"));
        jassert(state["display"] == juce::VariantConverter<Display>::toVar(Display::flex));

        flexDirection.onValueChange = [this]() {
            layoutChanged();
        };
        flexWrap.onValueChange = [this]() {
            layoutChanged();
        };
        flexJustifyContent.onValueChange = [this]() {
            layoutChanged();
        };
        flexAlignItems.onValueChange = [this]() {
            layoutChanged();
        };
        flexAlignContent.onValueChange = [this]() {
            layoutChanged();
        };
    }

    //==================================================================================================================
    void FlexContainer::layOutChildren()
    {
        const auto bounds = boxModel.getContentBounds();

        if (bounds.getWidth() <= 0 || bounds.getHeight() <= 0)
            return;

        buildFlexBox(bounds, LayoutStrategy::real).performLayout(bounds);
    }

    //==================================================================================================================
    FlexContainer::operator juce::FlexBox()
    {
        return buildFlexBox(boxModel.getContentBounds(), LayoutStrategy::real);
    }

    //==================================================================================================================
    juce::Rectangle<float> FlexContainer::calculateIdealSize(juce::Rectangle<float> constraints) const
    {
        switch (flexDirection.getOr(juce::FlexBox{}.flexDirection))
        {
        case juce::FlexBox::Direction::column:
        case juce::FlexBox::Direction::columnReverse:
            constraints.setHeight(std::numeric_limits<float>::max());
            break;
        case juce::FlexBox::Direction::row:
        case juce::FlexBox::Direction::rowReverse:
            constraints.setWidth(std::numeric_limits<float>::max());
            break;
        default:
            jassertfalse;
        }

        const auto flex = const_cast<FlexContainer&>(*this)
                              .buildFlexBox(constraints, LayoutStrategy::dummy);
        juce::Point<float> extremities{ -1.0f, -1.0f };

        for (const auto& flexItem : flex.items)
        {
            const auto right = flexItem.currentBounds.getRight() + flexItem.margin.right;
            const auto bottom = flexItem.currentBounds.getBottom() + flexItem.margin.bottom;

            if (right > extremities.x)
                extremities.x = right;
            if (bottom > extremities.y)
                extremities.y = bottom;
        }

        return {
            extremities.x + boxModel.getPadding().getLeftAndRight() + boxModel.getBorder().getLeftAndRight(),
            extremities.y + boxModel.getPadding().getTopAndBottom() + boxModel.getBorder().getTopAndBottom(),
        };
    }

    //==================================================================================================================
    void appendChildren(GuiItem& container,
                        juce::FlexBox& flex,
                        juce::Rectangle<float> bounds,
                        LayoutStrategy strategy)
    {
        for (auto& child : container)
        {
            if (auto* const decoratedItem = dynamic_cast<GuiItemDecorator*>(&child))
            {
                if (auto* const flexItem = decoratedItem->toType<FlexItem>())
                    flex.items.add(flexItem->toJuceFlexItem(bounds, strategy));
            }
        }
    }

    juce::FlexBox FlexContainer::buildFlexBox(juce::Rectangle<float> bounds,
                                              LayoutStrategy strategy)
    {
        juce::FlexBox flex;

        flex.flexDirection = flexDirection;
        flex.flexWrap = flexWrap;

        appendChildren(*this, flex, bounds, strategy);

        switch (strategy)
        {
        case LayoutStrategy::real:
            flex.justifyContent = flexJustifyContent;
            flex.alignItems = flexAlignItems;
            flex.alignContent = flexAlignContent;
            break;
        case LayoutStrategy::dummy:
            flex.justifyContent = juce::FlexBox::JustifyContent::flexStart;
            flex.alignItems = juce::FlexBox::AlignItems::flexStart;
            flex.alignContent = juce::FlexBox::AlignContent::flexStart;

            flex.performLayout(bounds);

            break;
        default:
            jassertfalse;
        }

        return flex;
    }
} // namespace jive

#if JIVE_UNIT_TESTS
class FlexContainerUnitTest : public juce::UnitTest
{
public:
    FlexContainerUnitTest()
        : juce::UnitTest{ "jive::FlexContainer", "jive" }
    {
    }

    void runTest() final
    {
        testDirection();
        testWrap();
        testAlignContent();
        testAlignItems();
        testJustifyContent();
        testChildren();
        testPadding();
        testAutoSize();
        testNestedWidgetWithText();
    }

private:
    std::unique_ptr<jive::FlexContainer> createFlexContainer(juce::ValueTree tree)
    {
        jive::Interpreter interpreter;

        return std::unique_ptr<jive::FlexContainer>(dynamic_cast<jive::FlexContainer*>(interpreter.interpret(tree).release()));
    }

    void testDirection()
    {
        beginTest("direction");

        juce::ValueTree tree{
            "Component",
            {
                { "width", 222 },
                { "height", 333 },
            },
        };
        auto item = createFlexContainer(tree);
        auto flexBox = static_cast<juce::FlexBox>(*item);

        expect(flexBox.flexDirection == juce::FlexBox::Direction::column);

        tree.setProperty("flex-direction", "row-reverse", nullptr);
        flexBox = static_cast<juce::FlexBox>(*item);

        expect(flexBox.flexDirection == juce::FlexBox::Direction::rowReverse);
    }

    void testWrap()
    {
        beginTest("wrap");

        juce::ValueTree tree{
            "Component",
            {
                { "width", 222 },
                { "height", 333 },
            },
        };
        auto item = createFlexContainer(tree);
        auto flexBox = static_cast<juce::FlexBox>(*item);

        expect(flexBox.flexWrap == juce::FlexBox::Wrap::noWrap);

        tree.setProperty("flex-wrap", "wrap-reverse", nullptr);
        flexBox = static_cast<juce::FlexBox>(*item);

        expect(flexBox.flexWrap == juce::FlexBox::Wrap::wrapReverse);
    }

    void testAlignContent()
    {
        beginTest("align-content");

        juce::ValueTree tree{
            "Component",
            {
                { "width", 222 },
                { "height", 333 },
            },
        };
        auto item = createFlexContainer(tree);
        auto flexBox = static_cast<juce::FlexBox>(*item);

        expect(flexBox.alignContent == juce::FlexBox::AlignContent::stretch);

        tree.setProperty("align-content", "space-between", nullptr);
        flexBox = static_cast<juce::FlexBox>(*item);

        expect(flexBox.alignContent == juce::FlexBox::AlignContent::spaceBetween);
    }

    void testAlignItems()
    {
        beginTest("align-items");

        juce::ValueTree tree{
            "Component",
            {
                { "width", 222 },
                { "height", 333 },
            },
        };
        auto item = createFlexContainer(tree);
        auto flexBox = static_cast<juce::FlexBox>(*item);

        expect(flexBox.alignItems == juce::FlexBox::AlignItems::stretch);

        tree.setProperty("align-items", "centre", nullptr);
        flexBox = static_cast<juce::FlexBox>(*item);

        expect(flexBox.alignItems == juce::FlexBox::AlignItems::center);
    }

    void testJustifyContent()
    {
        beginTest("justify-content");

        juce::ValueTree tree{
            "Component",
            {
                { "width", 222 },
                { "height", 333 },
            },
        };
        auto item = createFlexContainer(tree);
        auto flexBox = static_cast<juce::FlexBox>(*item);

        expect(flexBox.justifyContent == juce::FlexBox::JustifyContent::flexStart);

        tree.setProperty("justify-content", "centre", nullptr);
        flexBox = static_cast<juce::FlexBox>(*item);

        expect(flexBox.justifyContent == juce::FlexBox::JustifyContent::center);
    }

    void testChildren()
    {
        beginTest("children");

        juce::ValueTree tree{
            "Component",
            {
                { "width", 222 },
                { "height", 333 },
            },
        };
        auto item = createFlexContainer(tree);
        auto flexBox = static_cast<juce::FlexBox>(*item);

        expect(flexBox.items.isEmpty());

        tree.appendChild(juce::ValueTree{ "Component" }, nullptr);
        tree.appendChild(juce::ValueTree{ "Component" }, nullptr);
        tree.appendChild(juce::ValueTree{ "Component" }, nullptr);

        flexBox = static_cast<juce::FlexBox>(*item);

        expect(flexBox.items.size() == item->getNumChildren());
    }

    void testPadding()
    {
        beginTest("padding");

        juce::ValueTree tree{
            "Component",
            {
                { "width", 222 },
                { "height", 333 },
            },
            {
                juce::ValueTree{
                    "Component",
                    {
                        { "width", 100 },
                        { "height", 100 },
                    },
                },
            },
        };
        auto item = createFlexContainer(tree);
        expectEquals(item->getChild(0).getComponent()->getPosition(), juce::Point<int>{ 0, 0 });

        tree.setProperty("padding", "10 20 30 40", nullptr);
        expectEquals(item->getChild(0).getComponent()->getPosition(), juce::Point<int>{ 40, 10 });
    }

    void testAutoSize()
    {
        beginTest("auto-size");

        {
            juce::ValueTree parentState{
                "Component",
                {
                    { "id", "parent" },
                    { "width", 222 },
                    { "height", 333 },
                    { "display", "block" },
                },
                {
                    juce::ValueTree{
                        "Component",
                        {
                            { "display", "flex" },
                        },
                    },
                },
            };
            jive::Interpreter interpreter;
            auto parent = interpreter.interpret(parentState);
            auto& item = parent->getChild(0);
            expectEquals(item.boxModel.getWidth(), 0.0f);
            expectEquals(item.boxModel.getHeight(), 0.0f);
        }
        {
            juce::ValueTree parentState{
                "Component",
                {
                    { "id", "parent" },
                    { "width", 222 },
                    { "height", 333 },
                },
                {
                    juce::ValueTree{
                        "Component",
                        {
                            { "id", "item" },
                            { "display", "flex" },
                        },
                        {
                            juce::ValueTree{
                                "Component",
                                {
                                    { "id", "child" },
                                    { "width", 43 },
                                    { "height", 84 },
                                },
                            },
                        },
                    },
                },
            };
            jive::Interpreter interpreter;
            auto parent = interpreter.interpret(parentState);
            auto& item = parent->getChild(0);
            expectEquals(item.boxModel.getWidth(), 222.0f);
            expectEquals(item.boxModel.getHeight(), 84.0f);
            expectEquals(item.getChild(0).boxModel.getWidth(), 43.0f);
            expectEquals(item.getChild(0).boxModel.getHeight(), 84.0f);
        }
        {
            juce::ValueTree parentState{
                "Component",
                {
                    { "id", "parent" },
                    { "width", 222 },
                    { "height", 333 },
                },
                {
                    juce::ValueTree{
                        "Component",
                        {
                            { "display", "flex" },
                        },
                        { juce::ValueTree{
                              "Component",
                              {
                                  { "width", 43 },
                                  { "height", 84 },
                              },
                          },
                          juce::ValueTree{
                              "Component",
                              {
                                  { "width", 37 },
                                  { "height", 99 },
                                  { "margin", 5 },
                              },
                          } },
                    },
                },
            };
            jive::Interpreter interpreter;
            auto parent = interpreter.interpret(parentState);
            auto& item = parent->getChild(0);
            expectEquals(item.boxModel.getWidth(), 222.0f);
            expectEquals(item.boxModel.getHeight(), 193.0f);
            expectEquals(item.getChild(0).boxModel.getWidth(), 43.0f);
            expectEquals(item.getChild(0).boxModel.getHeight(), 84.0f);
            expectEquals(item.getChild(1).boxModel.getWidth(), 37.0f);
            expectEquals(item.getChild(1).boxModel.getHeight(), 99.0f);

            item.state.setProperty("flex-direction", "row", nullptr);
            expectGreaterOrEqual(item.boxModel.getWidth(), 90.0f);
            expectGreaterOrEqual(item.boxModel.getHeight(), 109.0f);
            expectEquals(item.getChild(0).boxModel.getWidth(), 43.0f);
            expectEquals(item.getChild(0).boxModel.getHeight(), 84.0f);
            expectEquals(item.getChild(1).boxModel.getWidth(), 37.0f);
            expectEquals(item.getChild(1).boxModel.getHeight(), 99.0f);
        }
    }

    void testNestedWidgetWithText()
    {
        beginTest("nested widget with text");

        juce::ValueTree containerState{
            "Component",
            {
                { "display", "flex" },
            },
        };
        juce::ValueTree windowState{
            "Window",
            {
                { "width", 150 },
                { "height", 1000 },
                { "align-items", "flex-start" },
            },
            {
                containerState,
            },
        };
        const jive::Interpreter interpreter;

        {
            const auto window = interpreter.interpret(windowState);
            jassert(window != nullptr);

            const auto& container = window->getChild(0);
            expectEquals(container.boxModel.getWidth(), 0.0f);
            expectEquals(container.boxModel.getHeight(), 0.0f);
        }

        juce::ValueTree buttonState{
            "Button",
            {
                { "width", 50 },
                { "height", 20 },
                { "min-height", 0 },
                { "min-width", 0 },
                { "padding", 0 },
            },
        };
        containerState.appendChild(buttonState, nullptr);

        {
            const auto window = interpreter.interpret(windowState);
            jassert(window != nullptr);

            const auto& container = window->getChild(0);
            expectEquals(container.boxModel.getWidth(), 50.0f);
            expectEquals(container.boxModel.getHeight(), 20.0f);
        }

        buttonState.removeProperty("width", nullptr);
        buttonState.removeProperty("height", nullptr);

        const juce::Font font;

        juce::ValueTree textState{
            "Text",
            {
                { "text", "Lorem ipsum" },
                { "typeface-name", font.getTypefaceName() },
                { "font-height", font.getHeightInPoints() },
            },
        };
        buttonState.appendChild(textState, nullptr);

        {
            const auto window = interpreter.interpret(windowState);
            jassert(window != nullptr);

            const auto& container = window->getChild(0);
            expectEquals(container.boxModel.getWidth(), std::ceil(font.getStringWidthFloat(textState["text"])));
            expectEquals(container.boxModel.getHeight(), font.getHeight());
        }

        const juce::String lorumIpsumSentence = "Lorem ipsum dolor sit amet, consectetur adipiscing elit.";
        textState.setProperty("text", lorumIpsumSentence, nullptr);

        {
            const auto window = interpreter.interpret(windowState);
            jassert(window != nullptr);

            const juce::AttributedString attributedString{ lorumIpsumSentence };
            juce::TextLayout layout;
            layout.createLayout(attributedString, 150.0f);

            const auto& container = window->getChild(0);
            expectEquals(container.boxModel.getWidth(), 150.0f);
            expectEquals(container.boxModel.getHeight(), std::ceil(layout.getHeight()));
        }
    }
};

static FlexContainerUnitTest flexContainerUnitTest;
#endif
