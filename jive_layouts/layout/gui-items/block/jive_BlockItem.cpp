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
            updatePosition();
        };
        y.onValueChange = [this]() {
            centreY.clear();
            updatePosition();
        };
        centreX.onValueChange = [this]() {
            x.clear();
            updatePosition();
        };
        centreY.onValueChange = [this]() {
            y.clear();
            updatePosition();
        };
        updatePosition();
    }

    //==================================================================================================================
    void BlockItem::updatePosition()
    {
        GuiItemDecorator::updatePosition();
        getViewport().setTopLeftPosition(calculatePosition());
    }

    //==================================================================================================================
    int BlockItem::calculateX() const
    {
        if (centreX.exists())
            return juce::roundToInt(centreX.calculatePixelValue() - getBoxModel().getWidth() / 2.f);

        return juce::roundToInt(x.calculatePixelValue());
    }

    int BlockItem::calculateY() const
    {
        if (centreY.exists())
            return juce::roundToInt(centreY.calculatePixelValue() - getBoxModel().getHeight() / 2.f);

        return juce::roundToInt(y.calculatePixelValue());
    }

    juce::Point<int> BlockItem::calculatePosition() const
    {
        return getParent()->getBoxModel().getContentBounds().getPosition().roundToInt()
             + juce::Point<int>{ calculateX(), calculateY() };
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
    std::unique_ptr<jive::BlockContainer> createBlockContainer(juce::ValueTree tree)
    {
        jive::Interpreter interpreter;

        return std::make_unique<jive::BlockContainer>(interpreter.interpret(tree));
    }

    void testPosition()
    {
        beginTest("position");

        {
            juce::ValueTree tree{
                "Component",
                {
                    { "display", "block" },
                    { "border-width", 10 },
                    { "padding", 15 },
                },
                {
                    juce::ValueTree{
                        "Component",
                    },
                },
            };
            const auto container = createBlockContainer(tree);
            const auto item = container->getChild(0);

            expectEquals(item.getViewport().getX(), 25);
            expectEquals(item.getViewport().getY(), 25);

            tree.getChild(0).setProperty("x", 10.4f, nullptr);
            tree.getChild(0).setProperty("y", 20.89f, nullptr);
            expectEquals(item.getViewport().getX(), 35);
            expectEquals(item.getViewport().getY(), 46);
        }
        {
            juce::ValueTree tree{
                "Component",
                {
                    { "display", "block" },
                    { "border-width", 10 },
                    { "padding", 15 },
                },
                {
                    juce::ValueTree{
                        "Component",
                        {
                            { "x", 15 },
                            { "y", 25 },
                        },
                    },
                },
            };
            const auto container = createBlockContainer(tree);
            const auto item = container->getChild(0);
            expectEquals(item.getViewport().getX(), 40);
            expectEquals(item.getViewport().getY(), 50);
        }
        {
            juce::ValueTree tree{
                "Component",
                {
                    { "display", "block" },
                    { "width", 50 },
                    { "height", 60 },
                },
                {
                    juce::ValueTree{
                        "Component",
                        {
                            { "x", "20%" },
                            { "y", "50%" },
                        },
                    },
                }
            };
            jive::Interpreter interpreter;
            auto parent = interpreter.interpret(tree);
            auto child = parent->getChild(0);

            expectEquals(child.getViewport().getX(), 10);
            expectEquals(child.getViewport().getY(), 30);

            tree.getChild(0).setProperty("x", "10%", nullptr);
            tree.getChild(0).setProperty("y", "33.3333333333333%", nullptr);
            expectEquals(child.getViewport().getX(), 5);
            expectEquals(child.getViewport().getY(), 20);
        }
    }

    void testCentre()
    {
        beginTest("centre");

        {
            juce::ValueTree tree{
                "Component",
                {
                    { "display", "block" },
                },
                {
                    juce::ValueTree{
                        "Component",
                        {
                            { "width", 50 },
                            { "height", 50 },
                        },
                    },
                },
            };
            const auto container = createBlockContainer(tree);
            const auto item = container->getChild(0);

            expectEquals(item.getViewport().getBounds().getCentreX(), 25);
            expectEquals(item.getViewport().getBounds().getCentreY(), 25);

            tree.getChild(0).setProperty("centre-x", 12.3f, nullptr);
            tree.getChild(0).setProperty("centre-y", 98.7f, nullptr);

            expectEquals(item.getViewport().getBounds().getCentreX(), 12);
            expectEquals(item.getViewport().getBounds().getCentreY(), 99);
        }
        {
            juce::ValueTree tree{
                "Component",
                {
                    { "display", "block" },
                },
                {
                    juce::ValueTree{
                        "Component",
                        {
                            { "width", 50 },
                            { "height", 50 },
                            { "centre-x", 85 },
                            { "centre-y", 43 },
                        },
                    },
                },
            };
            const auto container = createBlockContainer(tree);
            const auto item = container->getChild(0);

            expect(item.getViewport().getBounds().getCentreX() == 85);
            expect(item.getViewport().getBounds().getCentreY() == 43);

            tree.getChild(0).setProperty("x", 66, nullptr);
            expectEquals(item.getViewport().getX(), 66);

            tree.getChild(0).setProperty("centre-x", 44, nullptr);
            expectEquals(item.getViewport().getBounds().getCentreX(), 44);
        }
        {
            juce::ValueTree tree{
                "Component",
                {
                    { "display", "block" },
                    { "width", 100 },
                    { "height", 250 },
                },
                {
                    juce::ValueTree{
                        "Component",
                        {
                            { "x", "1%" },
                            { "y", "37.3%" },
                        },
                    },
                },
            };
            jive::Interpreter interpreter;
            auto parent = interpreter.interpret(tree);
            auto child = parent->getChild(0);

            expectEquals(child.getViewport().getX(), 1);
            expectEquals(child.getViewport().getY(), 93);

            tree.getChild(0).setProperty("x", "97.8%", nullptr);
            tree.getChild(0).setProperty("y", "10%", nullptr);
            expectEquals(child.getViewport().getX(), 98);
            expectEquals(child.getViewport().getY(), 25);
        }
    }

    void testSize()
    {
        beginTest("size");

        juce::ValueTree tree{
            "Component",
            {
                { "display", "block" },
            },
            {
                juce::ValueTree{
                    "Component",
                },
            },
        };
        const auto container = createBlockContainer(tree);
        const auto item = container->getChild(0);

        expectEquals(item.getViewport().getWidth(), 0);
        expectEquals(item.getViewport().getHeight(), 0);

        tree.getChild(0).setProperty("width", 10.4f, nullptr);
        tree.getChild(0).setProperty("height", 20.89f, nullptr);
        expectEquals(item.getViewport().getWidth(), 10);
        expectEquals(item.getViewport().getHeight(), 21);
    }
};

static BlockItemTest blockItemTest;
#endif
