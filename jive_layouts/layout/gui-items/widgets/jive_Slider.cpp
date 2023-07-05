#include <jive_layouts/jive_layouts.h>

namespace jive
{
    Slider::Slider(std::unique_ptr<GuiItem> itemToDecorate)
        : Slider{ std::move(itemToDecorate), 135.0f, 20.0f }
    {
    }

    Slider::Slider(std::unique_ptr<GuiItem> itemToDecorate, float defaultWidth, float defaultHeight)
        : GuiItemDecorator{ std::move(itemToDecorate) }
        , value{ state, "value" }
        , min{ state, "min" }
        , max{ state, "max" }
        , mid{ state, "mid" }
        , interval{ state, "interval" }
        , orientation{ state, "orientation" }
        , width{ state, "width" }
        , height{ state, "height" }
        , sensitivity{ state, "sensitivity" }
        , isInVelocityMode{ state, "velocity-mode" }
        , velocitySensitivity{ state, "velocity-sensitivity" }
        , velocityThreshold{ state, "velocity-threshold" }
        , velocityOffset{ state, "velocity-offset" }
        , snapToMouse{ state, "snap-to-mouse" }
        , onChange{ state, "on-change" }
    {
        if (!max.exists())
            max = "1.0";
        if (!sensitivity.exists())
            sensitivity = 1.0;
        if (!velocitySensitivity.exists())
            velocitySensitivity = 1.0;
        if (!velocityThreshold.exists())
            velocityThreshold = 1;
        if (!snapToMouse.exists())
            snapToMouse = true;

        min.onValueChange = [this]() {
            updateRange();
        };
        max.onValueChange = [this]() {
            updateRange();
        };
        mid.onValueChange = [this]() {
            updateRange();
        };
        interval.onValueChange = [this]() {
            updateRange();
        };
        updateRange();

        value.onValueChange = [this]() {
            getSlider().setValue(getSlider().getValueFromText(value));
        };
        getSlider().setValue(getSlider().getValueFromText(value));

        orientation.onValueChange = [this]() {
            updateStyle();
        };
        width.onValueChange = [this]() {
            updateStyle();
        };
        height.onValueChange = [this]() {
            updateStyle();
        };
        updateStyle();

        sensitivity.onValueChange = [this]() {
            getSlider().setMouseDragSensitivity(juce::roundToInt(250.0 / sensitivity));
        };
        getSlider().setMouseDragSensitivity(juce::roundToInt(250.0 / sensitivity));

        isInVelocityMode.onValueChange = [this]() {
            getSlider().setVelocityBasedMode(isInVelocityMode);
        };
        getSlider().setVelocityBasedMode(isInVelocityMode);

        velocitySensitivity.onValueChange = [this]() {
            getSlider().setVelocityModeParameters(velocitySensitivity, velocityThreshold, velocityOffset);
        };
        velocityThreshold.onValueChange = [this]() {
            getSlider().setVelocityModeParameters(velocitySensitivity, velocityThreshold, velocityOffset);
        };
        velocityOffset.onValueChange = [this]() {
            getSlider().setVelocityModeParameters(velocitySensitivity, velocityThreshold, velocityOffset);
        };
        getSlider().setVelocityModeParameters(velocitySensitivity, velocityThreshold, velocityOffset);

        snapToMouse.onValueChange = [this]() {
            getSlider().setSliderSnapsToMousePosition(snapToMouse);
        };
        getSlider().setSliderSnapsToMousePosition(snapToMouse);

        getSlider().setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
        getSlider().addListener(this);

        if (width.isAuto())
            width = juce::String{ defaultWidth };
        if (height.isAuto())
            height = juce::String{ defaultHeight };
    }

    bool Slider::isContainer() const
    {
        return false;
    }

    juce::Slider& Slider::getSlider()
    {
        return *dynamic_cast<juce::Slider*>(component.get());
    }

    const juce::Slider& Slider::getSlider() const
    {
        return *dynamic_cast<const juce::Slider*>(component.get());
    }

