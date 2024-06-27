#include "jive_FlexContainer.h"

#include "jive_FlexItem.h"

#include <jive_layouts/layout/gui-items/jive_CommonGuiItem.h>

namespace jive
{
    FlexContainer::FlexContainer(std::unique_ptr<GuiItem> itemToDecorate)
        : ContainerItem{ std::move(itemToDecorate) }
        , flexDirection{ state, "flex-direction" }
        , flexWrap{ state, "flex-wrap" }
        , flexJustifyContent{ state, "justify-content" }
        , flexAlignItems{ state, "align-items" }
        , flexAlignContent{ state, "align-content" }
    {
        jassert(state.hasProperty("display"));
        jassert(state["display"] == juce::VariantConverter<Display>::toVar(Display::flex));

        if (!flexDirection.exists())
            flexDirection = juce::FlexBox::Direction::column;

        const auto onPropertyChanged = [this] {
            layoutChanged();
        };
        flexDirection.onValueChange = onPropertyChanged;
        flexWrap.onValueChange = onPropertyChanged;
        flexJustifyContent.onValueChange = onPropertyChanged;
        flexAlignItems.onValueChange = onPropertyChanged;
        flexAlignContent.onValueChange = onPropertyChanged;

        state.addListener(this);
    }

    FlexContainer::~FlexContainer()
    {
        state.removeListener(this);
    }

    void FlexContainer::layOutChildren()
    {
        if (layoutRecursionLock)
            return;

        const juce::ScopedValueSetter svs{ layoutRecursionLock, true };

        GuiItemDecorator::layOutChildren();

        const auto bounds = boxModel(*this).getContentBounds();

        if (bounds.isEmpty())
            return;

        do
        {
            changesDuringLayout = false;
            buildFlexBox(bounds, LayoutStrategy::real)
                .performLayout(bounds);
        }
        while (changesDuringLayout);
    }

    FlexContainer::operator juce::FlexBox()
    {
        return buildFlexBox(boxModel(*this).getContentBounds(), LayoutStrategy::real);
    }

    juce::Rectangle<float> FlexContainer::calculateIdealSize(juce::Rectangle<float> constraints) const
    {
        constraints = constraints.withZeroOrigin();

        switch (flexDirection.getOr(juce::FlexBox{}.flexDirection))
        {
        case juce::FlexBox::Direction::column:
        case juce::FlexBox::Direction::columnReverse:
            constraints.setHeight(static_cast<float>(std::numeric_limits<juce::uint16>::max()));
            break;
        case juce::FlexBox::Direction::row:
        case juce::FlexBox::Direction::rowReverse:
            constraints.setWidth(static_cast<float>(std::numeric_limits<juce::uint16>::max()));
            break;
        default:
            jassertfalse;
        }

        auto flex = const_cast<FlexContainer&>(*this)
                        .buildFlexBox(constraints, LayoutStrategy::dummy);
        flex.performLayout(constraints);

        juce::Point extremities{ -1.0f, -1.0f };

        for (const auto& flexItem : flex.items)
        {
            const auto right = flexItem.currentBounds.getRight() + flexItem.margin.right;
            const auto bottom = flexItem.currentBounds.getBottom() + flexItem.margin.bottom;

            if (right > extremities.x)
                extremities.x = right;
            if (bottom > extremities.y)
                extremities.y = bottom;
        }

        auto& currentBoxModel = boxModel(*this);

        return {
            extremities.x
                + currentBoxModel
                      .getPadding()
                      .getLeftAndRight()
                + currentBoxModel
                      .getBorder()
                      .getLeftAndRight(),
            extremities.y
                + currentBoxModel
                      .getPadding()
                      .getTopAndBottom()
                + currentBoxModel
                      .getBorder()
                      .getTopAndBottom(),
        };
    }

    static void appendChildren(GuiItem& container,
                               juce::FlexBox& flex,
                               juce::Rectangle<float> bounds,
                               LayoutStrategy strategy)
    {
        for (auto* child : container.getChildren())
        {
            if (auto* const decoratedItem = dynamic_cast<GuiItemDecorator*>(child))
            {
                if (auto* const flexItem = decoratedItem->toType<FlexItem>())
                    flex.items.add(flexItem->toJuceFlexItem(bounds, strategy));
            }
        }
    }

