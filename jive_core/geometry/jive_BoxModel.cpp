#include <jive_layouts/jive_layouts.h>

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    BoxModel::BoxModel(juce::ValueTree stateSource)
        : state{ stateSource }
        , width{ state, "width", "auto" }
        , height{ state, "height", "auto" }
        , padding{ state, "padding" }
        , border{ state, "border-width" }
        , margin{ state, "margin" }
        , componentWidth{ state, "component-width" }
        , componentHeight{ state, "component-height" }
    {
        if (state.getParent().isValid())
            parentBoxModel = std::make_unique<BoxModel>(state.getParent());

        const auto recalculateWidth = [this]() {
            if (!width.isAuto())
            {
                const auto parentBounds = parentBoxModel != nullptr ? parentBoxModel->getContentBounds() : juce::Rectangle<float>{};
                setWidth(width.toPixels(parentBounds));
            }
        };
        width.onValueChange = recalculateWidth;
        recalculateWidth();

        const auto recalculateHeight = [this]() {
            if (!height.isAuto())
            {
                const auto parentBounds = parentBoxModel != nullptr ? parentBoxModel->getContentBounds() : juce::Rectangle<float>{};
                setHeight(height.toPixels(parentBounds));
            }
        };
        height.onValueChange = recalculateHeight;
        recalculateHeight();
    }

    //==================================================================================================================
    float BoxModel::getWidth() const
    {
        return juce::jmax(0.0f, getContentBounds().getWidth());
    }

    void BoxModel::setWidth(float newWidth)
    {
        jassert(newWidth >= 0.0f);

        if (state.getParent().isValid())
            newWidth += padding.get().getLeftAndRight() + border.get().getLeftAndRight();

        componentWidth = newWidth;
    }

    float BoxModel::getHeight() const
    {
        return juce::jmax(0.0f, getContentBounds().getHeight());
    }

    void BoxModel::setHeight(float newHeight)
    {
        jassert(newHeight >= 0.0f);

        if (state.getParent().isValid())
            newHeight += padding.get().getTopAndBottom() + border.get().getTopAndBottom();

        componentHeight = newHeight;
    }

    juce::BorderSize<float> BoxModel::getPadding() const
    {
        return padding;
    }

    juce::BorderSize<float> BoxModel::getBorder() const
    {
        return border;
    }

    juce::BorderSize<float> BoxModel::getMargin() const
    {
        return margin;
    }

    juce::Rectangle<float> BoxModel::getBorderBounds() const
    {
        return { componentWidth, componentHeight };
    }

    juce::Rectangle<float> BoxModel::getPaddingBounds() const
    {
        return border.get().subtractedFrom(getBorderBounds());
    }

    juce::Rectangle<float> BoxModel::getContentBounds() const
    {
        return padding.get().subtractedFrom(getPaddingBounds());
    }
} // namespace jive

//======================================================================================================================
#if JIVE_UNIT_TESTS
class BoxModelUnitTest : public juce::UnitTest
{
public:
    BoxModelUnitTest()
        : juce::UnitTest{ "jive::BoxModel", "jive" }
    {
    }

    void runTest() final
    {
        testWidthAndHeight();
        testPadding();
        testBorder();
        testMargin();
        testBorderBounds();
        testPaddingBounds();
        testContentBounds();
    }

private:
    void testWidthAndHeight()
    {
        beginTest("width and height");

        {
            juce::ValueTree state{ "Component" };
            jive::BoxModel box{ state };
            expectEquals(box.getWidth(), 0.0f);
            expectEquals(box.getHeight(), 0.0f);

            state.setProperty("width", 312.4f, nullptr);
            expectEquals(box.getWidth(), 312.4f);

            state.setProperty("height", 846.2f, nullptr);
            expectEquals(box.getHeight(), 846.2f);

            box.setWidth(112.8f);
            expectEquals(box.getWidth(), 112.8f);

            box.setHeight(839.49f);
            expectEquals(box.getHeight(), 839.49f);
        }
        {
            juce::ValueTree state{
                "Component",
                {
                    { "component-width", 100 },
                    { "component-height", 200 },
                },
            };
            jive::BoxModel box{ state };
            expectEquals(state["width"], juce::var{ "auto" });
            expectEquals(state["height"], juce::var{ "auto" });

            state.setProperty("width", 334, nullptr);
            expectEquals(static_cast<float>(state["component-width"]), 334.f);

            state.setProperty("width", "auto", nullptr);
            expectEquals(static_cast<float>(state["component-width"]), 334.f);

            state.setProperty("height", 990, nullptr);
            expectEquals(static_cast<float>(state["component-height"]), 990.f);
        }
    }