    void Slider::updateStyle()
    {
        Orientation ori;

        if (orientation.isAuto())
        {
            const BoxModel boxModel{ state };

            if (boxModel.getWidth() < boxModel.getHeight())
                ori = Orientation::vertical;
            else
                ori = Orientation::horizontal;
        }
        else
        {
            ori = orientation.getOr(Orientation::horizontal);
        }

        getSlider().setSliderStyle(getStyleForOrientation(ori));
    }

    void Slider::sliderValueChanged(juce::Slider* slider)
    {
        if (slider != &getSlider())
            return;

        value = getSlider().getTextFromValue(getSlider().getValue());
        onChange.triggerWithoutSelfCallback();
    }

    juce::Slider::SliderStyle Slider::getStyleForOrientation(Orientation ori)
    {
        switch (ori)
        {
        case Orientation::horizontal:
            return juce::Slider::SliderStyle::LinearHorizontal;
        case Orientation::vertical:
            return juce::Slider::SliderStyle::LinearVertical;
        }

        jassertfalse;
        return {};
    }

    void Slider::updateRange()
    {
        auto& slider = getSlider();
        juce::NormalisableRange<double> range{ slider.getValueFromText(min),
                                               slider.getValueFromText(max) };

        if (mid.exists())
            range.setSkewForCentre(slider.getValueFromText(mid));
        if (interval.exists())
            range.interval = slider.getValueFromText(interval);

        slider.setNormalisableRange(range);
    }
} // namespace jive

#if JIVE_UNIT_TESTS
class SliderTest : public juce::UnitTest
{
public:
    SliderTest()
        : juce::UnitTest{ "jive::Slider", "jive" }
    {
    }

    void runTest() final
    {
        testGuiItem();
        testValue();
        testRange();
        testOrientation();
        testSensitivity();
        testVelocityMode();
        testTextBox();
        testAutoSize();
        testEvents();
    }

private:
    std::unique_ptr<jive::Slider> createSlider(juce::ValueTree tree)
    {
        jive::Interpreter interpreter;

        return std::make_unique<jive::Slider>(interpreter.interpret(tree));
    }

    void testGuiItem()
    {
        beginTest("gui-item");

        auto item = createSlider(juce::ValueTree{
            "Slider",
            {
                { "width", 222 },
                { "height", 333 },
            },
        });
        expect(!item->isContainer());
    }

    void testValue()
    {
        beginTest("value");

        {
            juce::ValueTree tree{
                "Slider",
                {
                    { "width", 222 },
                    { "height", 333 },
                },
            };
            auto item = createSlider(tree);
            expectEquals(item->getSlider().getValue(), 0.0);

            tree.setProperty("value", 0.5, nullptr);
            expectEquals(item->getSlider().getValue(), 0.5);

            item->getSlider().valueFromTextFunction = [](const juce::String& text) {
                if (text == "foo")
                    return 1.0;

                return 0.0;
            };
            item->getSlider().textFromValueFunction = [](double value) {
                if (value == 1.0)
                    return "foo";

                return "bar";
            };
            tree.setProperty("value", "foo", nullptr);
            expectEquals(item->getSlider().getValue(), 1.0);

            item->getSlider().setValue(0.0, juce::sendNotificationSync);
            expectEquals(tree["value"].toString(), juce::String{ "bar" });
        }
        {
            juce::ValueTree tree{
                "Slider",
                {
                    { "width", 222 },
                    { "height", 333 },
                    { "value", 0.73 },
                },
            };
            auto item = createSlider(tree);
            expectEquals(item->getSlider().getValue(), 0.73);
        }
    }

