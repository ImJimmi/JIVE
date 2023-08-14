#include <jive_layouts/jive_layouts.h>

struct ButtonWithNestedIconAndText : public juce::UnitTest
{
    ButtonWithNestedIconAndText()
        : juce::UnitTest{ "Button with nested icon and text", "jive" }
    {
    }

    void runTest() final
    {
        juce::ValueTree state{
            "Component",
            {
                { "align-items", "flex-start" },
                { "width", 9999 },
                { "height", 9999 },
            },
            {
                juce::ValueTree{
                    "Component",
                    {
                        { "justify-content", "space-between" },
                        { "flex-direction", "row" },
                    },
                    {
                        juce::ValueTree{
                            "Button",
                            {
                                { "flex-direction", "row" },
                                { "justify-content", "flex-start" },
                                { "margin", "0 0 5 0" },
                                { "padding", 0 },
                            },
                            {
                                juce::ValueTree{
                                    "svg",
                                    {
                                        { "viewBox", "0 0 48 48" },
                                        { "height", "100%" },
                                    },
                                    {
                                        juce::ValueTree{ "rect", { { "width", 48 }, { "height", 48 } } },
                                    },
                                },
                                juce::ValueTree{
                                    "Text",
                                    {
                                        { "text", "Back" },
                                        { "margin", "0 0 0 10" },
                                    },
                                },
                            },
                        },
                    },
                },
            },
        };
        jive::Interpreter interpreter;
        auto container = interpreter.interpret(state);

        auto* button = container->getChildren()[0]->getChildren()[0];
        auto& icon = *button->getComponent()->getChildComponent(0);
        beginTest("icon has x-position 0");
        expectEquals(icon.getX(), 0);

        auto& text = *button->getComponent()->getChildComponent(1);
        beginTest("text is 10px further right than icon");
        expectEquals(text.getX(), icon.getRight() + 10);
    }
};

static ButtonWithNestedIconAndText buttonWithNestedIconAndText;
