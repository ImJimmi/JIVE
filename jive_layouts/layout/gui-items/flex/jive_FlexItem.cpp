#include <jive_layouts/jive_layouts.h>

namespace jive
{
    FlexItem::FlexItem(std::unique_ptr<GuiItem> itemToDecorate)
        : GuiItemDecorator{ std::move(itemToDecorate) }
        , order{ state, "order" }
        , flexGrow{ state, "flex-grow" }
        , flexShrink{ state, "flex-shrink" }
        , flexBasis{ state, "flex-basis" }
        , alignSelf{ state, "align-self" }
        , width{ state, "width" }
        , height{ state, "height" }
        , minWidth{ state, "min-width" }
        , minHeight{ state, "min-height" }
        , idealWidth{ state, "ideal-width" }
        , idealHeight{ state, "ideal-height" }
        , boxModel{ toType<CommonGuiItem>()->boxModel }
    {
        jassert(getParent() != nullptr);

        if (!flexShrink.exists())
            flexShrink = 1;

        const auto updateParentLayout = [this]() {
            getParent()->layOutChildren();
        };
        order.onValueChange = updateParentLayout;
        flexGrow.onValueChange = updateParentLayout;
        flexShrink.onValueChange = updateParentLayout;
        flexBasis.onValueChange = updateParentLayout;
        alignSelf.onValueChange = updateParentLayout;
    }

    juce::FlexItem::Margin transform(const juce::BorderSize<float>& border)
    {
        return juce::FlexItem::Margin{
            border.getTop(),
            border.getRight(),
            border.getBottom(),
            border.getLeft()
        };
    }

    juce::FlexItem FlexItem::toJuceFlexItem(juce::Rectangle<float> parentContentBounds,
                                            LayoutStrategy strategy) const
    {
        juce::FlexItem flexItem;

        if (strategy == LayoutStrategy::real)
            flexItem.associatedComponent = component.get();

        const auto minBounds = boxModel.getMinimumBounds();
        flexItem.minWidth = minBounds.getWidth();
        flexItem.minHeight = minBounds.getHeight();

        if (state.getParent().getProperty("flex-direction").toString().contains("column"))
        {
            if (!width.isAuto())
            {
                flexItem.width = width.toPixels(strategy == LayoutStrategy::real ? parentContentBounds : juce::Rectangle<float>{});
            }
            else
            {
                if (idealWidth.exists())
                {
                    if (idealWidth.get() < parentContentBounds.getWidth() || strategy == LayoutStrategy::dummy)
                        flexItem.minWidth = juce::jmax(flexItem.minWidth, idealWidth.get());
                    else
                        flexItem.width = parentContentBounds.getWidth();
                }
            }

            if (!height.isAuto())
            {
                flexItem.height = height.toPixels(strategy == LayoutStrategy::real ? parentContentBounds : juce::Rectangle<float>{});
            }
            else
            {
                if (idealHeight.exists())
                {
                    auto property = state["ideal-height"];
                    auto calculateHeight = property.getNativeFunction();

                    if (calculateHeight != nullptr)
                    {
                        juce::var args[] = {
                            strategy == LayoutStrategy::dummy
                                ? juce::jmin(idealWidth.get(), parentContentBounds.getWidth())
                                : juce::jmax(flexItem.width, flexItem.minWidth),
                        };
                        flexItem.minHeight = juce::jmax(flexItem.minHeight,
                                                        static_cast<float>(calculateHeight({ property, args, 1 })));
                    }
                    else
                    {
                        flexItem.minHeight = juce::jmax(flexItem.minHeight,
                                                        static_cast<float>(property));
                    }
                }
            }
        }
        else
        {
            if (!width.isAuto())
                flexItem.width = width.toPixels(strategy == LayoutStrategy::real ? parentContentBounds : juce::Rectangle<float>{});
            else if (idealWidth.exists())
                flexItem.width = idealWidth.get();

            if (!height.isAuto())
            {
                flexItem.height = height.toPixels(strategy == LayoutStrategy::real ? parentContentBounds : juce::Rectangle<float>{});
            }
            else
            {
                if (idealHeight.exists())
                {
                    auto property = state["ideal-height"];
                    auto calculateHeight = property.getNativeFunction();

                    if (calculateHeight != nullptr)
                    {
                        juce::var args[] = {
                            strategy == LayoutStrategy::dummy
                                ? juce::jmin(idealWidth.get(), parentContentBounds.getWidth())
                                : juce::jmax(flexItem.width, flexItem.minWidth),
                        };
                        const auto idealHeightAbsolute = static_cast<float>(calculateHeight({ property, args, 1 }));

                        if (idealHeightAbsolute < parentContentBounds.getHeight() || strategy == LayoutStrategy::dummy)
                            flexItem.minHeight = juce::jmax(flexItem.minHeight, idealHeightAbsolute);
                        else
                            flexItem.height = parentContentBounds.getHeight();
                    }
                    else
                    {
                        flexItem.minHeight = juce::jmax(flexItem.minHeight,
                                                        static_cast<float>(property));
                    }
                }
            }
        }

        flexItem.order = order;
        flexItem.flexGrow = flexGrow;
        flexItem.flexShrink = flexShrink;
        flexItem.flexBasis = flexBasis;
        flexItem.margin = transform(boxModel.getMargin());

        if (strategy == LayoutStrategy::real)
            flexItem.alignSelf = alignSelf;

        return flexItem;
    }
} // namespace jive

