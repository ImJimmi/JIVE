#include "jive_Length.h"

namespace jive
{
    [[nodiscard]] float Length::toPixels(const juce::Rectangle<float>& parentBounds) const
    {
        if (isAuto())
            return pixelValueWhenAuto;

        if (isPixels())
            return get().getFloatValue();

        if (isPercent())
        {
            const auto scale = static_cast<double>(get().getFloatValue()) * 0.01;
            return static_cast<float>(scale * getRelativeParentLength(parentBounds.toDouble()));
        }

        const auto fontSize = isRem() ? getRootFontSize() : getFontSize();
        return fontSize * get().getFloatValue();
    }

    [[nodiscard]] bool Length::isPixels() const
    {
        return !isAuto() && !isPercent() && !isEm() && !isRem();
    }

    [[nodiscard]] bool Length::isPercent() const
    {
        return !isAuto() && toString().endsWith("%");
    }

    [[nodiscard]] bool Length::isEm() const
    {
        return !isAuto() && !isRem() && toString().endsWithIgnoreCase("em");
    }

    [[nodiscard]] bool Length::isRem() const
    {
        return !isAuto() && toString().endsWithIgnoreCase("rem");
    }

    [[nodiscard]] double Length::getRelativeParentLength(const juce::Rectangle<double>& parentBounds) const
    {
        jassert(isValid(getParent(source)));

        if (id.toString().containsIgnoreCase("width") || id.toString().containsIgnoreCase("x"))
            return parentBounds.getWidth();

        return parentBounds.getHeight();
    }

    [[nodiscard]] float Length::getFontSize() const
    {
        for (auto toSearch = source;
             isValid(source);
             toSearch = getParent(source))
        {
            if (const auto style = getVar(toSearch, "style");
                style.isObject())
            {
                if (const auto fontSize = style["font-size"];
                    fontSize != juce::var{})
                {
                    return fromVar<float>(fontSize);
                }
            }
        }

        return 0.0f;
    }

    [[nodiscard]] float Length::getRootFontSize() const
    {
        if (const auto style = getVar(getRoot(source), "style");
            style.isObject())
        {
            if (const auto fontSize = style["font-size"];
                fontSize != juce::var{})
            {
                return fromVar<float>(fontSize);
            }
        }

        return 0.0f;
    }
} // namespace jive

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
        testEm();
        testRem();
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

    void testEm()
    {
        beginTest("em");

        juce::ValueTree state{
            "Component",
            {
                { "width", "123" },
                {
                    "style",
                    new jive::Object{
                        { "font-size", 16 },
                    },
                },
            },
        };
        jive::Length width{ state, "width" };
        expect(!width.isEm());
        expectEquals(width.toPixels({}), 123.0f);

        width = "2.5em";
        expect(width.isEm());
        expectEquals(width.toPixels({}), 40.0f);

        state.appendChild(juce::ValueTree{
                              "Component",
                              {
                                  { "height", "1.333em" },
                              },
                          },
                          nullptr);
        jive::Length height{ state.getChild(0), "height" };
        expect(height.isEm());
        expectEquals(height.toPixels({}), 21.328f);
    }

    void testRem()
    {
        beginTest("rem");

        juce::ValueTree state{
            "Component",
            {
                {
                    "style",
                    new jive::Object{
                        { "font-size", 20 },
                    },
                },
            },
            {
                juce::ValueTree{
                    "Component",
                    {
                        { "width", "2rem" },
                        {
                            "style",
                            new jive::Object{
                                { "font-size", 17 },
                            },
                        },
                    },
                },
            },
        };
        jive::Length width{ state.getChild(0), "width" };
        expect(!width.isEm());
        expect(width.isRem());
        expectEquals(width.toPixels({}), 40.0f);
    }
};

static LengthUnitTest lengthUnitTest;
#endif
