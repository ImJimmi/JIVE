#include "jive_BoxModel.h"

#include <jive_core/jive_core.h>

namespace jive
{
    BoxModel::BoxModel(juce::ValueTree stateSource)
        : state{ std::move(stateSource) }
        , width{ state, ids::width() }
        , height{ state, ids::height() }
        , minWidth{ state, ids::minWidth() }
        , minHeight{ state, ids::minHeight() }
        , maxWidth{ state, ids::maxWidth() }
        , maxHeight{ state, ids::maxHeight() }
        , idealWidth{ state, ids::idealWidth() }
        , idealHeight{ state, ids::idealHeight() }
        , componentWidth{ state, ids::componentWidth() }
        , componentHeight{ state, ids::componentHeight() }
        , padding{ state, ids::padding() }
        , border{ state, ids::borderWidth() }
        , margin{ state, ids::margin() }
    {
        state.addListener(this);

        if (!width.exists())
            width.setAuto();
        if (!height.exists())
            height.setAuto();

        if (width.isAuto())
            componentWidth.clear();
        else
            componentWidth = width.get().toPixels(getParentBounds().getWidth());

        if (height.isAuto())
            componentHeight.clear();
        else
            componentHeight = height.get().toPixels(getParentBounds().getHeight());

        const auto informBoxModelChanged = [this] {
            listeners.call(&Listener::boxModelChanged, *this);
        };
        const auto onBoxModelChanged = [this, informBoxModelChanged] {
            if (callbackLock)
                return;

            informBoxModelChanged();
        };
        const auto handleOnValueChange = [this, informBoxModelChanged](auto& property, bool updateWidth, bool updateHeight) {
            property.onValueChange = [this, informBoxModelChanged, &property, updateWidth, updateHeight] {
                if (callbackLock)
                    return;

                JIVE_TRACE("property", property.id.toString());

                if (updateWidth && !width.isAuto())
                    componentWidth = width.get().toPixels(getParentBounds().getWidth());

                if (updateHeight && !height.isAuto())
                    componentHeight = height.get().toPixels(getParentBounds().getHeight());

                if (!property.isTransitioning())
                {
                    if (!(updateWidth || updateHeight))
                        informBoxModelChanged();
                }
            };
        };

        handleOnValueChange(width, true, false);
        handleOnValueChange(height, false, true);
        handleOnValueChange(padding, false, false);
        handleOnValueChange(margin, false, false);
        handleOnValueChange(border, false, false);
        handleOnValueChange(minWidth, false, false);
        handleOnValueChange(minHeight, false, false);
        handleOnValueChange(maxWidth, false, false);
        handleOnValueChange(maxHeight, false, false);

        idealWidth.onValueChange = onBoxModelChanged;
        idealHeight.onValueChange = onBoxModelChanged;
        componentWidth.onValueChange = [this, onBoxModelChanged] {
            if (!componentWidth.isTransitioning())
                onBoxModelChanged();
        };
        componentHeight.onValueChange = [this, onBoxModelChanged] {
            if (!componentHeight.isTransitioning())
                onBoxModelChanged();
        };

        componentWidth.setTransitionSourceProperty(width.id);
        componentHeight.setTransitionSourceProperty(height.id);

        componentWidth.onTransitionProgressed = informBoxModelChanged;
        componentHeight.onTransitionProgressed = informBoxModelChanged;
        padding.onTransitionProgressed = informBoxModelChanged;
        border.onTransitionProgressed = informBoxModelChanged;
        margin.onTransitionProgressed = [this, informBoxModelChanged] {
            ++constraintsVersion;
            informBoxModelChanged();
        };
    }

    BoxModel::~BoxModel()
    {
        state.removeListener(this);
    }

    float BoxModel::getWidth() const
    {
        if (const auto* transition = componentWidth.getTransition())
            return transition->calculateCurrent<float>();

        return componentWidth;
    }

