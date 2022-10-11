#include <jive_layouts/jive_layouts.h>

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    BlockContainer::BlockContainer(std::unique_ptr<GuiItem> itemToDecorate)
        : GuiItemDecorator{ std::move(itemToDecorate) }
    {
        jassert(state.hasProperty("display"));
        jassert(state["display"] == juce::VariantConverter<Display>::toVar(Display::block));
    }

    //==================================================================================================================
    void BlockContainer::updateLayout()
    {
        GuiItemDecorator::updateLayout();

        for (auto& child : *this)
        {
            child.updatePosition();

            BoxModel childBoxModel{ child.state };
            const auto childContentBounds = childBoxModel.getContentBounds();
            child.getComponent()->setSize(juce::roundToInt(childContentBounds.getWidth()),
                                          juce::roundToInt(childContentBounds.getHeight()));
        }
    }

    //==================================================================================================================
    void BlockContainer::componentMovedOrResized(juce::Component& componentThatWasMovedOrResized,
                                                 bool wasMoved,
                                                 bool wasResized)
    {
        GuiItemDecorator::componentMovedOrResized(componentThatWasMovedOrResized, wasMoved, wasResized);

        if (!wasResized)
            return;

        updateLayout();
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
        expectEquals(item->getChild(0).getComponent()->getX(), 0);
        expectEquals(item->getChild(0).getComponent()->getHeight(), 0);

        state.setProperty("width", 300, nullptr);
        expectEquals(item->getChild(0).getComponent()->getX(), 150);

        state.setProperty("height", 100, nullptr);
        expectEquals(item->getChild(0).getComponent()->getHeight(), 10);
    }
};

static BlockContainerTest blockContainerTest;
#endif