    void testRange()
    {
        beginTest("range");

        {
            juce::ValueTree tree{
                "Slider",
                {
                    { "width", 222 },
                    { "height", 333 },
                },
            };
            auto item = createSlider(tree);
            juce::NormalisableRange<double> expectedRange{ 0.0, 1.0 };
            expectEquals(item->getSlider().getMinimum(), expectedRange.start);
            expectEquals(item->getSlider().getMaximum(), expectedRange.end);
            expectEquals(item->getSlider().getSkewFactor(), expectedRange.skew);
            expectEquals(item->getSlider().getInterval(), expectedRange.interval);

            tree.setProperty("min", 0.5, nullptr);
            tree.setProperty("max", 3.9, nullptr);
            tree.setProperty("mid", 1.3, nullptr);
            tree.setProperty("interval", 0.1, nullptr);
            expectedRange = { 0.5, 3.9 };
            expectedRange.setSkewForCentre(1.3);
            expectedRange.interval = 0.1;
            expectEquals(item->getSlider().getMinimum(), expectedRange.start);
            expectEquals(item->getSlider().getMaximum(), expectedRange.end);
            expectEquals(item->getSlider().getSkewFactor(), expectedRange.skew);
            expectEquals(item->getSlider().getInterval(), expectedRange.interval);
        }
        {
            juce::ValueTree tree{
                "Slider",
                {
                    { "width", 222 },
                    { "height", 333 },
                    { "min", 20.0 },
                    { "max", 20000.0 },
                    { "mid", 632.456 },
                    { "interval", 10.0 },
                },
            };
            auto item = createSlider(tree);
            juce::NormalisableRange<double> expectedRange{ 20.0, 20000.0 };
            expectedRange.setSkewForCentre(632.456);
            expectedRange.interval = 10.0;
            expectEquals(item->getSlider().getMinimum(), expectedRange.start);
            expectEquals(item->getSlider().getMaximum(), expectedRange.end);
            expectEquals(item->getSlider().getSkewFactor(), expectedRange.skew);
            expectEquals(item->getSlider().getInterval(), expectedRange.interval);
        }
    }

    void testOrientation()
    {
        beginTest("orientation");

        {
            juce::ValueTree tree{
                "Slider",
                {
                    { "width", 999 },
                    { "height", 333 },
                },
            };
            auto item = createSlider(tree);
            expect(item->getSlider().getSliderStyle() == juce::Slider::LinearHorizontal);

            tree.setProperty("width", 25, nullptr);
            tree.setProperty("height", 150, nullptr);
            expect(item->getSlider().getSliderStyle() == juce::Slider::LinearVertical);

            tree.setProperty("width", 250, nullptr);
            tree.setProperty("height", 15, nullptr);
            expect(item->getSlider().getSliderStyle() == juce::Slider::LinearHorizontal);

            tree.setProperty("orientation", "vertical", nullptr);
            expect(item->getSlider().getSliderStyle() == juce::Slider::LinearVertical);
        }
        {
            juce::ValueTree tree{
                "Slider",
                {
                    { "width", 222 },
                    { "height", 333 },
                    { "orientation", "vertical" },
                },
            };
            auto item = createSlider(tree);
            expect(item->getSlider().getSliderStyle() == juce::Slider::LinearVertical);
        }
    }

    void testSensitivity()
    {
        beginTest("sensitivity");

        {
            juce::ValueTree tree{
                "Slider",
                {
                    { "width", 222 },
                    { "height", 333 },
                },
            };
            auto item = createSlider(tree);
            expectEquals(item->getSlider().getMouseDragSensitivity(), 250);

            tree.setProperty("sensitivity", 2.0, nullptr);
            expectEquals(item->getSlider().getMouseDragSensitivity(), 125);
        }
        {
            juce::ValueTree tree{
                "Slider",
                {
                    { "width", 222 },
                    { "height", 333 },
                    { "sensitivity", 0.639 },
                },
            };
            auto item = createSlider(tree);
            expectEquals(item->getSlider().getMouseDragSensitivity(), 391);
        }
    }

