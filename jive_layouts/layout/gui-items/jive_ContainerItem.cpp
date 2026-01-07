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

        if (getChildren().size() != numChildrenBefore && !static_cast<bool>(state["jive::setup-in-progress"]))
            updateIdealSize();
    }

    void ContainerItem::setChildren(std::vector<std::unique_ptr<GuiItem>>&& newChildren)
    {
        {
            const BoxModel::ScopedCallbackLock boxModelLock{ box };
            GuiItemDecorator::setChildren(std::move(newChildren));
        }

        if (!getChildren().isEmpty() && !static_cast<bool>(state["jive::setup-in-progress"]))
            updateIdealSize();
    }

    juce::Rectangle<float> ContainerItem::getContentConstraints() const
    {
        auto constraints = box.getExplicitConstraints();

        if (auto* thisParent = dynamic_cast<const GuiItemDecorator*>(getParent()))
        {
            if (auto* parentContainer = thisParent->getTopLevelDecorator().toType<ContainerItem>())
                constraints = constraints.getIntersection(parentContainer->getContentConstraints());
        }

        return box
            .getPadding()
            .subtractedFrom(box.getBorder()
                                .subtractedFrom(constraints));
    }

    void ContainerItem::updateIdealSize(bool informParentOfChanges)
    {
        for (auto* child : getChildren())
        {
            if (auto* decorator = dynamic_cast<GuiItemDecorator*>(child))
                if (auto* container = decorator->getTopLevelDecorator().toType<ContainerItem>())
                    container->updateIdealSize(false);
        }

        const auto newIdealSize = calculateIdealSize(getContentConstraints());
        const auto widthChanged = !juce::approximatelyEqual(newIdealSize.getWidth(), idealWidth.get());
        const auto heightChanged = !juce::approximatelyEqual(newIdealSize.getHeight(), idealHeight.get());

        if (!(widthChanged || heightChanged))
        {
            callLayoutChildrenWithRecursionLock();
            return;
        }

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

        if (informParentOfChanges && getParent() != nullptr && !static_cast<bool>(state["jive::setup-in-progress"]))
        {
            if (auto* decorator = dynamic_cast<GuiItemDecorator*>(getParent()))
            {
                if (auto* containerParent = decorator->getTopLevelDecorator().toType<ContainerItem>())
                    containerParent->updateIdealSize();
            }
        }

        callLayoutChildrenWithRecursionLock();
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
        container.updateIdealSize();
        expectEquals(container.givenConstraints, jive::boxModel(container).getContentBounds());
    }
};

static ContainerItemUnitTest containerItemUnitTest;
#endif
