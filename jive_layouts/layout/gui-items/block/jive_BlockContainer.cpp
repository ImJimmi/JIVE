#include <jive_layouts/jive_layouts.h>

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    BlockContainer::BlockContainer(std::unique_ptr<GuiItem> itemToDecorate)
        : GuiItemDecorator{ std::move(itemToDecorate) }
    {
        jassert(tree.hasProperty("display"));
        jassert(tree["display"] == juce::VariantConverter<Display>::toVar(Display::block));
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
    std::unique_ptr<jive::BlockContainer> createBlockContainer(juce::ValueTree tree)
    {
        jive::Interpreter interpreter;

        tree.setProperty("display", "block", nullptr);

        return std::make_unique<jive::BlockContainer>(interpreter.interpret(tree));
    }

    void testLayout()
    {
        beginTest("layout");

        juce::ValueTree tree{
            "Component",
            {},
            {
                juce::ValueTree{
                    "Component",
                    {
                        { "x", "50%" },
                        { "height", "10%" },
                    },
                },
            },
        };
        auto item = createBlockContainer(tree);
        expectEquals(item->getChild(0).getComponent().getX(), 0);

        tree.setProperty("width", 300, nullptr);
        tree.setProperty("height", 100, nullptr);

        expectEquals(item->getChild(0).getComponent().getX(), 150);
        expectEquals(item->getChild(0).getComponent().getHeight(), 10);
    }
};

static BlockContainerTest blockContainerTest;
#endif
