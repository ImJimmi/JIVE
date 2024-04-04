#include "jive_Spinner.h"

namespace jive
{
    Spinner::Spinner(std::unique_ptr<GuiItem> itemToDecorate)
        : Slider{ std::move(itemToDecorate), 70.0f, 20.0f }
        , draggable{ state, "draggable" }
    {
        draggable.onValueChange = [this]() {
            getSlider().setIncDecButtonsMode(draggable ? juce::Slider::incDecButtonsDraggable_AutoDirection : juce::Slider::incDecButtonsNotDraggable);
        };
        getSlider().setIncDecButtonsMode(draggable ? juce::Slider::incDecButtonsDraggable_AutoDirection : juce::Slider::incDecButtonsNotDraggable);

        updateStyle();
    }

    juce::Slider::SliderStyle Spinner::getStyleForOrientation(Orientation /* orientation */)
    {
        return juce::Slider::IncDecButtons;
    }
} // namespace jive

#if JIVE_UNIT_TESTS
    #include <jive_layouts/layout/jive_Interpreter.h>

class SpinnerTest : public juce::UnitTest
{
public:
    SpinnerTest()
        : juce::UnitTest{ "jive::Spinner", "jive" }
    {
    }

    void runTest() final
    {
        testOrientation();
        testAutoSize();
    }

private:
    std::unique_ptr<jive::Spinner> createSpinner(juce::ValueTree tree)
    {
        jive::Interpreter interpreter;

        return std::make_unique<jive::Spinner>(interpreter.interpret(tree));
    }

    void testOrientation()
    {
        beginTest("orientation");

        juce::ValueTree tree{
            "Spinner",
            {
                { "width", 222 },
                { "height", 333 },
                { "orientation", "horizontal" },
            },
        };
        auto item = createSpinner(tree);
        expectEquals(item->getSlider().getSliderStyle(), juce::Slider::IncDecButtons);

        tree.setProperty("orientation", "vertical", nullptr);
        expectEquals(item->getSlider().getSliderStyle(), juce::Slider::IncDecButtons);
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
                juce::ValueTree{ "Spinner" },
            },
        };
        jive::Interpreter interpreter;
        auto parent = interpreter.interpret(parentState);
        auto& item = *parent->getChildren()[0];
        const auto& boxModel = jive::boxModel(item);
        expectEquals(boxModel.getWidth(), 70.0f);
        expectEquals(boxModel.getHeight(), 20.0f);

        parentState.getChild(0).setProperty("width", 38.0f, nullptr);
        expectEquals(boxModel.getWidth(), 38.0f);

        parentState.getChild(0).setProperty("height", 73.0f, nullptr);
        expectEquals(boxModel.getHeight(), 73.0f);
    }
};

static SpinnerTest spinnerTest;
#endif
