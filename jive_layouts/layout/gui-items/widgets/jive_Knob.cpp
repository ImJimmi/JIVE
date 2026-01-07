#include "jive_Knob.h"

namespace jive
{
    Knob::Knob(std::unique_ptr<GuiItem> itemToDecorate)
        : Slider{ std::move(itemToDecorate), 55.0f, 55.0f }
    {
        updateStyle();
    }

    juce::Slider::SliderStyle Knob::getStyleForOrientation(Orientation /* orientation */)
    {
        return juce::Slider::RotaryVerticalDrag;
    }
} // namespace jive

#if JIVE_UNIT_TESTS
    #include <jive_layouts/layout/interpreter/jive_Interpreter.h>

class KnobTest : public juce::UnitTest
{
public:
    KnobTest()
        : juce::UnitTest{ "jive::Knob", "jive" }
    {
    }

    void runTest() final
    {
        testOrientation();
        testAutoSize();
    }

private:
    jive::Interpreter interpreter;

    [[nodiscard]] auto& getKnob(jive::GuiItem& item)
    {
        return dynamic_cast<jive::GuiItemDecorator&>(item).toType<jive::Knob>()->getSlider();
    }

    void testOrientation()
    {
        beginTest("orientation");

        juce::ValueTree tree{
            "Knob",
            {
                { "width", 222 },
                { "height", 333 },
                { "orientation", "horizontal" },
            },
        };
        auto item = interpreter.interpret(tree);
        auto& knob = getKnob(*item);
        expectEquals(knob.getSliderStyle(), juce::Slider::RotaryVerticalDrag);

        tree.setProperty("orientation", "vertical", nullptr);
        expectEquals(knob.getSliderStyle(), juce::Slider::RotaryVerticalDrag);
    }

    void testAutoSize()
    {
        beginTest("auto size");

        juce::ValueTree parentState{
            "Component",
            {
                { "width", 222 },
                { "height", 333 },
            },
            {
                juce::ValueTree{ "Knob" },
            },
        };
        auto parent = interpreter.interpret(parentState);
        auto& item = *parent->getChildren()[0];
        const auto& boxModel = jive::boxModel(item);
        expectEquals(boxModel.getWidth(), 55.0f);
        expectEquals(boxModel.getHeight(), 55.0f);

        parentState.getChild(0).setProperty("width", 100.0f, nullptr);
        expectEquals(boxModel.getWidth(), 100.0f);

        parentState.getChild(0).setProperty("height", 78.0f, nullptr);
        expectEquals(boxModel.getHeight(), 78.0f);
    }
};

static KnobTest knobTest;
#endif