    void testVelocityMode()
    {
        beginTest("velocity");

        {
            juce::ValueTree tree{
                "Slider",
                {
                    { "width", 222 },
                    { "height", 333 },
                },
            };
            auto item = createSlider(tree);
            expect(!item->getSlider().getVelocityBasedMode());
            expectEquals(item->getSlider().getVelocitySensitivity(), 1.0);
            expectEquals(item->getSlider().getVelocityThreshold(), 1);
            expectEquals(item->getSlider().getVelocityOffset(), 0.0);

            tree.setProperty("velocity-mode", true, nullptr);
            expect(item->getSlider().getVelocityBasedMode());

            tree.setProperty("velocity-sensitivity", 2.3, nullptr);
            expectEquals(item->getSlider().getVelocitySensitivity(), 2.3);

            tree.setProperty("velocity-threshold", 7, nullptr);
            expectEquals(item->getSlider().getVelocityThreshold(), 7);

            tree.setProperty("velocity-offset", 3.4, nullptr);
            expectEquals(item->getSlider().getVelocityOffset(), 3.4);
        }
        {
            juce::ValueTree tree{
                "Slider",
                {
                    { "width", 222 },
                    { "height", 333 },
                    { "velocity-mode", true },
                    { "velocity-sensitivity", 0.43 },
                    { "velocity-threshold", 4 },
                    { "velocity-offset", 8.3 },
                },
            };
            auto item = createSlider(tree);
            expect(item->getSlider().getVelocityBasedMode());
            expectEquals(item->getSlider().getVelocitySensitivity(), 0.43);
            expectEquals(item->getSlider().getVelocityThreshold(), 4);
            expectEquals(item->getSlider().getVelocityOffset(), 8.3);
        }
    }

    void testSnapToMouse()
    {
        beginTest("snap-to-mouse");

        {
            juce::ValueTree tree{
                "Slider",
                {
                    { "width", 222 },
                    { "height", 333 },
                },
            };
            auto item = createSlider(tree);
            expect(item->getSlider().getSliderSnapsToMousePosition());

            tree.setProperty("snap-to-mouse", false, nullptr);
            expect(!item->getSlider().getSliderSnapsToMousePosition());
        }
        {
            juce::ValueTree tree{
                "Slider",
                {
                    { "width", 222 },
                    { "height", 333 },
                    { "snap-to-mouse", false },
                },
            };
            auto item = createSlider(tree);
            expect(!item->getSlider().getSliderSnapsToMousePosition());
        }
    }

    void testTextBox()
    {
        beginTest("text-box");

        {
            juce::ValueTree tree{
                "Slider",
                {
                    { "width", 222 },
                    { "height", 333 },
                },
            };
            auto item = createSlider(tree);
            expectEquals(item->getSlider().getTextBoxPosition(), juce::Slider::NoTextBox);
        }
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
                juce::ValueTree{ "Slider" },
            },
        };
        jive::Interpreter interpreter;
        auto parent = interpreter.interpret(parentState);
        auto& item = *parent->getChildren()[0];
        expectEquals(jive::BoxModel{ item.state }.getWidth(), 135.0f);
        expectEquals(jive::BoxModel{ item.state }.getHeight(), 20.0f);

        parentState.getChild(0).setProperty("width", 123.f, nullptr);
        expectEquals(jive::BoxModel{ item.state }.getWidth(), 123.f);

        parentState.getChild(0).setProperty("height", 311.f, nullptr);
        expectEquals(jive::BoxModel{ item.state }.getHeight(), 311.f);
    }

    void testEvents()
    {
        beginTest("events");

        juce::ValueTree parentState{
            "Component",
            {
                { "width", 222 },
                { "height", 333 },
            },
            {
                juce::ValueTree{ "Slider" },
            },
        };
        jive::Interpreter interpreter;
        auto parent = interpreter.interpret(parentState);
        auto& slider = *dynamic_cast<jive::GuiItemDecorator&>(*parent->getChildren()[0])
                            .toType<jive::Slider>();
        jive::Event onChange{ parentState.getChild(0), "on-change" };
        expectEquals(onChange.getAssumedTriggerCount(), 0);

        slider.getSlider().setValue(0.123, juce::sendNotificationSync);
        expectEquals(onChange.getAssumedTriggerCount(), 1);
    }
};

static SliderTest sliderTest;
#endif
