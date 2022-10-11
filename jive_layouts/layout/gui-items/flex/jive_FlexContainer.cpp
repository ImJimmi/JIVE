#include <jive_layouts/jive_layouts.h>

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    FlexContainer::FlexContainer(std::unique_ptr<GuiItem> itemToDecorate)
        : GuiItemDecorator{ std::move(itemToDecorate) }
        , flexDirection{ state, "flex-direction", juce::FlexBox::Direction::column }
        , flexWrap{ state, "flex-wrap", juce::FlexBox::Wrap::noWrap }
        , flexJustifyContent{ state, "justify-content", juce::FlexBox::JustifyContent::flexStart }
        , flexAlignItems{ state, "align-items", juce::FlexBox::AlignItems::stretch }
        , flexAlignContent{ state, "align-content", juce::FlexBox::AlignContent::stretch }
    {
        jassert(state.hasProperty("display"));
        jassert(state["display"] == juce::VariantConverter<Display>::toVar(Display::flex));

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

        if (getNumChildren() > 0)
            updateExplicitSize();
    }

    //==================================================================================================================
    void FlexContainer::updateLayout()
    {
        const auto bounds = boxModel.getContentBounds();

        if (bounds.getWidth() <= 0 || bounds.getHeight() <= 0)
            return;

        auto flex = static_cast<juce::FlexBox>(*this);
        flex.performLayout(bounds);
    }

    //==================================================================================================================
    void FlexContainer::addChild(std::unique_ptr<GuiItem> child)
    {
        GuiItemDecorator::addChild(std::move(child));

        updateExplicitSize();
    }

    //==================================================================================================================
    FlexContainer::operator juce::FlexBox()
    {
        return getFlexBox();
    }

    //==================================================================================================================
    void FlexContainer::componentMovedOrResized(juce::Component& componentThatWasMovedOrResized,
                                                bool wasMoved,
                                                bool wasResized)
    {
        GuiItemDecorator::componentMovedOrResized(componentThatWasMovedOrResized, wasMoved, wasResized);

        if (!wasResized)
            return;

        updateLayout();
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

    float FlexContainer::calculateMinimumContentWidth(juce::FlexBox flex) const
    {
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

    float FlexContainer::calculateMinimumContentHeight(juce::FlexBox flex) const
    {
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

    void FlexContainer::updateExplicitSize()
    {
        auto flex = getFlexBoxWithDummyItems();

        flex.justifyContent = juce::FlexBox::JustifyContent::flexStart;
        flex.alignItems = juce::FlexBox::AlignItems::flexStart;
        flex.alignContent = juce::FlexBox::AlignContent::flexStart;

        flex.performLayout(juce::Rectangle<float>{
            std::numeric_limits<float>::max(),
            std::numeric_limits<float>::max(),
        });

        if (hasAutoWidth())
            boxModel.setWidth(juce::jmax(boxModel.getWidth(), calculateMinimumContentWidth(flex)));

        if (hasAutoHeight())
            boxModel.setHeight(juce::jmax(boxModel.getHeight(), calculateMinimumContentHeight(flex)));

        updateLayout();
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

        return std::unique_ptr<jive::FlexContainer>(dynamic_cast<jive::FlexContainer*>(interpreter.interpret(tree).release()));
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
        expectEquals(item->getChild(0).getComponent().getPosition(), juce::Point<int>{ 0, 0 });

        tree.setProperty("padding", "10 20 30 40", nullptr);
        tree.setProperty("width", 220, nullptr);
        tree.setProperty("height", 330, nullptr);
        expectEquals(item->getChild(0).getComponent().getPosition(), juce::Point<int>{ 40, 10 });
    }

    void testAutoSize()
    {
        beginTest("auto-size");

        {
            juce::ValueTree tree{
                "Component",
                {
                    { "display", "flex" },
                },
            };
            auto item = createFlexContainer(tree);
            expectEquals(item->boxModel.getWidth(), 0.0f);
            expectEquals(item->boxModel.getHeight(), 0.0f);

            tree.appendChild(juce::ValueTree{
                                 "Component",
                                 {
                                     { "width", 43 },
                                     { "height", 84 },
                                 },
                             },
                             nullptr);
            item = createFlexContainer(tree);
            expectEquals(item->boxModel.getWidth(), 43.0f);
            expectEquals(item->boxModel.getHeight(), 84.0f);
            expectEquals(item->getChild(0).boxModel.getWidth(), 43.0f);
            expectEquals(item->getChild(0).boxModel.getHeight(), 84.0f);

            tree.appendChild(juce::ValueTree{
                                 "Component",
                                 {
                                     { "width", 37 },
                                     { "height", 99 },
                                     { "margin", 5 },
                                 },
                             },
                             nullptr);
            item = createFlexContainer(tree);
            expectEquals(item->boxModel.getWidth(), 47.0f);
            expectEquals(item->boxModel.getHeight(), 193.0f);
            expectEquals(item->getChild(0).boxModel.getWidth(), 43.0f);
            expectEquals(item->getChild(0).boxModel.getHeight(), 84.0f);
            expectEquals(item->getChild(1).boxModel.getWidth(), 37.0f);
            expectEquals(item->getChild(1).boxModel.getHeight(), 99.0f);

            tree.setProperty("flex-direction", "row", nullptr);
            item = createFlexContainer(tree);
            expectEquals(item->boxModel.getWidth(), 90.0f);
            expectEquals(item->boxModel.getHeight(), 109.0f);
            expectEquals(item->getChild(0).boxModel.getWidth(), 43.0f);
            expectEquals(item->getChild(0).boxModel.getHeight(), 84.0f);
            expectEquals(item->getChild(1).boxModel.getWidth(), 37.0f);
            expectEquals(item->getChild(1).boxModel.getHeight(), 99.0f);
        }
    }
};

static FlexContainerUnitTest flexContainerUnitTest;
#endif
