#include <jive_layouts/jive_layouts.h>

namespace jive
{
    BlockItem::BlockItem(std::unique_ptr<GuiItem> itemToDecorate)
        : GuiItemDecorator{ std::move(itemToDecorate) }
        , x{ state, "x" }
        , y{ state, "y" }
        , centreX{ state, "centre-x" }
        , centreY{ state, "centre-y" }
        , width{ state, "width" }
        , height{ state, "height" }
        , boxModel{ toType<CommonGuiItem>()->boxModel }
    {
        jassert(getParent() != nullptr);

        x.onValueChange = [this]() {
            centreX.clear();
            getComponent()->setBounds(calculateBounds());
        };
        y.onValueChange = [this]() {
            centreY.clear();
            getComponent()->setBounds(calculateBounds());
        };
        centreX.onValueChange = [this]() {
            x.clear();
            getComponent()->setBounds(calculateBounds());
        };
        centreY.onValueChange = [this]() {
            y.clear();
            getComponent()->setBounds(calculateBounds());
        };
        getComponent()->setBounds(calculateBounds());
    }

    int BlockItem::calculateX() const
    {
        const auto parentContentBounds = BoxModel{ state.getParent() }.getContentBounds();

        if (centreX.exists())
            return juce::roundToInt(centreX.toPixels(parentContentBounds) - boxModel.getWidth() / 2.f);

        return juce::roundToInt(x.toPixels(parentContentBounds));
    }

    int BlockItem::calculateY() const
    {
        const auto parentContentBounds = BoxModel{ state.getParent() }.getContentBounds();

        if (centreY.exists())
            return juce::roundToInt(centreY.toPixels(parentContentBounds) - boxModel.getHeight() / 2.f);

        return juce::roundToInt(y.toPixels(parentContentBounds));
    }

    juce::Rectangle<int> BlockItem::calculateBounds() const
    {
        juce::Rectangle<int> bounds;
        const auto& parentBoxModel = dynamic_cast<const GuiItemDecorator*>(getParent())->toType<CommonGuiItem>()->boxModel;
        const auto parentBounds = parentBoxModel.getContentBounds();

        if (!width.isAuto())
            bounds.setWidth(juce::roundToInt(width.toPixels(parentBounds)));
        if (!height.isAuto())
            bounds.setHeight(juce::roundToInt(height.toPixels(parentBounds)));

        return bounds.withPosition(parentBoxModel
                                       .getContentBounds()
                                       .getPosition()
                                       .roundToInt()
                                   + juce::Point{
                                       calculateX(),
                                       calculateY(),
                                   });
    }
} // namespace jive

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
    void testPosition()
    {
        beginTest("position");

        {
            juce::ValueTree parentState{
                "Component",
                {
                    { "width", 222 },
                    { "height", 333 },
                    { "display", "block" },
                    { "border-width", 10 },
                    { "padding", 15 },
                },
                {
                    juce::ValueTree{ "Component" },
                },
            };
            jive::Interpreter interpreter;
            const auto parent = interpreter.interpret(parentState);
            auto& item = *parent->getChildren()[0];
            expectEquals(item.getComponent()->getX(), 25);
            expectEquals(item.getComponent()->getY(), 25);

            parentState.getChild(0).setProperty("x", 10.4f, nullptr);
            parentState.getChild(0).setProperty("y", 20.89f, nullptr);
            expectEquals(item.getComponent()->getX(), 35);
            expectEquals(item.getComponent()->getY(), 46);
        }
        {
            juce::ValueTree parentState{
                "Component",
                {
                    { "width", 222 },
                    { "height", 333 },
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
            jive::Interpreter interpreter;
            const auto parent = interpreter.interpret(parentState);
            auto& item = *parent->getChildren()[0];
            expectEquals(item.getComponent()->getX(), 40);
            expectEquals(item.getComponent()->getY(), 50);
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
            auto& child = *parent->getChildren()[0];

            expectEquals(child.getComponent()->getX(), 10);
            expectEquals(child.getComponent()->getY(), 30);

            tree.getChild(0).setProperty("x", "10%", nullptr);
            tree.getChild(0).setProperty("y", "33.3333333333333%", nullptr);
            expectEquals(child.getComponent()->getX(), 5);
            expectEquals(child.getComponent()->getY(), 20);
        }
    }

    void testCentre()
    {
        beginTest("centre");

        {
            juce::ValueTree parentState{
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
                            { "width", 50 },
                            { "height", 50 },
                        },
                    },
                },
            };
            jive::Interpreter interpreter;
            const auto parent = interpreter.interpret(parentState);
            auto& item = *parent->getChildren()[0];
            expectEquals(item.getComponent()->getBounds().getCentreX(), 25);
            expectEquals(item.getComponent()->getBounds().getCentreY(), 25);

            parentState.getChild(0).setProperty("centre-x", 12.3f, nullptr);
            parentState.getChild(0).setProperty("centre-y", 98.7f, nullptr);
            expectEquals(item.getComponent()->getBounds().getCentreX(), 12);
            expectEquals(item.getComponent()->getBounds().getCentreY(), 99);
        }
        {
            juce::ValueTree parentState{
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
                            { "width", 50 },
                            { "height", 50 },
                            { "centre-x", 85 },
                            { "centre-y", 43 },
                        },
                    },
                },
            };
            jive::Interpreter interpreter;
            const auto parent = interpreter.interpret(parentState);
            auto& item = *parent->getChildren()[0];
            expect(item.getComponent()->getBounds().getCentreX() == 85);
            expect(item.getComponent()->getBounds().getCentreY() == 43);

            parentState.getChild(0).setProperty("x", 66, nullptr);
            expectEquals(item.getComponent()->getX(), 66);

            parentState.getChild(0).setProperty("centre-x", 44, nullptr);
            expectEquals(item.getComponent()->getBounds().getCentreX(), 44);
        }
        {
            juce::ValueTree parentState{
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
            const auto parent = interpreter.interpret(parentState);
            auto& item = *parent->getChildren()[0];
            expectEquals(item.getComponent()->getX(), 1);
            expectEquals(item.getComponent()->getY(), 93);

            parentState.getChild(0).setProperty("x", "97.8%", nullptr);
            parentState.getChild(0).setProperty("y", "10%", nullptr);
            expectEquals(item.getComponent()->getX(), 98);
            expectEquals(item.getComponent()->getY(), 25);
        }
    }

    void testSize()
    {
        beginTest("size");

        juce::ValueTree parentState{
            "Component",
            {
                { "width", 222 },
                { "height", 333 },
                { "display", "block" },
            },
            {
                juce::ValueTree{
                    "Component",
                },
            },
        };
        jive::Interpreter interpreter;
        const auto parent = interpreter.interpret(parentState);
        auto& item = *parent->getChildren()[0];
        expectEquals(item.getComponent()->getWidth(), 0);
        expectEquals(item.getComponent()->getHeight(), 0);

        parentState.getChild(0).setProperty("width", 10.4f, nullptr);
        expectEquals(item.getComponent()->getWidth(), 10);

        parentState.getChild(0).setProperty("height", 20.89f, nullptr);
        expectEquals(item.getComponent()->getHeight(), 21);
    }
};

static BlockItemTest blockItemTest;
#endif
