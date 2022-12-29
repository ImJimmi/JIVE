#include <jive_layouts/jive_layouts.h>

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    Knob::Knob(std::unique_ptr<GuiItem> itemToDecorate)
        : Slider{ std::move(itemToDecorate), 55.0f, 55.0f }
    {
        updateStyle();
    }

    //==============================================================================================================
    juce::Slider::SliderStyle Knob::getStyleForOrientation(Orientation /* orientation */)
    {
        return juce::Slider::RotaryVerticalDrag;
    }
} // namespace jive

//======================================================================================================================
#if JIVE_UNIT_TESTS
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
    std::unique_ptr<jive::Knob> createKnob(juce::ValueTree tree) const
    {
        jive::Interpreter interpreter;

        return std::make_unique<jive::Knob>(interpreter.interpret(tree));
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
        auto item = createKnob(tree);
        expectEquals(item->getSlider().getSliderStyle(), juce::Slider::RotaryVerticalDrag);

        tree.setProperty("orientation", "vertical", nullptr);
        expectEquals(item->getSlider().getSliderStyle(), juce::Slider::RotaryVerticalDrag);
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
        jive::Interpreter interpreter;
        auto parent = interpreter.interpret(parentState);
        auto& item = parent->getChild(0);
        expectEquals(item.boxModel.getWidth(), 55.0f);
        expectEquals(item.boxModel.getHeight(), 55.0f);

        parentState.getChild(0).setProperty("width", 100.0f, nullptr);
        expectEquals(item.boxModel.getWidth(), 100.0f);

        parentState.getChild(0).setProperty("height", 78.0f, nullptr);
        expectEquals(item.boxModel.getHeight(), 78.0f);
    }
};

static KnobTest knobTest;
#endif
