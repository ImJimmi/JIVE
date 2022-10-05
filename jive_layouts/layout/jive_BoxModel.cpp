#include <jive_layouts/jive_layouts.h>

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    BoxModel::BoxModel(juce::ValueTree sourceMarkup)
        : markup{ sourceMarkup }
        , padding{ markup, "padding" }
        , border{ markup, "border-width" }
        , margin{ markup, "margin" }
        , width{ markup, "width" }
        , height{ markup, "height" }
        , explicitWidth{ markup, "explicit-width" }
        , explicitHeight{ markup, "explicit-height" }
    {
        if (markup.getParent().isValid())
        {
            parentWidth = std::make_shared<TypedValue<float>>(markup.getParent(), "explicit-width");
            parentWidth->onValueChange = [this]() {
                setWidth(width.calculatePixelValue());
            };

            parentHeight = std::make_shared<TypedValue<float>>(markup.getParent(), "explicit-height");
            parentHeight->onValueChange = [this]() {
                setHeight(height.calculatePixelValue());
            };
        }
    }

    //==================================================================================================================
    float BoxModel::getWidth() const
    {
        return explicitWidth;
    }

    void BoxModel::setWidth(float newWidth)
    {
        jassert(newWidth >= 0.0f);
        explicitWidth = newWidth;
    }

    float BoxModel::getHeight() const
    {
        return explicitHeight;
    }

    void BoxModel::setHeight(float newHeight)
    {
        jassert(newHeight >= 0.0f);
        explicitHeight = newHeight;
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
        return { calculateBorderBoundsWidth(), calculateBorderBoundsHeight() };
    }

    juce::Rectangle<float> BoxModel::getPaddingBounds() const
    {
        return border.get().subtractedFrom(getBorderBounds());
    }

    juce::Rectangle<float> BoxModel::getContentBounds() const
    {
        return padding.get().subtractedFrom(getPaddingBounds());
    }

    //==================================================================================================================
    juce::Rectangle<float> BoxModel::calculateContentBounds(const juce::Component& component)
    {
        const auto contentBounds = getBorder().subtractedFrom(getPadding().subtractedFrom(component.getLocalBounds().toFloat()));
        return {
            juce::jmax(0.0f, contentBounds.getX()),
            juce::jmax(0.0f, contentBounds.getY()),
            juce::jmax(0.0f, contentBounds.getWidth()),
            juce::jmax(0.0f, contentBounds.getHeight()),
        };
    }

    //==================================================================================================================
    float BoxModel::calculateBorderBoundsWidth() const
    {
        if (markup.getParent().isValid())
            return getWidth() + padding.get().getLeftAndRight() + border.get().getLeftAndRight();

        return getWidth();
    }

    float BoxModel::calculateBorderBoundsHeight() const
    {
        if (markup.getParent().isValid())
            return getHeight() + padding.get().getTopAndBottom() + border.get().getTopAndBottom();

        return getHeight();
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
        testTopLevelItems();
        testParentResizing();
    }

private:
    void testWidthAndHeight()
    {
        beginTest("width and height");

        juce::ValueTree tree{ "Component" };
        jive::GuiItem item{ std::make_unique<juce::Component>(), tree };
        jive::BoxModel box{ tree };
        expectEquals(box.getWidth(), 0.0f);
        expectEquals(box.getHeight(), 0.0f);

        tree.setProperty("width", 312.4f, nullptr);
        tree.setProperty("height", 846.2f, nullptr);
        expectEquals(box.getWidth(), 312.4f);
        expectEquals(box.getHeight(), 846.2f);
    }

    void testPadding()
    {
        beginTest("padding");

        juce::ValueTree tree{ "Component" };
        jive::GuiItem item{ std::make_unique<juce::Component>(), tree };
        jive::BoxModel box{ tree };

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
        jive::GuiItem item{ std::make_unique<juce::Component>(), tree };
        jive::BoxModel box{ tree };

        expect(box.getBorder() == juce::BorderSize<float>{ 0.f });

        tree.setProperty("border-width", "9 8 7 6", nullptr);

        expect(box.getBorder() == juce::BorderSize<float>{ 9.f, 6.f, 7.f, 8.f });
    }

    void testMargin()
    {
        beginTest("margin");

        juce::ValueTree tree{ "Component" };
        jive::GuiItem item{ std::make_unique<juce::Component>(), tree };
        jive::BoxModel box{ tree };

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
        jive::Interpreter interpreter;
        auto item = interpreter.interpret(tree);
        jive::BoxModel box{ tree.getChild(0) };

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
        jive::Interpreter interpreter;
        auto item = interpreter.interpret(tree);
        jive::BoxModel box{ tree.getChild(0) };

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
        jive::Interpreter interpreter;
        auto item = interpreter.interpret(tree);
        jive::BoxModel box{ tree.getChild(0) };

        expect(box.getContentBounds() == juce::Rectangle<float>{ 50.f, 35.f, 75.f, 55.f });
    }

    void testTopLevelItems()
    {
        beginTest("top-level-items");

        juce::ValueTree tree{
            "Component",
            { { "width", 100 },
              { "height", 100 },
              { "padding", "25" },
              { "border-width", "5 10 15 20" } }
        };
        jive::GuiItem item{ std::make_unique<juce::Component>(), tree };
        jive::BoxModel box{ tree };

        expect(box.getBorderBounds().getWidth() == 100.f);
        expect(box.getBorderBounds().getHeight() == 100.f);
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
        jive::Interpreter interpreter;
        const auto parent = interpreter.interpret(tree);
        const auto box = parent->getChild(0).getBoxModel();
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
