#include "jive_Length.h"

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    const float Length::pixelValueWhenAuto = 0.0f;

    //==================================================================================================================
    float Length::calculatePixelValue() const
    {
        if (isAuto())
            return pixelValueWhenAuto;

        if (isPixels())
            return get().getFloatValue();

        const auto scale = static_cast<double>(get().getFloatValue()) * 0.01;
        return scale * getRelativeParentLength();
    }

    //==================================================================================================================
    bool Length::isAuto() const
    {
        return (!exists()) || get() == "auto";
    }

    bool Length::isPixels() const
    {
        return !isAuto() && !isPercent();
    }

    bool Length::isPercent() const
    {
        return !isAuto() && get().endsWith("%");
    }

    //==================================================================================================================
    double Length::getRelativeParentLength() const
    {
        jassert(tree.getParent().isValid());

        if (id.toString().contains("width") || id.toString().contains("x"))
        {
            Length parentWidth{ tree.getParent(), "width" };
            return static_cast<double>(parentWidth.calculatePixelValue());
        }

        Length parentHeight{ tree.getParent(), "height" };
        return static_cast<double>(parentHeight.calculatePixelValue());
    }
} // namespace jive

//======================================================================================================================
#if JIVE_UNIT_TESTS
class LengthUnitTest : public juce::UnitTest
{
public:
    LengthUnitTest()
        : juce::UnitTest{ "jive::Length", "jive" }
    {
    }

    void runTest() final
    {
        testPixels();
        testPercent();
    }

private:
    std::unique_ptr<jive::GuiItem> createGuiItem(juce::ValueTree tree)
    {
        jive::Interpreter interpreter;

        return interpreter.interpret(tree);
    }

    void testPixels()
    {
        beginTest("pixels");

        juce::ValueTree tree{ "Component" };
        jive::Length width{ tree, "width" };
        expect(width.isAuto());

        width = "10";
        expectEquals(width.calculatePixelValue(), 10.f);

        width = "312.65";
        expectEquals(width.calculatePixelValue(), 312.65f);
    }

    void testPercent()
    {
        beginTest("percent");

        {
            juce::ValueTree tree{
                "Component",
                {
                    { "width", 40.f },
                    { "height", 20.f },
                },
                {
                    juce::ValueTree{ "Component" },
                }
            };
            jive::Length width{ tree.getChild(0), "width" };
            width = "50%";
            expectEquals(width.calculatePixelValue(), 20.f);

            jive::Length height{ tree.getChild(0), "height" };
            height = "20%";
            expectWithinAbsoluteError(height.calculatePixelValue(), 4.f, 0.000001f);
        }
    }
};

static LengthUnitTest lengthUnitTest;
#endif
