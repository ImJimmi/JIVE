#include "jive_Length.h"

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    const float Length::pixelValueWhenAuto = 0.0f;

    //==================================================================================================================
    float Length::toPixels(const juce::Rectangle<float>& parentBounds) const
    {
        if (isAuto())
            return pixelValueWhenAuto;

        if (isPixels())
            return get().getFloatValue();

        const auto scale = static_cast<double>(get().getFloatValue()) * 0.01;
        return static_cast<float>(scale * getRelativeParentLength(parentBounds.toDouble()));
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
    double Length::getRelativeParentLength(const juce::Rectangle<double>& parentBounds) const
    {
        jassert(tree.getParent().isValid());

        if (id.toString().contains("width") || id.toString().contains("x"))
            return parentBounds.getWidth();

        return parentBounds.getHeight();
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
        expectEquals(width.toPixels({}), 10.f);

        width = "312.65";
        expectEquals(width.toPixels({}), 312.65f);
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
            expectEquals(width.toPixels({ 40.0f, 20.0f }), 20.f);

            jive::Length height{ tree.getChild(0), "height" };
            height = "20%";
            expectWithinAbsoluteError(height.toPixels({ 40.0f, 20.0f }), 4.f, 0.000001f);
        }
    }
};

static LengthUnitTest lengthUnitTest;
#endif
