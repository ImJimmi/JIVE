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
    int BlockItem::calculateX() const
    {
        if (centreX.exists())
        {
            auto length = centreX.get();
            length.setCorrespondingGuiItem(*item);

            return juce::roundToInt(length - getWidth() / 2.f);
        }

        auto length = x.get();
        length.setCorrespondingGuiItem(*item);

        return juce::roundToInt(length);
    }

    int BlockItem::calculateY() const
    {
        if (centreY.exists())
        {
            auto length = centreY.get();
            length.setCorrespondingGuiItem(*item);

            return juce::roundToInt(length - getHeight() / 2.f);
        }

        auto length = y.get();
        length.setCorrespondingGuiItem(*item);

        return juce::roundToInt(length);
    }

    juce::Point<int> BlockItem::calculatePosition() const
    {
        return { calculateX(), calculateY() };
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
        {
            juce::ValueTree parentTree{
                "Component",
                {
                    { "width", 50 },
                    { "height", 60 },
                },
            };
            auto parent = std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(),
                                                          parentTree);

            juce::ValueTree childTree{
                "Component",
                {
                    { "x", "20%" },
                    { "y", "50%" },
                }
            };
            auto child = std::make_unique<jive::BlockItem>(std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(),
                                                                                           childTree,
                                                                                           parent.get()));

            expectEquals(child->getComponent().getX(), 10);
            expectEquals(child->getComponent().getY(), 30);

            childTree.setProperty("x", "10%", nullptr);
            childTree.setProperty("y", "33.3333333333333%", nullptr);
            expectEquals(child->getComponent().getX(), 5);
            expectEquals(child->getComponent().getY(), 20);
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
        {
            juce::ValueTree parentTree{
                "Component",
                {
                    { "width", 100 },
                    { "height", 250 },
                },
            };
            auto parent = std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(),
                                                          parentTree);

            juce::ValueTree childTree{
                "Component",
                {
                    { "x", "1%" },
                    { "y", "37.3%" },
                }
            };
            auto child = std::make_unique<jive::BlockItem>(std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(),
                                                                                           childTree,
                                                                                           parent.get()));

            expectEquals(child->getComponent().getX(), 1);
            expectEquals(child->getComponent().getY(), 93);

            childTree.setProperty("x", "97.8%", nullptr);
            childTree.setProperty("y", "10%", nullptr);
            expectEquals(child->getComponent().getX(), 98);
            expectEquals(child->getComponent().getY(), 25);
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
