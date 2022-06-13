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
    float FlexContainer::getHeight() const
    {
        if (hasAutoHeight())
        {
            const auto contentHeight = getMinimumContentHeight();
            const auto box = getBoxModel();

            return contentHeight + box.getPadding().getTopAndBottom() + box.getBorder().getTopAndBottom();
        }

        return item->getHeight();
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

    float FlexContainer::getMinimumContentHeight() const
    {
        auto flex = getFlexBoxWithDummyItems();
        flex.performLayout(juce::Rectangle<float>{ 0.f, 0.f, getWidth(), std::numeric_limits<float>::max() });

        auto contentHeight = 0.f;

        for (const auto& flexItem : flex.items)
        {
            if (flexItem.currentBounds.getBottom() > contentHeight)
                contentHeight = flexItem.currentBounds.getBottom();
        }

        return contentHeight;
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
        expect(item->getChild(0).getComponent().getPosition() == juce::Point<int>{ 0, 0 });

        tree.setProperty("padding", "10 20 30 40", nullptr);
        item->getComponent().setSize(220, 330);

        expect(item->getChild(0).getComponent().getPosition() == juce::Point<int>{ 40, 10 });
    }

    void testAutoHeight()
    {
        beginTest("auto-height");

        juce::ValueTree tree{
            "Component",
            {
                { "flex-direction", "row" },
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
                    },
                },
                juce::ValueTree{
                    "Component",
                    {
                        { "width", 130 },
                        { "height", 14 },
                    },
                },
            },
        };
        auto item = createFlexContainer(tree);
        expect(item->getHeight() == 55.f);
    }
};

static FlexContainerUnitTest flexContainerUnitTest;
#endif