    void BoxModel::setWidth(float newWidth)
    {
        JIVE_TRACE("newWidth", newWidth);
        componentWidth = newWidth;

        if (!state.getParent().isValid())
        {
            width = jive::Length{
                std::round(newWidth),
                jive::Length::Unit::absolute,
            };
        }
    }

    bool BoxModel::hasAutoWidth() const
    {
        return width.isAuto();
    }

    float BoxModel::getHeight() const
    {
        if (const auto* transition = componentHeight.getTransition())
            return transition->calculateCurrent<float>();

        return componentHeight;
    }

    void BoxModel::setHeight(float newHeight)
    {
        JIVE_TRACE("newHeight", newHeight);
        componentHeight = newHeight;

        if (!state.getParent().isValid())
        {
            height = jive::Length{
                std::round(newHeight),
                jive::Length::Unit::absolute,
            };
        }
    }

    bool BoxModel::hasAutoHeight() const
    {
        return height.isAuto();
    }

    void BoxModel::setSize(float newWidth, float newHeight)
    {
        JIVE_TRACE("newWidth", newWidth, "newHeight", newHeight);
        const auto widthChanged = !juce::approximatelyEqual(newWidth, componentWidth.get());
        const auto heightChanged = !juce::approximatelyEqual(newHeight, componentHeight.get());
        std::unique_ptr<ScopedCallbackLock> scopedLock;

        if (widthChanged && heightChanged)
            scopedLock = std::make_unique<ScopedCallbackLock>(*this);

        if (widthChanged)
            setWidth(newWidth);

        scopedLock = nullptr;

        if (heightChanged)
            setHeight(newHeight);
    }

    juce::BorderSize<float> BoxModel::getPadding() const
    {
        if (const auto* transition = padding.getTransition())
            return transition->calculateCurrent<juce::BorderSize<float>>();

        return padding.getOr(juce::BorderSize<float>{ 0.0f });
    }

    juce::BorderSize<float> BoxModel::getBorder() const
    {
        if (const auto* transition = border.getTransition())
            return transition->calculateCurrent<juce::BorderSize<float>>();

        return border;
    }

    juce::BorderSize<float> BoxModel::getMargin() const
    {
        if (const auto* transition = margin.getTransition())
            return transition->calculateCurrent<juce::BorderSize<float>>();

        return margin;
    }

    juce::Rectangle<float> BoxModel::getOuterBounds() const
    {
        return { getWidth(), getHeight() };
    }

    juce::Rectangle<float> BoxModel::getContentBounds() const
    {
        const auto bounds = getPadding()
                                .subtractedFrom(getBorder()
                                                    .subtractedFrom(getOuterBounds()));
        return bounds.withSize(std::max(0.0f, bounds.getWidth()),
                               std::max(0.0f, bounds.getHeight()));
    }

    juce::Rectangle<float> BoxModel::getExplicitConstraints() const
    {
        return getMaximumBounds()
            .getIntersection(juce::Rectangle<float>{
                hasAutoWidth() ? std::numeric_limits<std::uint16_t>::max() : getWidth(),
                hasAutoHeight() ? std::numeric_limits<std::uint16_t>::max() : getHeight(),
            });
    }

    juce::Rectangle<float> BoxModel::getMinimumBounds() const
    {
        return juce::Rectangle<float>{
            minWidth.get().toPixels(getParentBounds().getWidth()),
            minHeight.get().toPixels(getParentBounds().getHeight()),
        };
    }

    juce::Rectangle<float> BoxModel::getMaximumBounds() const
    {
        return {
            maxWidth.exists()
                ? maxWidth.get().toPixels(getParentBounds().getWidth())
                : std::numeric_limits<std::uint16_t>::max(),
            maxHeight.exists()
                ? maxHeight.get().toPixels(getParentBounds().getHeight())
                : std::numeric_limits<std::uint16_t>::max(),
        };
    }

    std::uint64_t BoxModel::getConstraintsVersion() const
    {
        return constraintsVersion;
    }

