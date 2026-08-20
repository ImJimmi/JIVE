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
        JIVE_TRACE("index", index);

        const auto numChildrenBefore = getChildren().size();
        GuiItemDecorator::insertChild(std::move(child), index);

        if (getChildren().size() != numChildrenBefore && !static_cast<bool>(state["jive::setup-in-progress"]))
            updateIdealSize();
    }

    void ContainerItem::setChildren(std::vector<std::unique_ptr<GuiItem>>&& newChildren)
    {
        JIVE_TRACE("std::size(newChildren)", std::size(newChildren));

        {
            const BoxModel::ScopedCallbackLock boxModelLock{ box };
            GuiItemDecorator::setChildren(std::move(newChildren));
        }

        if (!getChildren().isEmpty() && !static_cast<bool>(state["jive::setup-in-progress"]))
            updateIdealSize();
    }

    // An ideal size is only as big as the space it was measured in allowed it to
    // be, so one measured when there wasn't much room says nothing about how big
    // the content actually wants to be. Nothing else asks for a fresh
    // measurement when room becomes available again - the content already fits,
    // so it's never found to be too big for its bounds - leaving whatever size
    // it was squashed down to for good.
    void ContainerItem::boxModelChanged(BoxModel& boxModelThatChanged)
    {
        if (!idealSizeWasLimitedByConstraints
            || boxModelThatChanged.state != state
            || static_cast<bool>(state["jive::setup-in-progress"]))
        {
            return;
        }

        const auto constraints = box.getExplicitConstraints();

        if (constraints.getWidth() > explicitConstraintsWhenIdealSizeWasMeasured.getWidth()
            || constraints.getHeight() > explicitConstraintsWhenIdealSizeWasMeasured.getHeight())
        {
            updateIdealSize();
        }
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

    void ContainerItem::updateIdealSize(bool informParentOfChanges, bool measureChildren)
    {
        JIVE_TRACE("inform parent?", informParentOfChanges);

        if (measureChildren)
        {
            for (auto* child : getChildren())
            {
                if (auto* decorator = dynamic_cast<GuiItemDecorator*>(child))
                    if (auto* container = decorator->getTopLevelDecorator().toType<ContainerItem>())
                        container->updateIdealSize(false);
            }
        }

        explicitConstraintsWhenIdealSizeWasMeasured = box.getExplicitConstraints();

        const auto constraints = getContentConstraints();
        const auto newIdealSize = calculateIdealSize(constraints);
        idealSizeWasLimitedByConstraints = newIdealSize.getWidth() >= constraints.getWidth()
                                        || newIdealSize.getHeight() >= constraints.getHeight();
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
                    containerParent->updateIdealSize(true, false);
            }
        }

        callLayoutChildrenWithRecursionLock();
    }
} // namespace jive

#if JIVE_UNIT_TESTS
    #include <jive_layouts/layout/interpreter/jive_Interpreter.h>

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
        testIdealSizeRemeasuredWhenRoomBecomesAvailable();
    }

private:
    void testIdealSizeRemeasuredWhenRoomBecomesAvailable()
    {
        beginTest("ideal-size re-measured when room becomes available");

        auto state = jive::parseXML(R"(
            <Component flex-direction="row" width="200" height="100">
                <Component padding="15">
                    <Button padding="10">Animations</Button>
                </Component>
                <Component flex-grow="1" />
            </Component>
        )");
        jive::Interpreter interpreter;
        auto item = interpreter.interpret(state);
        auto& sidebar = *item->getChildren()[0]->getComponent();
        const auto idealWidth = sidebar.getWidth();
        expectGreaterThan(idealWidth, 50);

        for (auto width = 200; width >= 1; width--)
            state.setProperty("width", width, nullptr);

        for (auto width = 1; width <= 200; width++)
            state.setProperty("width", width, nullptr);

        expectEquals(sidebar.getWidth(), idealWidth);
    }

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
