#include <jive_layouts/jive_layouts.h>

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    BlockContainer::BlockContainer(std::unique_ptr<GuiItem> itemToDecorate)
        : GuiItemDecorator{ std::move(itemToDecorate) }
    {
    }

    //==================================================================================================================
    void BlockContainer::updateLayout()
    {
        GuiItemDecorator::updateLayout();

        for (auto& item : *this)
        {
            item.updatePosition();
            item.updateSize();
        }
    }
} // namespace jive

#if JIVE_UNIT_TESTS
class BlockContainerTest : public juce::UnitTest
{
public:
    BlockContainerTest()
        : UnitTest{ "jive::BlockContainer", "jive" }
    {
    }

    void runTest() override
    {
        testLayout();
    }

private:
    void testLayout()
    {
        beginTest("layout");

        juce::ValueTree tree{ "Component" };
        auto item = std::make_unique<jive::BlockContainer>(std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(),
                                                                                           tree));
        auto child = std::make_unique<jive::BlockItem>(std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(),
                                                                                       juce::ValueTree{
                                                                                           "Component",
                                                                                           {
                                                                                               { "x", "50%" },
                                                                                               { "height", "10%" },
                                                                                           },
                                                                                       },
                                                                                       item.get()));
        item->addChild(std::move(child));
        item->updateLayout();

        expectEquals(item->getChild(0).getComponent().getX(), 0);

        tree.setProperty("width", 300, nullptr);
        tree.setProperty("height", 100, nullptr);

        expectEquals(item->getChild(0).getComponent().getX(), 150);
        expectEquals(item->getChild(0).getComponent().getHeight(), 10);
    }
};

static BlockContainerTest blockContainerTest;
#endif
