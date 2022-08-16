#include <jive_layouts/jive_layouts.h>

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    FlexContainer::FlexContainer(std::unique_ptr<GuiItem> itemToDecorate)
        : GuiItemDecorator{ std::move(itemToDecorate) }
        , flexDirection{ tree, "flex-direction", juce::FlexBox::Direction::column }
        , flexWrap{ tree, "flex-wrap", juce::FlexBox::Wrap::noWrap }
        , flexJustifyContent{ tree, "justify-content", juce::FlexBox::JustifyContent::flexStart }
        , flexAlignItems{ tree, "align-items", juce::FlexBox::AlignItems::stretch }
        , flexAlignContent{ tree, "align-content", juce::FlexBox::AlignContent::stretch }
    {
        jassert(tree.hasProperty("display"));
        jassert(tree["display"] == juce::VariantConverter<Display>::toVar(Display::flex));

        flexDirection.onValueChange = [this]() {
            updateLayout();
        };
        flexWrap.onValueChange = [this]() {
            updateLayout();
        };
        flexJustifyContent.onValueChange = [this]() {
            updateLayout();
        };
        flexAlignItems.onValueChange = [this]() {
            updateLayout();
        };
        flexAlignContent.onValueChange = [this]() {
            updateLayout();
        };
    }

    //==================================================================================================================
    float FlexContainer::getWidth() const
    {
        if (hasAutoWidth())
            return getMinimumContentWidth();

        return GuiItemDecorator::getWidth();
    }

    float FlexContainer::getHeight() const
    {
        if (hasAutoHeight())
            return getMinimumContentHeight();

        return GuiItemDecorator::getHeight();
    }

    //==================================================================================================================
    void FlexContainer::updateLayout()
    {
        auto flex = static_cast<juce::FlexBox>(*this);
        flex.performLayout(getBoxModel().getContentBounds());
    }

    //==================================================================================================================
    FlexContainer::operator juce::FlexBox()
    {
        return getFlexBox();
    }

    //==================================================================================================================
    void appendChildren(GuiItem& container, juce::FlexBox& flex)
    {
        for (auto& child : container)
        {
            if (auto* const decoratedItem = dynamic_cast<GuiItemDecorator*>(&child))
            {
                if (auto* const flexItem = decoratedItem->toType<FlexItem>())
                    flex.items.add(*flexItem);
            }
        }
    }

    juce::FlexBox FlexContainer::getFlexBox()
    {
        juce::FlexBox flex;

        flex.flexDirection = flexDirection;
        flex.flexWrap = flexWrap;
        flex.justifyContent = flexJustifyContent;
        flex.alignItems = flexAlignItems;
        flex.alignContent = flexAlignContent;

        appendChildren(*this, flex);

        return flex;
    }

    juce::FlexBox FlexContainer::getFlexBoxWithDummyItems() const
    {
        auto flex = const_cast<FlexContainer*>(this)->getFlexBox();

        for (auto& flexItem : flex.items)
            flexItem.associatedComponent = nullptr;

        return flex;
    }

    float FlexContainer::getMinimumContentWidth() const
    {
        auto flex = getFlexBoxWithDummyItems();

        flex.justifyContent = juce::FlexBox::JustifyContent::flexStart;
        flex.alignItems = juce::FlexBox::AlignItems::flexStart;
        flex.alignContent = juce::FlexBox::AlignContent::flexStart;

        flex.performLayout(juce::Rectangle<float>{
            std::numeric_limits<float>::max(),
            std::numeric_limits<float>::max(),
        });

        auto rightOfFarthestItem = -1.0f;

        for (const auto& flexItem : flex.items)
        {
            if (flexItem.currentBounds.getWidth() == 0)
                continue;

            const auto right = flexItem.currentBounds.getRight() + flexItem.margin.right;

            if (right > rightOfFarthestItem)
                rightOfFarthestItem = right;
        }

        return rightOfFarthestItem;
    }

    float FlexContainer::getMinimumContentHeight() const
    {
        auto flex = getFlexBoxWithDummyItems();

        flex.justifyContent = juce::FlexBox::JustifyContent::flexStart;
        flex.alignItems = juce::FlexBox::AlignItems::flexStart;
        flex.alignContent = juce::FlexBox::AlignContent::flexStart;

        flex.performLayout(juce::Rectangle<float>{
            std::numeric_limits<float>::max(),
            std::numeric_limits<float>::max(),
        });

        auto bottomOfLowestItem = -1.0f;

        for (const auto& flexItem : flex.items)
        {
            if (flexItem.currentBounds.getHeight() == 0)
                continue;

            const auto bottom = flexItem.currentBounds.getBottom() + flexItem.margin.bottom;

            if (bottom > bottomOfLowestItem)
                bottomOfLowestItem = bottom;
        }

        return bottomOfLowestItem;
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
    }

private:
    std::unique_ptr<jive::FlexContainer> createFlexContainer(juce::ValueTree tree)
    {
        jive::Interpreter interpreter;

        return std::make_unique<jive::FlexContainer>(interpreter.interpret(tree));
    }

    void testDirection()
    {
        beginTest("direction");

        juce::ValueTree tree{ "Component" };
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

        juce::ValueTree tree{ "Component" };
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

        juce::ValueTree tree{ "Component" };
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

        juce::ValueTree tree{ "Component" };
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

        juce::ValueTree tree{ "Component" };
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

        juce::ValueTree tree{ "Component" };
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
            {},
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
        expectEquals(item->getChild(0).getViewport().getPosition(), juce::Point<int>{ 0, 0 });

        tree.setProperty("padding", "10 20 30 40", nullptr);
        item->getComponent().setSize(220, 330);

        expectEquals(item->getChild(0).getViewport().getPosition(), juce::Point<int>{ 40, 10 });
    }

    void testAutoSize()
    {
        beginTest("auto-size");

        {
            juce::ValueTree tree{
                "Component",
                {
                    { "justify-content", "centre" },
                    { "padding", 10 },
                    { "border-width", 5 },
                    { "margin", 15 },
                },
                {
                    juce::ValueTree{
                        "Component",
                        {
                            { "width", 100 },
                            { "height", 25 },
                            { "padding", 7 },
                            { "border-width", 3 },
                        },
                    },
                    juce::ValueTree{
                        "Component",
                        {
                            { "width", 130 },
                            { "height", 14 },
                            { "margin", 18 },
                        },
                    },
                },
            };
            auto item = createFlexContainer(tree);
            expectEquals(item->getHeight(), 95.0f);
            expectEquals(item->getWidth(), 166.0f);
        }
    }
};

static FlexContainerUnitTest flexContainerUnitTest;
#endif
