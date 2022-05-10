#include <jive_layouts/jive_layouts.h>

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    BlockItem::BlockItem(std::unique_ptr<GuiItem> itemToDecorate)
        : GuiItemDecorator{ std::move(itemToDecorate) }
        , x{ tree, "x" }
        , y{ tree, "y" }
        , centreX{ tree, "centre-x" }
        , centreY{ tree, "centre-y" }
    {
        x.onValueChange = [this]() {
            centreX.clear();

            const auto nearestX = juce::roundToInt(x);
            auto& component = getComponent();
            component.setTopLeftPosition(component.getPosition().withX(nearestX));
        };
        y.onValueChange = [this]() {
            centreY.clear();

            const auto nearestY = juce::roundToInt(y);
            auto& component = getComponent();
            component.setTopLeftPosition(component.getPosition().withY(nearestY));
        };
        centreX.onValueChange = [this]() {
            x.clear();

            auto& component = getComponent();
            const auto nearestCentreX = juce::roundToInt(centreX);
            const auto centre = component.getBounds().getCentre().withX(nearestCentreX);
            component.setCentrePosition(centre);
        };
        centreY.onValueChange = [this]() {
            y.clear();

            auto& component = getComponent();
            const auto nearestCentreY = juce::roundToInt(centreY);
            const auto centre = component.getBounds().getCentre().withY(nearestCentreY);
            component.setCentrePosition(centre);
        };
        getComponent().setTopLeftPosition(calculatePosition());
    }

    //==================================================================================================================
    float BlockItem::getWidth() const
    {
        if (hasAutoWidth())
            return 0.f;

        return GuiItemDecorator::getWidth();
    }

    float BlockItem::getHeight() const
    {
        if (hasAutoHeight())
            return 0.f;

        return GuiItemDecorator::getHeight();
    }

    //==================================================================================================================
    juce::Point<int> BlockItem::calculatePosition() const
    {
        juce::Point<int> position;

        if (centreX.exists())
            position.x = centreX - getWidth() / 2.f;
        else
            position.x = x;

        if (centreY.exists())
            position.y = centreY - getHeight() / 2.f;
        else
            position.y = y;

        return position;
    }
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
        testCentre();
        testSize();
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

            expectEquals(item->getComponent().getX(), 0);
            expectEquals(item->getComponent().getY(), 0);

            tree.setProperty("x", 10.4f, nullptr);
            tree.setProperty("y", 20.89f, nullptr);
            expectEquals(item->getComponent().getX(), 10);
            expectEquals(item->getComponent().getY(), 21);
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

            expectEquals(item->getComponent().getX(), 15);
            expectEquals(item->getComponent().getY(), 25);
        }
    }

    void testCentre()
    {
        beginTest("centre");

        {
            juce::ValueTree tree{
                "Component",
                {
                    { "width", 50 },
                    { "height", 50 },
                }
            };
            const auto item = createBlockItem(tree);

            expectEquals(item->getComponent().getBounds().getCentreX(), 25);
            expectEquals(item->getComponent().getBounds().getCentreY(), 25);

            tree.setProperty("centre-x", 12.3f, nullptr);
            tree.setProperty("centre-y", 98.7f, nullptr);

            expectEquals(item->getComponent().getBounds().getCentreX(), 12);
            expectEquals(item->getComponent().getBounds().getCentreY(), 99);
        }
        {
            juce::ValueTree tree{
                "Component",
                {
                    { "width", 50 },
                    { "height", 50 },
                    { "centre-x", 85 },
                    { "centre-y", 43 },
                }
            };
            const auto item = createBlockItem(tree);

            expect(item->getComponent().getBounds().getCentreX() == 85);
            expect(item->getComponent().getBounds().getCentreY() == 43);

            tree.setProperty("x", 66, nullptr);
            expectEquals(item->getComponent().getX(), 66);

            tree.setProperty("centre-x", 44, nullptr);
            expectEquals(item->getComponent().getBounds().getCentreX(), 44);
        }
    }

    void testSize()
    {
        beginTest("size");

        juce::ValueTree tree{ "Component" };
        const auto item = createBlockItem(tree);

        expectEquals(item->getWidth(), 0.f);
        expectEquals(item->getHeight(), 0.f);

        tree.setProperty("width", 10.4f, nullptr);
        tree.setProperty("height", 20.89f, nullptr);
        expectEquals(item->getWidth(), 10.f);
        expectEquals(item->getHeight(), 21.f);
    }
};

static BlockItemTest blockItemTest;
#endif
