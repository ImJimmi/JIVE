#include "jive_BlockItem.h"

#include <jive_layouts/layout/gui-items/jive_CommonGuiItem.h>
#include <jive_layouts/layout/gui-items/jive_GuiItem.h>

namespace jive
{
    BlockItem::BlockItem(std::unique_ptr<GuiItem> itemToDecorate)
        : ContainerItem::Child{ std::move(itemToDecorate) }
        , x{ state, ids::x() }
        , y{ state, ids::y() }
        , anchor{ state, ids::anchor() }
        , width{ state, ids::width() }
        , height{ state, ids::height() }
    {
        jassert(getParent() != nullptr);

        const auto updateBounds = [this] {
            getComponent()->setBounds(calculateBounds());
        };

        x.onValueChange = updateBounds;
        x.onTransitionProgressed = updateBounds;
        y.onValueChange = updateBounds;
        y.onTransitionProgressed = updateBounds;
        anchor.onValueChange = updateBounds;
        anchor.onTransitionProgressed = updateBounds;
        width.onValueChange = updateBounds;
        width.onTransitionProgressed = updateBounds;
        height.onValueChange = updateBounds;
        height.onTransitionProgressed = updateBounds;

        updateBounds();

        state.addListener(this);
    }

    BlockItem::~BlockItem()
    {
        state.removeListener(this);
    }

    void BlockItem::valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property)
    {
        forwardPropertyChange(tree, property, x, y, anchor, width, height);
    }

    int BlockItem::calculateX(float itemWidth) const
    {
        const auto parentContentBounds = boxModel(*getParent()).getContentBounds();

        return juce::roundToInt(
            x
                .calculateCurrent()
                .toPixels(parentContentBounds.getWidth())
            - anchor
                  .calculateCurrent()
                  .x
                  .toPixels(itemWidth));
    }

    int BlockItem::calculateY(float itemHeight) const
    {
        const auto parentContentBounds = boxModel(*getParent()).getContentBounds();

        return juce::roundToInt(
            y
                .calculateCurrent()
                .toPixels(parentContentBounds.getHeight())
            - anchor
                  .calculateCurrent()
                  .y
                  .toPixels(itemHeight));
    }

    juce::Rectangle<int> BlockItem::calculateBounds() const
    {
        juce::Rectangle<int> bounds;
        const auto& parentBoxModel = boxModel(*getParent());
        const auto parentBounds = parentBoxModel.getContentBounds();

        // The size is calculated here rather than taken from the box model, as
        // the box model may not have seen a change to the width or height yet.
        auto itemWidth = boxModel(*this).getWidth();
        auto itemHeight = boxModel(*this).getHeight();

        if (!width.isAuto())
        {
            itemWidth = width.calculateCurrent().toPixels(parentBounds.getWidth());
            bounds.setWidth(juce::roundToInt(itemWidth));
        }
        if (!height.isAuto())
        {
            itemHeight = height.calculateCurrent().toPixels(parentBounds.getHeight());
            bounds.setHeight(juce::roundToInt(itemHeight));
        }

        return bounds.withPosition(parentBoxModel
                                       .getContentBounds()
                                       .getPosition()
                                       .roundToInt()
                                   + juce::Point{
                                       calculateX(itemWidth),
                                       calculateY(itemHeight),
                                   });
    }
} // namespace jive

#if JIVE_UNIT_TESTS
    #include <jive_layouts/layout/interpreter/jive_Interpreter.h>

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
        testAnchor();
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

    void testAnchor()
    {
        beginTest("anchor");

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
                            { "height", 40 },
                            { "x", 100 },
                            { "y", 100 },
                        },
                    },
                },
            };
            jive::Interpreter interpreter;
            const auto parent = interpreter.interpret(parentState);
            auto& item = *parent->getChildren()[0];
            expectEquals(item.getComponent()->getBounds(), juce::Rectangle{ 100, 100, 50, 40 });

            parentState.getChild(0).setProperty("anchor", "centred", nullptr);
            expectEquals(item.getComponent()->getBounds(), juce::Rectangle{ 75, 80, 50, 40 });

            parentState.getChild(0).setProperty("anchor", "bottom-right", nullptr);
            expectEquals(item.getComponent()->getBounds(), juce::Rectangle{ 50, 60, 50, 40 });

            parentState.getChild(0).setProperty("anchor", "centred-top", nullptr);
            expectEquals(item.getComponent()->getBounds(), juce::Rectangle{ 75, 100, 50, 40 });

            parentState.getChild(0).setProperty("anchor", "10px 50%", nullptr);
            expectEquals(item.getComponent()->getBounds(), juce::Rectangle{ 90, 80, 50, 40 });

            parentState.getChild(0).removeProperty("anchor", nullptr);
            expectEquals(item.getComponent()->getBounds(), juce::Rectangle{ 100, 100, 50, 40 });
        }
        {
            juce::ValueTree parentState{
                "Component",
                {
                    { "width", 222 },
                    { "height", 334 },
                    { "display", "block" },
                    { "padding", 10 },
                },
                {
                    juce::ValueTree{
                        "Component",
                        {
                            { "width", 50 },
                            { "height", 50 },
                            { "x", "50%" },
                            { "y", "50%" },
                            { "anchor", "centred" },
                        },
                    },
                },
            };
            jive::Interpreter interpreter;
            const auto parent = interpreter.interpret(parentState);
            auto& item = *parent->getChildren()[0];
            expectEquals(item.getComponent()->getBounds().getCentre(), juce::Point{ 111, 167 });
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

        parentState.getChild(0).setProperty("width", "50%", nullptr);
        expectEquals(item.getComponent()->getWidth(), 111);

        parentState.getChild(0).setProperty("height", "10%", nullptr);
        expectEquals(item.getComponent()->getHeight(), 33);

        beginTest("a centred item stays centred when its size changes");

        parentState.getChild(0).setProperty("x", "50%", nullptr);
        parentState.getChild(0).setProperty("y", 100, nullptr);
        parentState.getChild(0).setProperty("anchor", "centred", nullptr);
        expectEquals(item.getComponent()->getBounds(), juce::Rectangle{ 56, 83, 111, 33 });

        parentState.getChild(0).setProperty("width", 20, nullptr);
        parentState.getChild(0).setProperty("height", "20%", nullptr);
        expectEquals(item.getComponent()->getBounds(), juce::Rectangle{ 101, 67, 20, 67 });
    }
};

static BlockItemTest blockItemTest;
#endif
