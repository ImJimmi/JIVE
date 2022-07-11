#include <jive_layouts/jive_layouts.h>

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    Knob::Knob(std::unique_ptr<GuiItem> itemToDecorate)
        : Slider{ std::move(itemToDecorate) }
    {
        updateStyle();
    }

    //==================================================================================================================
    float Knob::getWidth() const
    {
        if (!hasAutoWidth())
            return Slider::getWidth();

        return defaultSize;
    }

    float Knob::getHeight() const
    {
        if (!hasAutoHeight())
            return Slider::getHeight();

        return defaultSize;
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

        juce::ValueTree tree{ "Knob" };
        auto item = createKnob(tree);
        expectEquals(item->getWidth(), 55.0f);
        expectEquals(item->getHeight(), 55.0f);

        tree.setProperty("width", 100.0f, nullptr);
        expectEquals(item->getWidth(), 100.0f);

        tree.setProperty("height", 78.0f, nullptr);
        expectEquals(item->getHeight(), 78.0f);
    }
};

static KnobTest knobTest;
#endif