#if JIVE_UNIT_TESTS
class FlexItemUnitTest : public juce::UnitTest
{
public:
    FlexItemUnitTest()
        : juce::UnitTest{ "jive::FlexItem", "jive" }
    {
    }

    void runTest() final
    {
        testComponent();
        testOrder();
        testFlexGrow();
        testFlexShrink();
        testFlexBasis();
        testAlignSelf();
        testSize();
        testMargin();
        testRelativeSizes();
    }

private:
    void testComponent()
    {
        beginTest("component");

        jive::Interpreter interpreter;
        juce::ValueTree state{
            "Component",
            {
                { "width", 222 },
                { "height", 333 },
            },
            {
                juce::ValueTree{ "Component" },
            },
        };
        auto parent = interpreter.interpret(state);
        auto& item = *parent->getChildren()[0];
        const auto flexItem = dynamic_cast<jive::GuiItemDecorator&>(item)
                                  .toType<jive::FlexItem>()
                                  ->toJuceFlexItem({}, jive::LayoutStrategy::real);
        expect(flexItem.associatedComponent == item.getComponent().get());
    }

    void testOrder()
    {
        beginTest("order");

        jive::Interpreter interpreter;
        juce::ValueTree state{
            "Component",
            {
                { "width", 222 },
                { "height", 333 },
            },
            {
                juce::ValueTree{ "Component" },
            },
        };
        auto parent = interpreter.interpret(state);
        auto& item = *parent->getChildren()[0];
        auto flexItem = dynamic_cast<jive::GuiItemDecorator&>(item)
                            .toType<jive::FlexItem>()
                            ->toJuceFlexItem({}, jive::LayoutStrategy::real);

        expect(flexItem.order == 0);

        state.getChild(0).setProperty("order", 10, nullptr);
        flexItem = dynamic_cast<jive::GuiItemDecorator&>(item)
                       .toType<jive::FlexItem>()
                       ->toJuceFlexItem({}, jive::LayoutStrategy::real);

        expect(flexItem.order == 10);
    }

    void testFlexGrow()
    {
        beginTest("flex-grow");

        jive::Interpreter interpreter;
        juce::ValueTree state{
            "Component",
            {
                { "width", 222 },
                { "height", 333 },
            },
            {
                juce::ValueTree{ "Component" },
            },
        };
        auto parent = interpreter.interpret(state);
        auto& item = *parent->getChildren()[0];
        auto flexItem = dynamic_cast<jive::GuiItemDecorator&>(item)
                            .toType<jive::FlexItem>()
                            ->toJuceFlexItem({}, jive::LayoutStrategy::real);

        expect(flexItem.flexGrow == 0.f);

        state.getChild(0).setProperty("flex-grow", 5.f, nullptr);
        flexItem = dynamic_cast<jive::GuiItemDecorator&>(item)
                       .toType<jive::FlexItem>()
                       ->toJuceFlexItem({}, jive::LayoutStrategy::real);

        expect(flexItem.flexGrow == 5.f);
    }

    void testFlexShrink()
    {
        beginTest("flex-shrink");

        jive::Interpreter interpreter;
        juce::ValueTree state{
            "Component",
            {
                { "width", 222 },
                { "height", 333 },
            },
            {
                juce::ValueTree{ "Component" },
            },
        };
        auto parent = interpreter.interpret(state);
        auto& item = *parent->getChildren()[0];
        auto flexItem = dynamic_cast<jive::GuiItemDecorator&>(item)
                            .toType<jive::FlexItem>()
                            ->toJuceFlexItem({}, jive::LayoutStrategy::real);

        expect(flexItem.flexShrink == 1.f);

        state.getChild(0).setProperty("flex-shrink", 3.4f, nullptr);
        flexItem = dynamic_cast<jive::GuiItemDecorator&>(item)
                       .toType<jive::FlexItem>()
                       ->toJuceFlexItem({}, jive::LayoutStrategy::real);

        expect(flexItem.flexShrink == 3.4f);
    }

