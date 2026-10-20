#include "jive_ContainerItem.h"

#include "jive_CommonGuiItem.h"

namespace jive
{
    ContainerItem::ContainerItem(std::unique_ptr<GuiItem> itemToDecorate)
        : GuiItemDecorator{ std::move(itemToDecorate) }
        , box{ boxModel(*this) }
        , idealWidth{ state, ids::idealWidth() }
        , idealHeight{ state, ids::idealHeight() }
    {
        box.addListener(*this);
        state.addListener(this);
    }

    ContainerItem::~ContainerItem()
    {
        state.removeListener(this);
        box.removeListener(*this);
    }

    void ContainerItem::insertChild(std::unique_ptr<GuiItem> child, int index)
    {
        JIVE_TRACE("index", index);

        const auto numChildrenBefore = getChildren().size();
        GuiItemDecorator::insertChild(std::move(child), index);

        if (getChildren().size() != numChildrenBefore && !static_cast<bool>(state[ids::setupInProgress()]))
            updateIdealSize();
    }

    void ContainerItem::setChildren(std::vector<std::unique_ptr<GuiItem>>&& newChildren)
    {
        JIVE_TRACE("std::size(newChildren)", std::size(newChildren));

        {
            const BoxModel::ScopedCallbackLock boxModelLock{ box };
            GuiItemDecorator::setChildren(std::move(newChildren));
        }

        if (!getChildren().isEmpty() && !static_cast<bool>(state[ids::setupInProgress()]))
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
            || static_cast<bool>(state[ids::setupInProgress()]))
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

        if (const auto* thisParent = dynamic_cast<const GuiItemDecorator*>(getParent()))
        {
            if (const auto* parentContainer = thisParent->getTopLevelDecorator().toType<ContainerItem>())
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
            updateChildrensIdealSizesIfOutOfDate();

        explicitConstraintsWhenIdealSizeWasMeasured = box.getExplicitConstraints();

        const auto constraints = getContentConstraints();

        if (!measureChildren && constraints != constraintsWhenIdealSizeWasMeasured)
            childrenMayBeOutOfDate = true;

        const auto newIdealSize = calculateIdealSize(constraints);
        constraintsWhenIdealSizeWasMeasured = constraints;
        idealSizeIsOutOfDate = false;
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
                const BoxModel::ScopedCallbackLock boxModelLock{ box };
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

        if (informParentOfChanges && getParent() != nullptr && !static_cast<bool>(state[ids::setupInProgress()]))
        {
            if (auto* decorator = dynamic_cast<GuiItemDecorator*>(getParent()))
            {
                if (auto* containerParent = decorator->getTopLevelDecorator().toType<ContainerItem>())
                    containerParent->updateIdealSize(true, false);
            }
        }

        callLayoutChildrenWithRecursionLock();
    }

    void ContainerItem::invalidateIdealSize()
    {
        idealSizeIsOutOfDate = true;

        for (auto* ancestor = getParent(); ancestor != nullptr; ancestor = ancestor->getParent())
        {
            if (auto* decorator = dynamic_cast<GuiItemDecorator*>(ancestor))
                if (auto* container = decorator->getTopLevelDecorator().toType<ContainerItem>())
                    container->childrenMayBeOutOfDate = true;
        }
    }

    // A container is only re-measured when something it was measured from has
    // changed: the constraints it was given, its own size (its children's
    // relative min and max sizes are based on it), or anything else in its
    // subtree besides its own ideal size and its descendants' sizes. Any change
    // below it means its children need checking, in case it affected theirs.
    void ContainerItem::updateIdealSizeIfOutOfDate()
    {
        if (idealSizeIsOutOfDate || getContentConstraints() != constraintsWhenIdealSizeWasMeasured)
        {
            updateIdealSize(false);
            return;
        }

        if (!childrenMayBeOutOfDate)
            return;

        updateChildrensIdealSizesIfOutOfDate();

        if (idealSizeIsOutOfDate)
            updateIdealSize(false, false);
    }

    void ContainerItem::updateChildrensIdealSizesIfOutOfDate()
    {
        childrenMayBeOutOfDate = false;

        for (auto* child : getChildren())
        {
            if (auto* decorator = dynamic_cast<GuiItemDecorator*>(child))
                if (auto* container = decorator->getTopLevelDecorator().toType<ContainerItem>())
                    container->updateIdealSizeIfOutOfDate();
        }
    }