    void BoxModel::addListener(Listener& listener) const
    {
        const_cast<juce::ListenerList<Listener>*>(&listeners)->add(&listener);
    }

    void BoxModel::removeListener(Listener& listener) const
    {
        const_cast<juce::ListenerList<Listener>*>(&listeners)->remove(&listener);
    }

    juce::Rectangle<float> BoxModel::getParentBounds() const
    {
        const auto parent = state.getParent();

        if (!parent.isValid())
            return {};

        return {
            fromVar<float>(parent[componentWidth.id]),
            fromVar<float>(parent[componentHeight.id]),
        };
    }

    void BoxModel::valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property)
    {
        if (tree == state
            && (property == width.id
                || property == height.id
                || property == minWidth.id
                || property == minHeight.id
                || property == maxWidth.id
                || property == maxHeight.id
                || property == idealWidth.id
                || property == idealHeight.id
                || property == margin.id))
        {
            ++constraintsVersion;
        }

        forwardPropertyChange(tree,
                              property,
                              componentWidth,
                              componentHeight,
                              width,
                              height,
                              idealWidth,
                              idealHeight,
                              padding,
                              border,
                              margin,
                              minWidth,
                              minHeight,
                              maxWidth,
                              maxHeight);
    }

    BoxModel::ScopedCallbackLock::ScopedCallbackLock(BoxModel& boxModelToLock)
        : boxModel{ boxModelToLock }
    {
        boxModel.lock();
    }

    BoxModel::ScopedCallbackLock::~ScopedCallbackLock()
    {
        boxModel.unlock();
    }

    void BoxModel::lock()
    {
        callbackLock = true;
    }

    void BoxModel::unlock()
    {
        callbackLock = false;
    }
} // namespace jive

#if JIVE_UNIT_TESTS
    #include <jive_core/logging/jive_StringStreams.h>
    #include <jive_core/time/jive_Timer.h>

    #include <cstdint>
    #include <utility>

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
        testContentBounds();
        testTransitions();
        testConstraintsVersion();
    }

