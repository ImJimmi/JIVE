#include <jive_layouts/jive_layouts.h>

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    BlockContainer::BlockContainer(std::unique_ptr<GuiItem> itemToDecorate)
        : ContainerItem{ std::move(itemToDecorate) }
    {
        jassert(state.hasProperty("display"));
        jassert(state["display"] == juce::VariantConverter<Display>::toVar(Display::block));
    }

    //==================================================================================================================
    void BlockContainer::layOutChildren()
    {
        for (auto child : getChildren())
        {
            auto& blockItem = *dynamic_cast<GuiItemDecorator&>(*child).toType<BlockItem>();
            child->getComponent()->setBounds(blockItem.calculateBounds());
        }
    }

    //==================================================================================================================
    juce::Rectangle<float> BlockContainer::calculateIdealSize(juce::Rectangle<float>) const
    {
        return { 0.0f, 0.0f };
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

        juce::ValueTree state{
            "Component",
            {
                { "width", 222 },
                { "height", 333 },
                { "display", "block" },
            },
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
        jive::Interpreter interpreter;
        auto item = interpreter.interpret(state);
        expectEquals(item->getChildren()[0]->getComponent()->getX(), 111);
        expectEquals(item->getChildren()[0]->getComponent()->getHeight(), 33);

        state.setProperty("width", 300, nullptr);
        expectEquals(item->getChildren()[0]->getComponent()->getX(), 150);

        state.setProperty("height", 100, nullptr);
        expectEquals(item->getChildren()[0]->getComponent()->getHeight(), 10);
    }
};

static BlockContainerTest blockContainerTest;
#endif
