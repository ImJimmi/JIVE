#include "jive_ContainerItem.h"

#include "jive_CommonGuiItem.h"

namespace jive
{
    ContainerItem::ContainerItem(std::unique_ptr<GuiItem> itemToDecorate)
        : GuiItemDecorator{ std::move(itemToDecorate) }
        , box{ boxModel(*this) }
        , idealWidth{ state, "jive::ideal-width" }
        , idealHeight{ state, "jive::ideal-height" }
    {
        box.addListener(*this);
    }

    ContainerItem::~ContainerItem()
    {
        box.removeListener(*this);
    }

    void ContainerItem::insertChild(std::unique_ptr<GuiItem> child, int index)
    {
        const auto numChildrenBefore = getChildren().size();
        GuiItemDecorator::insertChild(std::move(child), index);

        if (getChildren().size() != numChildrenBefore)
            updateIdealSizeUnrestrained();
    }

    void ContainerItem::setChildren(std::vector<std::unique_ptr<GuiItem>>&& newChildren)
    {
        {
            const BoxModel::ScopedCallbackLock boxModelLock{ box };
            GuiItemDecorator::setChildren(std::move(newChildren));
        }

        if (!getChildren().isEmpty())
            updateIdealSizeUnrestrained();
    }

    void ContainerItem::updateIdealSizeUnrestrained()
    {
        updateIdealSize({
            static_cast<float>(std::numeric_limits<juce::uint16>::max()),
            static_cast<float>(std::numeric_limits<juce::uint16>::max()),
        });
    }

    void ContainerItem::updateIdealSizeWithinConstraints()
    {
        updateIdealSize(box.getContentBounds());
    }

    void ContainerItem::updateIdealSize(juce::Rectangle<float> constraints)
    {
        const auto newIdealSize = calculateIdealSize(constraints);
        const auto widthChanged = !juce::approximatelyEqual(newIdealSize.getWidth(), idealWidth.get());
        const auto heightChanged = !juce::approximatelyEqual(newIdealSize.getHeight(), idealHeight.get());

        if (widthChanged && heightChanged)
        {
            {
                BoxModel::ScopedCallbackLock boxModelLock{ box };
                idealWidth = newIdealSize.getWidth();
            }
            idealHeight = newIdealSize.getHeight();
        }
        else if (widthChanged)
        {
            idealWidth = newIdealSize.getWidth();
        }
        else if (heightChanged)
        {
            idealHeight = newIdealSize.getHeight();
        }
        else
        {
            callLayoutChildrenWithRecursionLock();
        }

        if ((widthChanged || heightChanged) && getParent() != nullptr)
        {
            if (auto* decorator = dynamic_cast<GuiItemDecorator*>(getParent()))
                if (auto* containerParent = decorator->getTopLevelDecorator().toType<ContainerItem>())
                    containerParent->updateIdealSizeUnrestrained();
        }
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
        container.updateIdealSizeWithinConstraints();
        expectEquals(container.givenConstraints, jive::boxModel(container).getContentBounds());
    }
};

static ContainerItemUnitTest containerItemUnitTest;
#endif
