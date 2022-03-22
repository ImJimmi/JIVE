#include <jive_layouts/jive_layouts.h>

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    BoxModel::BoxModel(const GuiItem& owningItem, juce::ValueTree valueTree)
        : item{ owningItem }
        , tree{ valueTree }
        , padding{ tree, "padding" }
        , border{ tree, "border-width" }
        , margin{ tree, "margin" }
    {
    }

    //==================================================================================================================
    float BoxModel::getWidth() const
    {
        return item.getWidth();
    }

    float BoxModel::getHeight() const
    {
        return item.getHeight();
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
        const auto width = item.hasAutoWidth() ? item.getComponent().getWidth() : calculateBorderBoundsWidth();
        const auto height = item.hasAutoHeight() ? item.getComponent().getHeight() : calculateBorderBoundsHeight();

        return { width, height };
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
    float BoxModel::calculateBorderBoundsWidth() const
    {
        if (item.getParent() == nullptr)
            return getWidth();

        return getWidth() + padding.get().getLeftAndRight() + border.get().getLeftAndRight();
    }

    float BoxModel::calculateBorderBoundsHeight() const
    {
        if (item.getParent() == nullptr)
            return getHeight();

        return getHeight() + padding.get().getTopAndBottom() + border.get().getTopAndBottom();
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
    }

private:
    void testWidthAndHeight()
    {
        beginTest("width and height");

        juce::ValueTree tree{ "Component" };
        const jive::GuiItem item{ std::make_unique<juce::Component>(), tree };
        jive::BoxModel box{ item, tree };

        expect(box.getWidth() == item.getWidth());
        expect(box.getHeight() == item.getHeight());

        tree.setProperty("width", 312.4f, nullptr);
        tree.setProperty("height", 846.2f, nullptr);

        expect(box.getWidth() == item.getWidth());
        expect(box.getHeight() == item.getHeight());
    }

    void testPadding()
    {
        beginTest("padding");

        juce::ValueTree tree{ "Component" };
        const jive::GuiItem item{ std::make_unique<juce::Component>(), tree };
        jive::BoxModel box{ item, tree };

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
        const jive::GuiItem item{ std::make_unique<juce::Component>(), tree };
        jive::BoxModel box{ item, tree };

        expect(box.getBorder() == juce::BorderSize<float>{ 0.f });

        tree.setProperty("border-width", "9 8 7 6", nullptr);

        expect(box.getBorder() == juce::BorderSize<float>{ 9.f, 6.f, 7.f, 8.f });
    }

    void testMargin()
    {
        beginTest("margin");

        juce::ValueTree tree{ "Component" };
        const jive::GuiItem item{ std::make_unique<juce::Component>(), tree };
        jive::BoxModel box{ item, tree };

        expect(box.getMargin() == juce::BorderSize<float>{ 0.f });

        tree.setProperty("margin", "2 4 6 8", nullptr);

        expect(box.getMargin() == juce::BorderSize<float>{ 2.f, 8.f, 6.f, 4.f });
    }

    void testBorderBounds()
    {
        juce::ValueTree tree{
            "Component",
            { { "width", 100 },
              { "height", 25 },
              { "padding", "10 20" },
              { "border-width", "5" } }
        };
        jive::GuiItem parentItem{ std::make_unique<juce::Component>(), juce::ValueTree{ "Component" } };
        const jive::GuiItem item{ std::make_unique<juce::Component>(), tree, &parentItem };
        jive::BoxModel box{ item, tree };

        expect(box.getBorderBounds() == juce::Rectangle<float>{ 0.f, 0.f, 150.f, 55.f });
    }

    void testPaddingBounds()
    {
        juce::ValueTree tree{
            "Component",
            { { "width", 130 },
              { "height", 20 },
              { "padding", "15 5" },
              { "border-width", "10" } }
        };
        jive::GuiItem parentItem{ std::make_unique<juce::Component>(), juce::ValueTree{ "Component" } };
        const jive::GuiItem item{ std::make_unique<juce::Component>(), tree, &parentItem };
        jive::BoxModel box{ item, tree };

        expect(box.getPaddingBounds() == juce::Rectangle<float>{ 10.f, 10.f, 140.f, 50.f });
    }

    void testContentBounds()
    {
        juce::ValueTree tree{
            "Component",
            { { "width", 75 },
              { "height", 55 },
              { "padding", "30" },
              { "border-width", "5 10 15 20" } }
        };
        jive::GuiItem parentItem{ std::make_unique<juce::Component>(), juce::ValueTree{ "Component" } };
        const jive::GuiItem item{ std::make_unique<juce::Component>(), tree, &parentItem };
        jive::BoxModel box{ item, tree };

        expect(box.getContentBounds() == juce::Rectangle<float>{ 50.f, 35.f, 75.f, 55.f });
    }

    void testTopLevelItems()
    {
        juce::ValueTree tree{
            "Component",
            { { "width", 100 },
              { "height", 100 },
              { "padding", "25" },
              { "border-width", "5 10 15 20" } }
        };
        jive::GuiItem item{ std::make_unique<juce::Component>(), tree };
        jive::BoxModel box{ item, tree };

        expect(box.getBorderBounds().getWidth() == 100.f);
        expect(box.getBorderBounds().getHeight() == 100.f);
    }
};

static BoxModelUnitTest boxModelUnitTest;
#endif
