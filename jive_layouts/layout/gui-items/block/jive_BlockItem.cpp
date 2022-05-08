#include <jive_layouts/jive_layouts.h>

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    BlockItem::BlockItem(std::unique_ptr<GuiItem> itemToDecorate)
        : GuiItemDecorator{ std::move(itemToDecorate) }
        , x{ tree, "x" }
        , y{ tree, "y" }
    {
        x.onValueChange = [this]() {
            const auto nearestX = juce::roundToInt(x);
            auto& component = getComponent();
            component.setTopLeftPosition(component.getPosition().withX(nearestX));
        };
        y.onValueChange = [this]() {
            const auto nearestY = juce::roundToInt(y);
            auto& component = getComponent();
            component.setTopLeftPosition(component.getPosition().withY(nearestY));
        };
        getComponent().setTopLeftPosition({ juce::roundToInt(x), juce::roundToInt(y) });
    }

    //==================================================================================================================
} // namespace jive

//======================================================================================================================
#if JIVE_UNIT_TESTS
class BlockItemTest : public juce::UnitTest
{
public:
    BlockItemTest()
        : juce::UnitTest{ "jive::BlockItem", "jive" }
    {
    }

    void runTest() override
    {
        testPosition();
    }

private:
    std::unique_ptr<jive::BlockItem> createBlockItem(juce::ValueTree tree)
    {
        return std::make_unique<jive::BlockItem>(std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(),
                                                                                 tree));
    }

    void testPosition()
    {
        beginTest("position");

        {
            juce::ValueTree tree{ "Component" };
            const auto item = createBlockItem(tree);

            expect(item->getComponent().getX() == 0);
            expect(item->getComponent().getY() == 0);

            tree.setProperty("x", 10.4f, nullptr);
            tree.setProperty("y", 20.89f, nullptr);
            expect(item->getComponent().getX() == 10);
            expect(item->getComponent().getY() == 21);
        }
        {
            juce::ValueTree tree{
                "Component",
                {
                    { "x", 15 },
                    { "y", 25 },
                },
            };
            const auto item = createBlockItem(tree);

            expect(item->getComponent().getX() == 15);
            expect(item->getComponent().getY() == 25);
        }
    }
};

static BlockItemTest blockItemTest;
#endif