private:
    struct Listener : public jive::BoxModel::Listener
    {
        void boxModelChanged(jive::BoxModel&) final
        {
            ++callbackCount;
        }

        int callbackCount = 0;
    };

    void testWidthAndHeight()
    {
        beginTest("width and height");

        // Top-level component
        {
            juce::ValueTree state{ "Component" };
            const jive::BoxModel boxModel{ state };
            Listener listener;
            boxModel.addListener(listener);
            expectEquals(boxModel.getWidth(), 0.0f);
            expectEquals(boxModel.getHeight(), 0.0f);

            state.setProperty("padding", "5 10 15 20", nullptr);
            expectEquals(listener.callbackCount, 1);

            state.setProperty("border-width", "6, 12, 18, 24", nullptr);
            expectEquals(listener.callbackCount, 2);

            state.setProperty("width", 123, nullptr);
            expectEquals(listener.callbackCount, 3);

            state.setProperty("height", 987, nullptr);
            expectEquals(listener.callbackCount, 4);
        }

        // Child component
        {
            const juce::ValueTree parentState{
                "Component",
                {
                    { "width", 333 },
                    { "height", 888 },
                },
                {
                    juce::ValueTree{ "Component" },
                },
            };
            const jive::BoxModel parentBoxModel{ parentState };
            expectEquals(parentBoxModel.getWidth(), 333.0f);
            expectEquals(parentBoxModel.getHeight(), 888.0f);

            auto state = parentState.getChild(0);
            const jive::BoxModel boxModel{ state };
            expectEquals(boxModel.getWidth(), 0.0f);
            expectEquals(boxModel.getHeight(), 0.0f);

            state.setProperty("width", juce::String{ 100.0f / 3.0f } + "%", nullptr);
            expectWithinAbsoluteError(boxModel.getWidth(), 111.0f, 0.001f);

            state.setProperty("height", "50%", nullptr);
            expectEquals(boxModel.getHeight(), 444.0f);
        }
    }

    void testPadding()
    {
        beginTest("padding");

        juce::ValueTree state{ "Component" };
        const jive::BoxModel box{ state };
        expectEquals(box.getPadding(), juce::BorderSize<float>{ 0.0f });

        state.setProperty("padding", 14, nullptr);
        expectEquals(box.getPadding(), juce::BorderSize<float>{ 14.0f });

        state.setProperty("padding", "112.4 73.7", nullptr);
        expectEquals(box.getPadding(), juce::BorderSize<float>{ 112.4f, 73.7f, 112.4f, 73.7f });

        state.setProperty("padding", "14.25 8.3 1.1", nullptr);
        expectEquals(box.getPadding(), juce::BorderSize<float>{ 14.25f, 8.3f, 1.1f, 8.3f });

        state.setProperty("padding", "1 2 3 4", nullptr);
        expectEquals(box.getPadding(), juce::BorderSize<float>{ 1.0f, 4.0f, 3.0f, 2.0f });
    }

    void testBorder()
    {
        beginTest("border");

        juce::ValueTree state{ "Component" };
        const jive::BoxModel box{ state };
        expectEquals(box.getBorder(), juce::BorderSize<float>{ 0.0f });

        state.setProperty("border-width", "9 8 7 6", nullptr);
        expectEquals(box.getBorder(), juce::BorderSize<float>{ 9.0f, 6.0f, 7.0f, 8.0f });
    }

    void testMargin()
    {
        beginTest("margin");

        juce::ValueTree state{ "Component" };
        const jive::BoxModel box{ state };
        expectEquals(box.getMargin(), juce::BorderSize<float>{ 0.0f });

        state.setProperty("margin", "2 4 6 8", nullptr);
        expectEquals(box.getMargin(), juce::BorderSize<float>{ 2.0f, 8.0f, 6.0f, 4.0f });
    }

    void testContentBounds()
    {
        beginTest("content-bounds");

        const juce::ValueTree parentState{
            "Component",
            {},
            {
                juce::ValueTree{
                    "Component",
                    {
                        { "width", 200 },
                        { "height", 150 },
                        { "padding", "30" },
                        { "border-width", "5 10 15 20" },
                    },
                },
            }
        };
        const jive::BoxModel box{ parentState.getChild(0) };
        expectEquals(box.getContentBounds(),
                     juce::Rectangle<float>{
                         30.0f + 20.0f,
                         30.0f + 5.0f,
                         200.0f - 30.0f - 30.0f - 10.0f - 20.0f,
                         150.0f - 30.0f - 30.0f - 5.0f - 15.0f,
                     });
    }

    void testTransitions()
    {
        beginTest("component size transitions");
        {
            juce::ValueTree state{
                "Component",
                {
                    { "width", 0 },
                    { "height", 0 },
                    { "padding", 10 },
                    { "border-width", 10 },
                    { "margin", 10 },
                    { "transition", "width 5s, height 3s" },
                },
            };
            const jive::BoxModel boxModel{ state };
            Listener listener;
            boxModel.addListener(listener);
            expectEquals(boxModel.getWidth(), 0.0f);
            expectEquals(boxModel.getHeight(), 0.0f);
            expectEquals(boxModel.getOuterBounds(), juce::Rectangle{ 0.0f, 0.0f });
            expectEquals(boxModel.getContentBounds(), juce::Rectangle{ 20.0f, 20.0f, 0.0f, 0.0f });
            expectEquals(listener.callbackCount, 0);

            state.setProperty("width", 100, nullptr);
            expectEquals(boxModel.getWidth(), 0.0f);
            expectEquals(boxModel.getHeight(), 0.0f);
            expectEquals(boxModel.getOuterBounds(), juce::Rectangle{ 0.0f, 0.0f });
            expectEquals(boxModel.getContentBounds(), juce::Rectangle{ 20.0f, 20.0f, 0.0f, 0.0f });
            expectEquals(listener.callbackCount, 0);

            jive::FakeTime::incrementTime(juce::RelativeTime::seconds(1.0));
            expectEquals(boxModel.getWidth(), 20.0f);
            expectEquals(boxModel.getHeight(), 0.0f);
            expectEquals(boxModel.getOuterBounds(), juce::Rectangle{ 20.0f, 0.0f });
            expectEquals(boxModel.getContentBounds(), juce::Rectangle{ 20.0f, 20.0f, 0.0f, 0.0f });
            expectWithinAbsoluteError(listener.callbackCount, 60, 1);

            state.setProperty("height", 90, nullptr);
            jive::FakeTime::incrementTime(juce::RelativeTime::seconds(1.0));
            expectEquals(boxModel.getWidth(), 40.0f);
            expectEquals(boxModel.getHeight(), 30.0f);
            expectWithinAbsoluteError(listener.callbackCount, 180, 2);
        }

        beginTest("padding transitions");
        {
            juce::ValueTree state{
                "Component",
                {
                    { "width", 100 },
                    { "height", 100 },
                    { "padding", 0 },
                    { "border-width", 10 },
                    { "margin", 10 },
                    { "transition", "padding 200ms" },
                },
            };
            const jive::BoxModel boxModel{ state };
            Listener listener;
            boxModel.addListener(listener);
            expectEquals(boxModel.getPadding(), juce::BorderSize{ 0.0f });
            expectEquals(boxModel.getOuterBounds(), juce::Rectangle{ 100.0f, 100.0f });
            expectEquals(boxModel.getContentBounds(), juce::Rectangle{ 10.0f, 10.0f, 80.0f, 80.0f });
            expectEquals(listener.callbackCount, 0);

            state.setProperty("padding", "10 20 30 40", nullptr);
            expectEquals(boxModel.getPadding(), juce::BorderSize{ 0.0f });
            expectEquals(boxModel.getOuterBounds(), juce::Rectangle{ 100.0f, 100.0f });
            expectEquals(boxModel.getContentBounds(), juce::Rectangle{ 10.0f, 10.0f, 80.0f, 80.0f });
            expectEquals(listener.callbackCount, 0);

            jive::FakeTime::incrementTime(juce::RelativeTime::seconds(0.1));
            expectEquals(boxModel.getPadding(), juce::BorderSize{ 5.0f, 20.0f, 15.0f, 10.0f });
            expectEquals(boxModel.getOuterBounds(), juce::Rectangle{ 100.0f, 100.0f });
            expectEquals(boxModel.getContentBounds(), juce::Rectangle{ 30.0f, 15.0f, 50.0f, 60.0f });
            expectWithinAbsoluteError(listener.callbackCount, 5, 1);
        }

        beginTest("border transitions");
        {
            juce::ValueTree state{
                "Component",
                {
                    { "width", 100 },
                    { "height", 100 },
                    { "padding", 10 },
                    { "border-width", 0 },
                    { "margin", 10 },
                    { "transition", "border-width 10s" },
                },
            };
            const jive::BoxModel boxModel{ state };
            Listener listener;
            boxModel.addListener(listener);
            expectEquals(boxModel.getBorder(), juce::BorderSize{ 0.0f });
            expectEquals(boxModel.getOuterBounds(), juce::Rectangle{ 100.0f, 100.0f });
            expectEquals(boxModel.getContentBounds(), juce::Rectangle{ 10.0f, 10.0f, 80.0f, 80.0f });
            expectEquals(listener.callbackCount, 0);

            state.setProperty("border-width", "100 200 300 400", nullptr);
            expectEquals(boxModel.getBorder(), juce::BorderSize{ 0.0f });
            expectEquals(boxModel.getOuterBounds(), juce::Rectangle{ 100.0f, 100.0f });
            expectEquals(boxModel.getContentBounds(), juce::Rectangle{ 10.0f, 10.0f, 80.0f, 80.0f });
            expectEquals(listener.callbackCount, 0);

            jive::FakeTime::incrementTime(juce::RelativeTime::seconds(1.0));
            expectEquals(boxModel.getBorder(), juce::BorderSize{ 10.0f, 40.0f, 30.0f, 20.0f });
            expectEquals(boxModel.getOuterBounds(), juce::Rectangle{ 100.0f, 100.0f });
            expectEquals(boxModel.getContentBounds(), juce::Rectangle{ 50.0f, 20.0f, 20.0f, 40.0f });
            expectWithinAbsoluteError(listener.callbackCount, 60, 1);
        }

        beginTest("margin transitions");
        {
            juce::ValueTree state{
                "Component",
                {
                    { "width", 100 },
                    { "height", 100 },
                    { "padding", 10 },
                    { "border-width", 10 },
                    { "margin", 0 },
                    { "transition", "margin 10s" },
                },
            };
            const jive::BoxModel boxModel{ state };
            Listener listener;
            boxModel.addListener(listener);
            expectEquals(boxModel.getMargin(), juce::BorderSize{ 0.0f });
            expectEquals(boxModel.getOuterBounds(), juce::Rectangle{ 100.0f, 100.0f });
            expectEquals(boxModel.getContentBounds(), juce::Rectangle{ 20.0f, 20.0f, 60.0f, 60.0f });
            expectEquals(listener.callbackCount, 0);

            state.setProperty("margin", "100 200 300 400", nullptr);
            expectEquals(boxModel.getMargin(), juce::BorderSize{ 0.0f });
            expectEquals(boxModel.getOuterBounds(), juce::Rectangle{ 100.0f, 100.0f });
            expectEquals(boxModel.getContentBounds(), juce::Rectangle{ 20.0f, 20.0f, 60.0f, 60.0f });
            expectEquals(listener.callbackCount, 0);

            jive::FakeTime::incrementTime(juce::RelativeTime::seconds(1.0));
            expectEquals(boxModel.getMargin(), juce::BorderSize{ 10.0f, 40.0f, 30.0f, 20.0f });
            expectEquals(boxModel.getOuterBounds(), juce::Rectangle{ 100.0f, 100.0f });
            expectEquals(boxModel.getContentBounds(), juce::Rectangle{ 20.0f, 20.0f, 60.0f, 60.0f });
            expectWithinAbsoluteError(listener.callbackCount, 60, 1);
        }
    }

    void testConstraintsVersion()
    {
        beginTest("constraints version / changes");
        {
            const juce::ValueTree parentState{
                "Component",
                {
                    { "width", 200 },
                    { "height", 200 },
                },
                {
                    juce::ValueTree{ "Component" },
                },
            };
            auto state = parentState.getChild(0);
            jive::BoxModel boxModel{ state };
            auto version = boxModel.getConstraintsVersion();

            boxModel.setSize(100.0f, 50.0f);
            state.setProperty("padding", 10, nullptr);
            state.setProperty("border-width", 5, nullptr);
            expectEquals(boxModel.getConstraintsVersion(), version);

            for (const auto* property : {
                     "width",
                     "height",
                     "min-width",
                     "min-height",
                     "max-width",
                     "max-height",
                     "margin",
                     "jive::ideal-width",
                     "jive::ideal-height",
                 })
            {
                state.setProperty(property, 20, nullptr);
                expectNotEquals(boxModel.getConstraintsVersion(), version, property);
                version = boxModel.getConstraintsVersion();
            }
        }

        beginTest("constraints version / margin transitions");
        {
            juce::ValueTree state{
                "Component",
                {
                    { "width", 100 },
                    { "height", 100 },
                    { "margin", 0 },
                    { "transition", "margin 10s" },
                },
            };
            const jive::BoxModel boxModel{ state };
            state.setProperty("margin", 100, nullptr);
            const auto version = boxModel.getConstraintsVersion();

            jive::FakeTime::incrementTime(juce::RelativeTime::seconds(1.0));
            expectNotEquals(boxModel.getConstraintsVersion(), version);
        }
    }
};

static BoxModelUnitTest boxModelUnitTest;
#endif