    void FlexContainer::valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& id)
    {
        if (tree != state && tree.getParent() != state)
            return;

        if (layoutRecursionLock)
        {
            static const juce::Array<juce::Identifier> propertiesForWhichChangesRequireAnotherLayOut{
                "ideal-width",
                "ideal-height",
            };

            if (propertiesForWhichChangesRequireAnotherLayOut.contains(id))
                changesDuringLayout = true;
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
            break;
        default:
            jassertfalse;
        }

        return flex;
    }
} // namespace jive

#if JIVE_UNIT_TESTS
    #include <jive_layouts/layout/jive_Interpreter.h>

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
        jive::Interpreter interpreter;
        auto item = interpreter.interpret(tree);
        auto flexBox = static_cast<juce::FlexBox>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                                       .toType<jive::FlexContainer>());

        expect(flexBox.flexDirection == juce::FlexBox::Direction::column);

        tree.setProperty("flex-direction", "row-reverse", nullptr);
        flexBox = static_cast<juce::FlexBox>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                                  .toType<jive::FlexContainer>());

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
        jive::Interpreter interpreter;
        auto item = interpreter.interpret(tree);
        auto flexBox = static_cast<juce::FlexBox>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                                       .toType<jive::FlexContainer>());

        expect(flexBox.flexWrap == juce::FlexBox::Wrap::noWrap);

        tree.setProperty("flex-wrap", "wrap-reverse", nullptr);
        flexBox = static_cast<juce::FlexBox>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                                  .toType<jive::FlexContainer>());

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
        jive::Interpreter interpreter;
        auto item = interpreter.interpret(tree);
        auto flexBox = static_cast<juce::FlexBox>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                                       .toType<jive::FlexContainer>());

        expect(flexBox.alignContent == juce::FlexBox::AlignContent::stretch);

        tree.setProperty("align-content", "space-between", nullptr);
        flexBox = static_cast<juce::FlexBox>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                                  .toType<jive::FlexContainer>());

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
        jive::Interpreter interpreter;
        auto item = interpreter.interpret(tree);
        auto flexBox = static_cast<juce::FlexBox>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                                       .toType<jive::FlexContainer>());

        expect(flexBox.alignItems == juce::FlexBox::AlignItems::stretch);

        tree.setProperty("align-items", "centre", nullptr);
        flexBox = static_cast<juce::FlexBox>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                                  .toType<jive::FlexContainer>());

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
        jive::Interpreter interpreter;
        auto item = interpreter.interpret(tree);
        auto flexBox = static_cast<juce::FlexBox>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                                       .toType<jive::FlexContainer>());

        expect(flexBox.justifyContent == juce::FlexBox::JustifyContent::flexStart);

        tree.setProperty("justify-content", "centre", nullptr);
        flexBox = static_cast<juce::FlexBox>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                                  .toType<jive::FlexContainer>());

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
        jive::Interpreter interpreter;
        auto item = interpreter.interpret(tree);
        auto flexBox = static_cast<juce::FlexBox>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                                       .toType<jive::FlexContainer>());

        expect(flexBox.items.isEmpty());

        tree.appendChild(juce::ValueTree{ "Component" }, nullptr);
        tree.appendChild(juce::ValueTree{ "Component" }, nullptr);
        tree.appendChild(juce::ValueTree{ "Component" }, nullptr);

        flexBox = static_cast<juce::FlexBox>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                                  .toType<jive::FlexContainer>());

        expect(flexBox.items.size() == item->getChildren().size());
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
        jive::Interpreter interpreter;
        auto item = interpreter.interpret(tree);
        expectEquals(item->getChildren()[0]->getComponent()->getPosition(),
                     juce::Point<int>{ 0, 0 });

        tree.setProperty("padding", "10 20 30 40", nullptr);
        expectEquals(item->getChildren()[0]->getComponent()->getPosition(),
                     juce::Point<int>{ 40, 10 });
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
            auto& item = *parent->getChildren()[0];
            const auto& boxModel = jive::boxModel(item);
            expectEquals(boxModel.getWidth(), 0.0f);
            expectEquals(boxModel.getHeight(), 0.0f);
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
            auto& item = *parent->getChildren()[0];
            const auto& boxModel = jive::boxModel(item);
            const auto& childBoxModel = jive::boxModel(*item.getChildren()[0]);
            expectEquals(boxModel.getWidth(), 222.0f);
            expectEquals(boxModel.getHeight(), 84.0f);
            expectEquals(childBoxModel.getWidth(), 43.0f);
            expectEquals(childBoxModel.getHeight(), 84.0f);
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
            auto& item = *parent->getChildren()[0];
            const auto& boxModel = jive::boxModel(item);
            const auto& child0BoxModel = jive::boxModel(*item.getChildren()[0]);
            const auto& child1BoxModel = jive::boxModel(*item.getChildren()[1]);
            expectEquals(boxModel.getWidth(), 222.0f);
            expectEquals(boxModel.getHeight(), 193.0f);
            expectEquals(child0BoxModel.getWidth(), 43.0f);
            expectEquals(child0BoxModel.getHeight(), 84.0f);
            expectEquals(child1BoxModel.getWidth(), 37.0f);
            expectEquals(child1BoxModel.getHeight(), 99.0f);

            item.state.setProperty("flex-direction", "row", nullptr);
            expectGreaterOrEqual(boxModel.getWidth(), 90.0f);
            expectGreaterOrEqual(boxModel.getHeight(), 109.0f);
            expectEquals(child0BoxModel.getWidth(), 43.0f);
            expectEquals(child0BoxModel.getHeight(), 84.0f);
            expectEquals(child1BoxModel.getWidth(), 37.0f);
            expectEquals(child1BoxModel.getHeight(), 99.0f);
        }
    }

    void testNestedWidgetWithText()
    {
        beginTest("nested widget with text");

        static constexpr auto containerPadding = 20;

        juce::ValueTree containerState{
            "Component",
            {
                { "display", "flex" },
                { "padding", containerPadding },
            },
        };
        juce::ValueTree windowState{
            "Window",
            {
                { "native", true },
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

            const auto& container = *window->getChildren()[0];
            const auto& boxModel = jive::boxModel(container);
            expectEquals(boxModel.getContentBounds().getWidth(), 0.0f);
            expectEquals(boxModel.getContentBounds().getHeight(), 0.0f);
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

            const auto& container = *window->getChildren()[0];
            const auto& boxModel = jive::boxModel(container);
            expectEquals(boxModel.getContentBounds().getWidth(), 50.f);
            expectEquals(boxModel.getContentBounds().getHeight(), 20.0f);
        }

        buttonState.removeProperty("width", nullptr);
        buttonState.removeProperty("height", nullptr);

        const juce::Font font{
    #if JUCE_MAJOR_VERSION >= 8
            juce::FontOptions{},
    #endif
        };

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

            const auto& container = *window->getChildren()[0];
            const auto& boxModel = jive::boxModel(container);
            expectEquals(boxModel.getContentBounds().getWidth(),
                         std::ceil(font.getStringWidthFloat(textState["text"])));
            expectEquals(boxModel.getContentBounds().getHeight(),
                         font.getHeight());
        }

        const juce::String lorumIpsumSentence = "Lorem ipsum dolor sit amet, consectetur adipiscing elit.";
        textState.setProperty("text", lorumIpsumSentence, nullptr);

        {
            const auto window = interpreter.interpret(windowState);
            jassert(window != nullptr);

            const auto& container = *window->getChildren()[0];

            const juce::AttributedString attributedString{ lorumIpsumSentence };
            juce::TextLayout layout;
            layout.createLayout(attributedString, jive::boxModel(container).getContentBounds().getWidth());

            const auto& boxModel = jive::boxModel(container);
            expectEquals(boxModel.getWidth(), 150.0f);
            expectEquals(boxModel.getContentBounds().getHeight(), std::ceil(layout.getHeight()));
        }
    }
};

static FlexContainerUnitTest flexContainerUnitTest;
#endif
