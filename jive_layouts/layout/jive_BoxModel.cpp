#include <jive_layouts/jive_layouts.h>

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    BoxModel::BoxModel(std::shared_ptr<juce::Component> comp,
                       juce::ValueTree stateSource)
        : state{ stateSource }
        , component{ comp }
        , width{ state, "width", "auto" }
        , height{ state, "height", "auto" }
        , padding{ state, "padding" }
        , border{ state, "border-width" }
        , margin{ state, "margin" }
    {
        jassert(component != nullptr);

        width.onValueChange = [this]() {
            if (!width.isAuto())
                setWidth(width.calculatePixelValue());
        };
        if (!width.isAuto())
            setWidth(width.calculatePixelValue());

        height.onValueChange = [this]() {
            if (!height.isAuto())
                setHeight(height.calculatePixelValue());
        };
        if (!height.isAuto())
            setHeight(height.calculatePixelValue());

        if (state.getParent().isValid())
        {
            parentWidth = std::make_shared<TypedValue<float>>(state.getParent(), "width");
            parentWidth->onValueChange = [this]() {
                setWidth(width.calculatePixelValue());
            };

            parentHeight = std::make_shared<TypedValue<float>>(state.getParent(), "height");
            parentHeight->onValueChange = [this]() {
                setHeight(height.calculatePixelValue());
            };
        }
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

        component->setSize(juce::roundToInt(newWidth),
                           component->getHeight());
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

        component->setSize(component->getWidth(),
                           juce::roundToInt(newHeight));
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
        return component->getLocalBounds().toFloat();
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
        testParentResizing();
    }

private:
    void testWidthAndHeight()
    {
        beginTest("width and height");

        {
            juce::ValueTree tree{ "Component" };
            jive::BoxModel box{ std::make_shared<juce::Component>(), tree };
            expectEquals(box.getWidth(), 0.0f);
            expectEquals(box.getHeight(), 0.0f);

            tree.setProperty("width", 312.4f, nullptr);
            expectEquals(box.getWidth(), 312.0f);

            tree.setProperty("height", 846.2f, nullptr);
            expectEquals(box.getHeight(), 846.0f);

            box.setWidth(112.8f);
            expectEquals(box.getWidth(), 113.0f);

            box.setHeight(839.49f);
            expectEquals(box.getHeight(), 839.0f);
        }
        {
            juce::ValueTree tree{ "Component" };
            auto component = std::make_shared<juce::Component>();
            jive::BoxModel box{ component, tree };

            component->setSize(100, 200);
            expectEquals(tree["width"], juce::var{ "auto" });
            expectEquals(tree["height"], juce::var{ "auto" });

            tree.setProperty("width", 334, nullptr);
            expectEquals(component->getWidth(), 334);

            tree.setProperty("width", "auto", nullptr);
            expectEquals(component->getWidth(), 334);

            tree.setProperty("height", 990, nullptr);
            expectEquals(component->getHeight(), 990);
        }
    }

    void testPadding()
    {
        beginTest("padding");

        juce::ValueTree tree{ "Component" };
        jive::BoxModel box{ std::make_shared<juce::Component>(), tree };
        expect(box.getPadding() == juce::BorderSize<float>{ 0.f });

        tree.setProperty("padding", 14, nullptr);
        expect(box.getPadding() == juce::BorderSize<float>{ 14.f });

        tree.setProperty("padding", "112.4 73.7", nullptr);
        expect(box.getPadding() == juce::BorderSize<float>{ 112.4f, 73.7f, 112.4f, 73.7f });

        tree.setProperty("padding", "14.25 8.3 1.1", nullptr);
        expect(box.getPadding() == juce::BorderSize<float>{ 14.25f, 8.3f, 1.1f, 8.3f });

        tree.setProperty("padding", "1 2 3 4", nullptr);
        expect(box.getPadding() == juce::BorderSize<float>{ 1.f, 4.f, 3.f, 2.f });
    }

    void testBorder()
    {
        beginTest("border");

        juce::ValueTree tree{ "Component" };
        jive::BoxModel box{ std::make_shared<juce::Component>(), tree };
        expect(box.getBorder() == juce::BorderSize<float>{ 0.f });

        tree.setProperty("border-width", "9 8 7 6", nullptr);
        expect(box.getBorder() == juce::BorderSize<float>{ 9.f, 6.f, 7.f, 8.f });
    }

    void testMargin()
    {
        beginTest("margin");

        juce::ValueTree tree{ "Component" };
        jive::BoxModel box{ std::make_shared<juce::Component>(), tree };
        expect(box.getMargin() == juce::BorderSize<float>{ 0.f });

        tree.setProperty("margin", "2 4 6 8", nullptr);
        expect(box.getMargin() == juce::BorderSize<float>{ 2.f, 8.f, 6.f, 4.f });
    }

    void testBorderBounds()
    {
        beginTest("border-bounds");

        juce::ValueTree tree{
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
        jive::BoxModel box{ std::make_shared<juce::Component>(), tree.getChild(0) };
        expectEquals(box.getBorderBounds(), juce::Rectangle<float>{ 0.f, 0.f, 150.f, 55.f });
    }

    void testPaddingBounds()
    {
        beginTest("padding-bounds");

        juce::ValueTree tree{
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
        jive::BoxModel box{ std::make_shared<juce::Component>(), tree.getChild(0) };
        expect(box.getPaddingBounds() == juce::Rectangle<float>{ 10.f, 10.f, 140.f, 50.f });
    }

    void testContentBounds()
    {
        beginTest("content-bounds");

        juce::ValueTree tree{
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
        jive::BoxModel box{ std::make_shared<juce::Component>(), tree.getChild(0) };
        expect(box.getContentBounds() == juce::Rectangle<float>{ 50.f, 35.f, 75.f, 55.f });
    }

    void testParentResizing()
    {
        beginTest("parent-resizing");

        juce::ValueTree tree{
            "Component",
            {
                { "width", 50 },
                { "height", 50 },
            },
            {
                juce::ValueTree{
                    "Component",
                    {
                        { "width", "10%" },
                        { "height", "20%" },
                    },
                },
            },
        };
        jive::BoxModel box{ std::make_shared<juce::Component>(), tree.getChild(0) };
        expectWithinAbsoluteError(box.getWidth(), 5.0f, 0.00001f);
        expectWithinAbsoluteError(box.getHeight(), 10.0f, 0.00001f);

        tree.setProperty("width", 250, nullptr);
        expectWithinAbsoluteError(box.getWidth(), 25.0f, 0.00001f);

        tree.setProperty("height", 300, nullptr);
        expectWithinAbsoluteError(box.getHeight(), 60.0f, 0.00001f);
    }
};

static BoxModelUnitTest boxModelUnitTest;
#endif