    void testFlexBasis()
    {
        beginTest("flex-basis");

        jive::Interpreter interpreter;
        juce::ValueTree state{
            "Component",
            {
                { "width", 222 },
                { "height", 333 },
            },
            {
                juce::ValueTree{ "Component" },
            },
        };
        auto parent = interpreter.interpret(state);
        auto& item = *parent->getChildren()[0];
        auto flexItem = dynamic_cast<jive::GuiItemDecorator&>(item)
                            .toType<jive::FlexItem>()
                            ->toJuceFlexItem({}, jive::LayoutStrategy::real);

        expect(flexItem.flexBasis == 0.f);

        state.getChild(0).setProperty("flex-basis", 4.f, nullptr);
        flexItem = dynamic_cast<jive::GuiItemDecorator&>(item)
                       .toType<jive::FlexItem>()
                       ->toJuceFlexItem({}, jive::LayoutStrategy::real);

        expect(flexItem.flexBasis == 4.f);
    }

    void testAlignSelf()
    {
        beginTest("align-self");

        jive::Interpreter interpreter;
        juce::ValueTree state{
            "Component",
            {
                { "width", 222 },
                { "height", 333 },
            },
            {
                juce::ValueTree{ "Component" },
            },
        };
        auto parent = interpreter.interpret(state);
        auto& item = *parent->getChildren()[0];
        auto flexItem = dynamic_cast<jive::GuiItemDecorator&>(item)
                            .toType<jive::FlexItem>()
                            ->toJuceFlexItem({}, jive::LayoutStrategy::real);

        expect(flexItem.alignSelf == juce::FlexItem{}.alignSelf);

        state.getChild(0).setProperty("align-self", "centre", nullptr);
        flexItem = dynamic_cast<jive::GuiItemDecorator&>(item)
                       .toType<jive::FlexItem>()
                       ->toJuceFlexItem({}, jive::LayoutStrategy::real);

        expect(flexItem.alignSelf == juce::FlexItem::AlignSelf::center);
    }

    void testSize()
    {
        beginTest("size");

        jive::Interpreter interpreter;
        juce::ValueTree state{
            "Component",
            {
                { "width", 100 },
                { "height", 200 },
            },
            {
                juce::ValueTree{ "Component" },
            },
        };
        auto parent = interpreter.interpret(state);
        auto& item = *parent->getChildren()[0];
        auto flexItem = dynamic_cast<jive::GuiItemDecorator&>(item)
                            .toType<jive::FlexItem>()
                            ->toJuceFlexItem({}, jive::LayoutStrategy::real);
        expectEquals(flexItem.width, juce::FlexItem{}.width);
        expectEquals(flexItem.height, juce::FlexItem{}.height);

        state.getChild(0).setProperty("width", 50.f, nullptr);
        state.getChild(0).setProperty("height", 175.f, nullptr);
        flexItem = dynamic_cast<jive::GuiItemDecorator&>(item)
                       .toType<jive::FlexItem>()
                       ->toJuceFlexItem({}, jive::LayoutStrategy::real);

        expect(flexItem.width == 50.f);
        expect(flexItem.height == 175.f);
    }

    void testMargin()
    {
        beginTest("margin");

        jive::Interpreter interpreter;
        juce::ValueTree state{
            "Component",
            {
                { "width", 100 },
                { "height", 200 },
            },
            {
                juce::ValueTree{ "Component" },
            },
        };
        auto parent = interpreter.interpret(state);
        auto& item = *parent->getChildren()[0];
        auto flexItem = dynamic_cast<jive::GuiItemDecorator&>(item)
                            .toType<jive::FlexItem>()
                            ->toJuceFlexItem({}, jive::LayoutStrategy::real);

        expect(flexItem.margin.top == 0.f);
        expect(flexItem.margin.right == 0.f);
        expect(flexItem.margin.bottom == 0.f);
        expect(flexItem.margin.left == 0.f);

        state.getChild(0).setProperty("margin", "1 2 3 4", nullptr);
        flexItem = dynamic_cast<jive::GuiItemDecorator&>(item)
                       .toType<jive::FlexItem>()
                       ->toJuceFlexItem({}, jive::LayoutStrategy::real);

        expect(flexItem.margin.top == 1.f);
        expect(flexItem.margin.right == 2.f);
        expect(flexItem.margin.bottom == 3.f);
        expect(flexItem.margin.left == 4.f);
    }

    void testRelativeSizes()
    {
        beginTest("relative sizes");

        juce::ValueTree state{
            "Component",
            {
                { "width", 300 },
                { "height", 200 },
                { "display", "flex" },
                { "align-items", "flex-start" },
            },
            {
                juce::ValueTree{
                    "Component",
                    {
                        { "width", "50%" },
                        { "height", "10%" },
                    },
                },
            },
        };
        jive::Interpreter interpreter;
        auto parent = interpreter.interpret(state);
        auto& item = *parent->getChildren()[0];
        expectEquals(jive::BoxModel{ item.state }.getWidth(), 150.0f);
        expectEquals(jive::BoxModel{ item.state }.getHeight(), 20.0f);
    }
};

static FlexItemUnitTest flexItemUnitTest;
#endif
