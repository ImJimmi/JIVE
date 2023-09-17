#include <jive_layouts/jive_layouts.h>

namespace jive
{
    ContainerItem::ContainerItem(std::unique_ptr<GuiItem> itemToDecorate)
        : GuiItemDecorator{ std::move(itemToDecorate) }
        , idealWidth{ state, "ideal-width" }
        , idealHeight{ state, "ideal-height" }
        , boxModel{ toType<CommonGuiItem>()->boxModel }
    {
        boxModel.addListener(*this);
    }

    ContainerItem::~ContainerItem()
    {
        boxModel.removeListener(*this);
    }

    void ContainerItem::insertChild(std::unique_ptr<GuiItem> child, int index)
    {
        GuiItemDecorator::insertChild(std::move(child), index);
        layoutChanged();
    }

    void ContainerItem::boxModelInvalidated(BoxModel& box)
    {
        const auto newIdealSize = calculateIdealSize(box.getContentBounds());
        const auto idealWidthChanged = !juce::approximatelyEqual(newIdealSize.getWidth(), idealWidth.get());
        const auto idealHeightChanged = !juce::approximatelyEqual(newIdealSize.getHeight(), idealHeight.get());

        idealWidth = newIdealSize.getWidth();
        idealHeight = newIdealSize.getHeight();

        const auto idealSizeChanged = idealWidthChanged || idealHeightChanged;

        if (!idealSizeChanged || isTopLevel())
            layOutChildren();
    }

    void ContainerItem::layoutChanged()
    {
        const auto newIdealSize = calculateIdealSize({
            static_cast<float>(std::numeric_limits<juce::uint16>::max()),
            static_cast<float>(std::numeric_limits<juce::uint16>::max()),
        });
        idealWidth = newIdealSize.getWidth();
        idealHeight = newIdealSize.getHeight();
    }
} // namespace jive

#if JIVE_UNIT_TESTS
class ContainerItemUnitTest : public juce::UnitTest
{
public:
    ContainerItemUnitTest()
        : juce::UnitTest{ "jive::ContainerItem", "jive" }
    {
    }

    void runTest() final
    {
        testIdealSizeCalculation();
    }

private:
    void testIdealSizeCalculation()
    {
        beginTest("ideal-size calculation");

        class SpyContainer : public jive::ContainerItem
        {
        public:
            using jive::ContainerItem::ContainerItem;

            mutable juce::Rectangle<float> givenConstraints;

        protected:
            juce::Rectangle<float> calculateIdealSize(juce::Rectangle<float> constraints) const final
            {
                givenConstraints = constraints;
                return constraints;
            }
        };

        juce::ValueTree state{
            "Component",
            {
                { "width", 300 },
                { "height", 200 },
                { "padding", 13 },
                { "border-width", 11 },
            },
        };
        auto commonItem = std::make_unique<jive::CommonGuiItem>(std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(), state));
        SpyContainer container{ std::move(commonItem) };
        state.setProperty("box-model-valid", false, nullptr);
        expectEquals(container.givenConstraints, jive::BoxModel{ state }.getContentBounds());
    }
};

static ContainerItemUnitTest containerItemUnitTest;
#endif
