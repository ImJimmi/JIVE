#include <jive_layouts/jive_layouts.h>

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    FlexItem::FlexItem(std::unique_ptr<GuiItem> itemToDecorate)
        : GuiItemDecorator{ std::move(itemToDecorate) }
        , flexItemOrder{ tree, "order" }
        , flexItemGrow{ tree, "flex-grow" }
        , flexItemShrink{ tree, "flex-shrink", 1 }
        , flexItemBasis{ tree, "flex-basis" }
        , flexItemAlignSelf{ tree, "align-self", juce::FlexItem::AlignSelf::autoAlign }
    {
        flexItemOrder.onValueChange = [this]() {
            updateParentLayout();
        };
        flexItemGrow.onValueChange = [this]() {
            updateParentLayout();
        };
        flexItemShrink.onValueChange = [this]() {
            updateParentLayout();
        };
        flexItemBasis.onValueChange = [this]() {
            updateParentLayout();
        };
        flexItemAlignSelf.onValueChange = [this]() {
            updateParentLayout();
        };
    }

    //==================================================================================================================
    juce::FlexItem::Margin boxModelToFlexMargin(const juce::BorderSize<float>& border)
    {
        return juce::FlexItem::Margin{
            border.getTop(),
            border.getRight(),
            border.getBottom(),
            border.getLeft()
        };
    }

    FlexItem::operator juce::FlexItem()
    {
        juce::FlexItem flexItem{ getViewport() };

        flexItem.width = getWidth();
        flexItem.height = getHeight();

        flexItem.order = flexItemOrder;
        flexItem.flexGrow = flexItemGrow;
        flexItem.flexShrink = flexItemShrink;
        flexItem.flexBasis = flexItemBasis;
        flexItem.alignSelf = flexItemAlignSelf;
        flexItem.margin = boxModelToFlexMargin(getBoxModel().getMargin());

        return flexItem;
    }

    //==================================================================================================================
    void FlexItem::updateParentLayout()
    {
        if (auto* container = dynamic_cast<FlexContainer*>(getParent()))
            container->updateLayout();
        else if (getParent() != nullptr)
            jassertfalse;
    }
} // namespace jive

//======================================================================================================================
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
    }

private:
    std::unique_ptr<jive::FlexItem> createFlexItem(juce::ValueTree tree)
    {
        jive::Interpreter interpreter;

        return std::make_unique<jive::FlexItem>(interpreter.interpret(tree));
    }

    void testComponent()
    {
        beginTest("component");

        juce::ValueTree tree{ "Component" };
        auto item = createFlexItem(tree);
        auto flexItem = static_cast<juce::FlexItem>(*item);

        expect(flexItem.associatedComponent == &item->getViewport());
    }

    void testOrder()
    {
        beginTest("order");

        juce::ValueTree tree{ "Component" };
        auto item = createFlexItem(tree);
        auto flexItem = static_cast<juce::FlexItem>(*item);

        expect(flexItem.order == 0);

        tree.setProperty("order", 10, nullptr);
        flexItem = static_cast<juce::FlexItem>(*item);

        expect(flexItem.order == 10);
    }

    void testFlexGrow()
    {
        beginTest("flex-grow");

        juce::ValueTree tree{ "Component" };
        auto item = createFlexItem(tree);
        auto flexItem = static_cast<juce::FlexItem>(*item);

        expect(flexItem.flexGrow == 0.f);

        tree.setProperty("flex-grow", 5.f, nullptr);
        flexItem = static_cast<juce::FlexItem>(*item);

        expect(flexItem.flexGrow == 5.f);
    }

    void testFlexShrink()
    {
        beginTest("flex-shrink");

        juce::ValueTree tree{ "Component" };
        auto item = createFlexItem(tree);
        auto flexItem = static_cast<juce::FlexItem>(*item);

        expect(flexItem.flexShrink == 1.f);

        tree.setProperty("flex-shrink", 3.4f, nullptr);
        flexItem = static_cast<juce::FlexItem>(*item);

        expect(flexItem.flexShrink == 3.4f);
    }

    void testFlexBasis()
    {
        beginTest("flex-basis");

        juce::ValueTree tree{ "Component" };
        auto item = createFlexItem(tree);
        auto flexItem = static_cast<juce::FlexItem>(*item);

        expect(flexItem.flexBasis == 0.f);

        tree.setProperty("flex-basis", 4.f, nullptr);
        flexItem = static_cast<juce::FlexItem>(*item);

        expect(flexItem.flexBasis == 4.f);
    }

    void testAlignSelf()
    {
        beginTest("align-self");

        juce::ValueTree tree{ "Component" };
        auto item = createFlexItem(tree);
        auto flexItem = static_cast<juce::FlexItem>(*item);

        expect(flexItem.alignSelf == juce::FlexItem::AlignSelf::autoAlign);

        tree.setProperty("align-self", "centre", nullptr);
        flexItem = static_cast<juce::FlexItem>(*item);

        expect(flexItem.alignSelf == juce::FlexItem::AlignSelf::center);
    }

    void testSize()
    {
        beginTest("size");

        juce::ValueTree tree{
            "Component",
            { { "width", 100 },
              { "height", 200 } }
        };
        auto item = createFlexItem(tree);
        auto flexItem = static_cast<juce::FlexItem>(*item);

        expect(flexItem.width == 100.f);
        expect(flexItem.height == 200.f);

        tree.setProperty("width", 50.f, nullptr);
        tree.setProperty("height", 175.f, nullptr);
        flexItem = static_cast<juce::FlexItem>(*item);

        expect(flexItem.width == 50.f);
        expect(flexItem.height == 175.f);
    }

    void testMargin()
    {
        beginTest("margin");

        juce::ValueTree tree{ "Component" };
        auto item = createFlexItem(tree);
        auto flexItem = static_cast<juce::FlexItem>(*item);

        expect(flexItem.margin.top == 0.f);
        expect(flexItem.margin.right == 0.f);
        expect(flexItem.margin.bottom == 0.f);
        expect(flexItem.margin.left == 0.f);

        tree.setProperty("margin", "1 2 3 4", nullptr);
        flexItem = static_cast<juce::FlexItem>(*item);

        expect(flexItem.margin.top == 1.f);
        expect(flexItem.margin.right == 2.f);
        expect(flexItem.margin.bottom == 3.f);
        expect(flexItem.margin.left == 4.f);
    }
};

static FlexItemUnitTest flexItemUnitTest;
#endif