    void testPadding()
    {
        beginTest("padding");

        juce::ValueTree state{ "Component" };
        jive::BoxModel box{ state };
        expect(box.getPadding() == juce::BorderSize<float>{ 0.f });

        state.setProperty("padding", 14, nullptr);
        expect(box.getPadding() == juce::BorderSize<float>{ 14.f });

        state.setProperty("padding", "112.4 73.7", nullptr);
        expect(box.getPadding() == juce::BorderSize<float>{ 112.4f, 73.7f, 112.4f, 73.7f });

        state.setProperty("padding", "14.25 8.3 1.1", nullptr);
        expect(box.getPadding() == juce::BorderSize<float>{ 14.25f, 8.3f, 1.1f, 8.3f });

        state.setProperty("padding", "1 2 3 4", nullptr);
        expect(box.getPadding() == juce::BorderSize<float>{ 1.f, 4.f, 3.f, 2.f });
    }

    void testBorder()
    {
        beginTest("border");

        juce::ValueTree state{ "Component" };
        jive::BoxModel box{ state };
        expect(box.getBorder() == juce::BorderSize<float>{ 0.f });

        state.setProperty("border-width", "9 8 7 6", nullptr);
        expect(box.getBorder() == juce::BorderSize<float>{ 9.f, 6.f, 7.f, 8.f });
    }

    void testMargin()
    {
        beginTest("margin");

        juce::ValueTree state{ "Component" };
        jive::BoxModel box{ state };
        expect(box.getMargin() == juce::BorderSize<float>{ 0.f });

        state.setProperty("margin", "2 4 6 8", nullptr);
        expect(box.getMargin() == juce::BorderSize<float>{ 2.f, 8.f, 6.f, 4.f });
    }

    void testBorderBounds()
    {
        beginTest("border-bounds");

        juce::ValueTree state{
            "Component",
            {},
            {
                juce::ValueTree{
                    "Component",
                    {
                        { "width", 100 },
                        { "height", 25 },
                        { "padding", "10 20" },
                        { "border-width", "5" },
                    },
                },
            },
        };
        jive::BoxModel box{ state.getChild(0) };
        expectEquals(box.getBorderBounds(), juce::Rectangle<float>{ 0.f, 0.f, 150.f, 55.f });
    }

    void testPaddingBounds()
    {
        beginTest("padding-bounds");

        juce::ValueTree state{
            "Component",
            {},
            {
                juce::ValueTree{
                    "Component",
                    {
                        { "width", 130 },
                        { "height", 20 },
                        { "padding", "15 5" },
                        { "border-width", "10" },
                    },
                },
            },
        };
        jive::BoxModel box{ state.getChild(0) };
        expect(box.getPaddingBounds() == juce::Rectangle<float>{ 10.f, 10.f, 140.f, 50.f });
    }

    void testContentBounds()
    {
        beginTest("content-bounds");

        juce::ValueTree state{
            "Component",
            {},
            {
                juce::ValueTree{
                    "Component",
                    {
                        { "width", 75 },
                        { "height", 55 },
                        { "padding", "30" },
                        { "border-width", "5 10 15 20" },
                    },
                },
            }
        };
        jive::BoxModel box{ state.getChild(0) };
        expect(box.getContentBounds() == juce::Rectangle<float>{ 50.f, 35.f, 75.f, 55.f });
    }
};

static BoxModelUnitTest boxModelUnitTest;
#endif