    void ContainerItem::valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& id)
    {
        const auto isOwnProperty = tree == state;

        if (id == ids::idealWidth() || id == ids::idealHeight())
        {
            if (!isOwnProperty)
                idealSizeIsOutOfDate = true;

            return;
        }

        if (id == ids::componentWidth() || id == ids::componentHeight())
        {
            if (isOwnProperty)
                idealSizeIsOutOfDate = true;

            childrenMayBeOutOfDate = true;
            return;
        }

        idealSizeIsOutOfDate = true;
        childrenMayBeOutOfDate = true;
    }

    void ContainerItem::valueTreeChildAdded(juce::ValueTree&, juce::ValueTree&)
    {
        idealSizeIsOutOfDate = true;
        childrenMayBeOutOfDate = true;
    }

    void ContainerItem::valueTreeChildRemoved(juce::ValueTree&, juce::ValueTree&, int)
    {
        idealSizeIsOutOfDate = true;
        childrenMayBeOutOfDate = true;
    }

    void ContainerItem::valueTreeChildOrderChanged(juce::ValueTree&, int, int)
    {
        idealSizeIsOutOfDate = true;
        childrenMayBeOutOfDate = true;
    }
} // namespace jive

#if JIVE_UNIT_TESTS
    #include <jive_layouts/layout/interpreter/jive_Interpreter.h>

    #include <utility>

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
        testUnchangedContainerNotRemeasured();
        testContainerRemeasuredWhenItsContentChanges();
        testContainerRemeasuredWhenAvailableSpaceChanges();
        testContainerRemeasuredAfterBeingResized();
        testContainerRemeasuredWhenChildsIdealSizeChanges();
        testContainerRemeasuredWhenItsChildrenChange();
        testContainerRemeasuredWhenInvalidated();
        testLayoutMatchesFreshInterpretationAfterChanges();
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
        const auto& sidebar = *item->getChildren()[0]->getComponent();
        const auto idealWidth = sidebar.getWidth();
        expectGreaterThan(idealWidth, 50);

        for (auto width = 200; width >= 1; --width)
            state.setProperty("width", width, nullptr);

        for (auto width = 1; width <= 200; ++width)
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

        const juce::ValueTree state{
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

    class MeasurementCounter : public jive::ContainerItem
    {
    public:
        using jive::ContainerItem::ContainerItem;

        mutable int numMeasurements = 0;
        std::function<juce::Rectangle<float>()> measure = [] {
            return juce::Rectangle<float>{ 10.0f, 10.0f };
        };

    protected:
        juce::Rectangle<float> calculateIdealSize(juce::Rectangle<float>) const final
        {
            ++numMeasurements;
            return measure();
        }
    };

    static void addMeasurementCounter(jive::Interpreter& interpreter)
    {
        interpreter.getComponentFactory().set("Counter", [] {
            return std::make_shared<juce::Component>();
        });
        interpreter.addDecorator<MeasurementCounter>("Counter");
    }

    [[nodiscard]] static MeasurementCounter& findMeasurementCounter(jive::GuiItem& root,
                                                                    juce::Identifier id = "counter")
    {
        auto& counter = *dynamic_cast<jive::GuiItemDecorator&>(*jive::findItemWithID(root, std::move(id)))
                             .toType<MeasurementCounter>();
        counter.numMeasurements = 0;
        return counter;
    }

    void testUnchangedContainerNotRemeasured()
    {
        beginTest("a container isn't re-measured when nothing it depends on has changed");

        jive::Interpreter interpreter;
        addMeasurementCounter(interpreter);
        const auto state = jive::parseXML(R"(
            <Component width="300" height="300">
                <Text>Short</Text>
                <Component width="100" height="100">
                    <Counter id="counter" width="10" height="10" />
                </Component>
            </Component>
        )");
        auto root = interpreter.interpret(state);
        const auto& counter = findMeasurementCounter(*root);

        state.getChild(0).setProperty("text", "Something much longer", nullptr);
        expectEquals(counter.numMeasurements, 0);
    }

    void testContainerRemeasuredWhenItsContentChanges()
    {
        beginTest("a container is re-measured when something inside it has changed");

        jive::Interpreter interpreter;
        addMeasurementCounter(interpreter);
        const auto state = jive::parseXML(R"(
            <Component width="300" height="300">
                <Text>Short</Text>
                <Component width="100" height="100">
                    <Counter id="counter" width="10" height="10" />
                </Component>
            </Component>
        )");
        auto root = interpreter.interpret(state);
        const auto& counter = findMeasurementCounter(*root);

        state.getChild(1).getChild(0).setProperty("some-property", "some-value", nullptr);
        state.getChild(0).setProperty("text", "Something much longer", nullptr);
        expectEquals(counter.numMeasurements, 1);
    }

    void testContainerRemeasuredWhenAvailableSpaceChanges()
    {
        beginTest("a container is re-measured when the space available to it has changed");

        jive::Interpreter interpreter;
        addMeasurementCounter(interpreter);
        const auto state = jive::parseXML(R"(
            <Component width="300" height="300">
                <Text>Short</Text>
                <Component width="100" height="100" align-items="flex-start">
                    <Counter id="counter" />
                </Component>
            </Component>
        )");
        auto root = interpreter.interpret(state);
        state.getChild(0).setProperty("text", "Something longer", nullptr);
        const auto& counter = findMeasurementCounter(*root);
        const auto counterBounds = jive::findItemWithID(*root, "counter")->getComponent()->getBounds();

        state.getChild(1).setProperty("width", 50, nullptr);
        state.getChild(0).setProperty("text", "Something much longer", nullptr);
        expectEquals(jive::findItemWithID(*root, "counter")->getComponent()->getBounds(), counterBounds);
        expectEquals(counter.numMeasurements, 1);
    }

    void testContainerRemeasuredAfterBeingResized()
    {
        beginTest("a container is re-measured after it has been resized");

        jive::Interpreter interpreter;
        addMeasurementCounter(interpreter);
        const auto state = jive::parseXML(R"(
            <Component width="300" height="300">
                <Counter id="outer" width="100" height="20" flex-direction="row">
                    <Counter id="inner" flex-grow="1" />
                    <Text>A</Text>
                </Counter>
                <Text>B</Text>
            </Component>
        )");
        auto root = interpreter.interpret(state);
        state.getChild(1).setProperty("text", "BB", nullptr);
        const auto& outer = findMeasurementCounter(*root, "outer");
        auto& inner = findMeasurementCounter(*root, "inner");

        // Like an item with a relative min or max size, its ideal size depends
        // on its actual size.
        inner.measure = [&inner] {
            return juce::Rectangle<float>{ 10.0f, static_cast<float>(inner.getComponent()->getWidth()) / 10.0f };
        };

        state.getChild(0).getChild(1).setProperty("text", "AAAAAAAAAA", nullptr);
        expectEquals(inner.numMeasurements, 0);

        state.getChild(1).setProperty("text", "B", nullptr);
        expectEquals(inner.numMeasurements, 1);
        expectEquals(outer.numMeasurements, 2);
    }

    void testContainerRemeasuredWhenChildsIdealSizeChanges()
    {
        beginTest("a container is re-measured when a child's ideal size has changed");

        jive::Interpreter interpreter;
        addMeasurementCounter(interpreter);
        const auto state = jive::parseXML(R"(
            <Component width="300" height="300">
                <Counter id="outer" width="100" height="100">
                    <Counter id="inner" width="10" height="10" />
                </Counter>
                <Text>Short</Text>
            </Component>
        )");
        auto root = interpreter.interpret(state);
        const auto& outer = findMeasurementCounter(*root, "outer");
        auto& inner = findMeasurementCounter(*root, "inner");
        inner.measure = [&inner] {
            return juce::Rectangle<float>{ 10.0f, 10.0f + static_cast<float>(inner.numMeasurements) };
        };

        inner.invalidateIdealSize();
        state.getChild(1).setProperty("text", "Something much longer", nullptr);
        expectEquals(inner.numMeasurements, 1);
        expectEquals(outer.numMeasurements, 1);
    }

    void testContainerRemeasuredWhenItsChildrenChange()
    {
        beginTest("a container is re-measured when its children have changed");

        jive::Interpreter interpreter;
        addMeasurementCounter(interpreter);
        const auto state = jive::parseXML(R"(
            <Component width="300" height="300">
                <Text>Short</Text>
                <Component width="100" height="100">
                    <Counter id="counter" width="10" height="10">
                        <Component id="first" />
                        <Component id="second" />
                        <Component id="third" />
                    </Counter>
                </Component>
            </Component>
        )");
        auto root = interpreter.interpret(state);
        const auto& counter = findMeasurementCounter(*root);
        auto counterState = state.getChild(1).getChild(0);

        counterState.removeChild(2, nullptr);
        state.getChild(0).setProperty("text", "Something much longer", nullptr);
        expectEquals(counter.numMeasurements, 1);

        counterState.moveChild(0, 1, nullptr);
        state.getChild(0).setProperty("text", "Short", nullptr);
        expectEquals(counter.numMeasurements, 2);
    }

    void testContainerRemeasuredWhenInvalidated()
    {
        beginTest("a container is re-measured after its ideal size is invalidated");

        jive::Interpreter interpreter;
        addMeasurementCounter(interpreter);
        const auto state = jive::parseXML(R"(
            <Component width="300" height="300">
                <Text>Short</Text>
                <Component width="100" height="100">
                    <Counter id="counter" width="10" height="10" />
                </Component>
            </Component>
        )");
        auto root = interpreter.interpret(state);
        auto& counter = findMeasurementCounter(*root);

        counter.invalidateIdealSize();
        state.getChild(0).setProperty("text", "Something much longer", nullptr);
        expectEquals(counter.numMeasurements, 1);
    }

    void expectSameBounds(jive::GuiItem& item, jive::GuiItem& expected, const juce::String& context)
    {
        const auto description = context + ": " + item.state.getType().toString() + "#" + item.state["id"].toString();
        expectEquals(item.getComponent()->getBounds(), expected.getComponent()->getBounds(), description);

        if (item.getChildren().size() != expected.getChildren().size())
        {
            expect(false, description + " has a different number of children");
            return;
        }

        for (auto i = 0; i < item.getChildren().size(); ++i)
            expectSameBounds(*item.getChildren()[i], *expected.getChildren()[i], context);
    }

    static void removeComputedProperties(juce::ValueTree tree)
    {
        for (auto i = tree.getNumProperties() - 1; i >= 0; --i)
        {
            if (tree.getPropertyName(i).toString().startsWith("jive::"))
                tree.removeProperty(tree.getPropertyName(i), nullptr);
        }

        for (const auto child : tree)
            removeComputedProperties(child);
    }

    void expectSameLayoutAsFreshInterpretation(jive::GuiItem& item, const juce::String& context)
    {
        const auto copy = item.state.createCopy();
        removeComputedProperties(copy);

        jive::Interpreter interpreter;
        const auto fresh = interpreter.interpret(copy);
        expectSameBounds(item, *fresh, context);
    }

    void testLayoutMatchesFreshInterpretationAfterChanges()
    {
        beginTest("layout matches that of a fresh interpretation after changes");

        jive::Interpreter interpreter;
        const auto state = jive::parseXML(R"(
            <Component id="root" width="400" height="400" flex-direction="row" gap="10">
                <Component id="sidebar" gap="5" padding="5">
                    <Button padding="5"><Text>One</Text></Button>
                    <Button padding="5"><Text>Two</Text></Button>
                </Component>
                <Component id="main" flex-grow="1" gap="10">
                    <Text id="heading">Heading</Text>
                    <Component id="cards" flex-direction="row" flex-wrap="wrap" gap="10">
                        <Component id="first" width="120" padding="5">
                            <Text id="body">Some body text that wraps.</Text>
                            <Component flex-direction="row">
                                <Button><Text>OK</Text></Button>
                            </Component>
                        </Component>
                        <Component id="second" width="120" padding="5">
                            <Text>More body text.</Text>
                        </Component>
                    </Component>
                </Component>
            </Component>
        )");
        auto root = interpreter.interpret(state);
        expectSameLayoutAsFreshInterpretation(*root, "initial");

        const std::vector<std::tuple<juce::String, juce::Identifier, juce::var>> changes{
            { "heading", "text", "A heading that is long enough to wrap onto more lines than before" },
            { "body", "text", "Body text that is now quite a lot longer than it was before it changed." },
            { "cards", "flex-direction", "column" },
            { "second", "width", 200 },
            { "heading", "text", "Short" },
            { "cards", "gap", 30 },
            { "body", "text", "Short" },
            { "root", "width", 300 },
            { "root", "width", 500 },
        };

        for (const auto& [id, property, value] : changes)
        {
            jive::findElementWithID(state, id).setProperty(property, value, nullptr);
            expectSameLayoutAsFreshInterpretation(*root, id + " " + property.toString());
        }

        jive::findElementWithID(state, "cards").appendChild(jive::parseXML(R"(<Component width="50" height="60" />)"), nullptr);
        expectSameLayoutAsFreshInterpretation(*root, "child added");
    }
};

static ContainerItemUnitTest containerItemUnitTest;
#endif
