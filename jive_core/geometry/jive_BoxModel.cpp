#include "jive_BoxModel.h"

namespace jive
{
    BoxModel::BoxModel(juce::ValueTree stateSource)
        : state{ stateSource }
        , width{ state, "width" }
        , height{ state, "height" }
        , minWidth{ state, "min-width" }
        , minHeight{ state, "min-height" }
        , maxWidth{ state, "max-width" }
        , maxHeight{ state, "max-height" }
        , idealWidth{ state, "ideal-width" }
        , idealHeight{ state, "ideal-height" }
        , componentWidth{ state, "component-width" }
        , componentHeight{ state, "component-height" }
        , padding{ state, "padding" }
        , border{ state, "border-width" }
        , margin{ state, "margin" }
        , isValid{ state, "box-model-valid" }
        , callbackLock{ state, "box-model-callback-lock" }
    {
        if (!width.exists())
            width.setAuto();
        if (!height.exists())
            height.setAuto();
        if (!isValid.exists())
            isValid = true;
        if (!componentWidth.exists())
            componentWidth = calculateComponentWidth();
        if (!componentHeight.exists())
            componentHeight = calculateComponentHeight();

        const auto informBoxModelChanged = [this]() {
            listeners.call(&Listener::boxModelChanged, *this);
        };
        const auto onBoxModelChanged = [this, informBoxModelChanged]() {
            if (callbackLock.get())
                return;

            isValid = true;
            informBoxModelChanged();
            invalidateParent();
        };
        const auto handleOnValueChange = [this, informBoxModelChanged](auto& property, bool updateWidth, bool updateHeight) {
            const auto recalculateSize = [this, informBoxModelChanged, &property, updateWidth, updateHeight] {
                if (callbackLock.get())
                    return;

                const auto widthBefore = componentWidth.get();

                if (updateWidth)
                    componentWidth = calculateComponentWidth();

                const auto heightBefore = componentHeight.get();

                if (updateHeight)
                    componentHeight = calculateComponentHeight();

                const auto isSameSize = juce::approximatelyEqual(componentWidth.get(), widthBefore)
                                     && juce::approximatelyEqual(componentHeight.get(), heightBefore);

                if (!property.isTransitioning())
                {
                    informBoxModelChanged();

                    if (!isSameSize)
                        invalidateParent();
                }
            };

            property.onValueChange = recalculateSize;
        };

        handleOnValueChange(width, true, false);
        handleOnValueChange(height, false, true);
        handleOnValueChange(padding, true, true);
        handleOnValueChange(border, true, true);
        handleOnValueChange(minWidth, true, false);
        handleOnValueChange(minHeight, false, true);
        handleOnValueChange(maxWidth, true, false);
        handleOnValueChange(maxHeight, false, true);

        idealWidth.onValueChange = onBoxModelChanged;
        idealHeight.onValueChange = onBoxModelChanged;
        componentWidth.onValueChange = [this, onBoxModelChanged]() {
            if (!componentWidth.isTransitioning())
                onBoxModelChanged();
        };
        componentHeight.onValueChange = [this, onBoxModelChanged]() {
            if (!componentHeight.isTransitioning())
                onBoxModelChanged();
        };
        margin.onValueChange = [this, onBoxModelChanged]() {
            if (!margin.isTransitioning())
                onBoxModelChanged();
        };

        isValid.onValueChange = [this]() {
            if (callbackLock.get())
                return;

            if (!isValid.get())
                listeners.call(&Listener::boxModelInvalidated, *this);
        };

        componentWidth.setTransitionSourceProperty(width.id);
        componentHeight.setTransitionSourceProperty(height.id);

        componentWidth.onTransitionProgressed = informBoxModelChanged;
        componentHeight.onTransitionProgressed = informBoxModelChanged;
        padding.onTransitionProgressed = informBoxModelChanged;
        border.onTransitionProgressed = informBoxModelChanged;
        margin.onTransitionProgressed = informBoxModelChanged;

        state.addListener(this);
    }

    float BoxModel::getWidth() const
    {
        if (auto* transition = componentWidth.getTransition())
            return transition->calculateCurrent<float>();

        return componentWidth;
    }

    void BoxModel::setWidth(float newWidth)
    {
        componentWidth = newWidth;

        if (!state.getParent().isValid())
            width = juce::String{ juce::roundToInt(newWidth) };
    }

    bool BoxModel::hasAutoWidth() const
    {
        return width.isAuto();
    }

    float BoxModel::getHeight() const
    {
        if (auto* transition = componentHeight.getTransition())
            return transition->calculateCurrent<float>();

        return componentHeight;
    }

    void BoxModel::setHeight(float newHeight)
    {
        componentHeight = newHeight;

        if (!state.getParent().isValid())
            height = juce::String{ juce::roundToInt(newHeight) };
    }

    bool BoxModel::hasAutoHeight() const
    {
        return height.isAuto();
    }

    void BoxModel::setSize(float newWidth, float newHeight)
    {
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
        if (auto* transition = padding.getTransition())
            return transition->calculateCurrent<juce::BorderSize<float>>();

        return padding;
    }

    juce::BorderSize<float> BoxModel::getBorder() const
    {
        if (auto* transition = border.getTransition())
            return transition->calculateCurrent<juce::BorderSize<float>>();

        return border;
    }

    juce::BorderSize<float> BoxModel::getMargin() const
    {
        if (auto* transition = margin.getTransition())
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
        return bounds.withSize(juce::jmax(0.0f, bounds.getWidth()),
                               juce::jmax(0.0f, bounds.getHeight()));
    }

    juce::Rectangle<float> BoxModel::getMinimumBounds() const
    {
        return juce::Rectangle<float>{
            minWidth.toPixels(getParentBounds()),
            minHeight.toPixels(getParentBounds()),
        };
    }

    juce::Rectangle<float> BoxModel::getMaximumBounds() const
    {
        return {
            maxWidth.exists() ? maxWidth.toPixels(getParentBounds()) : -1.0f,
            maxHeight.exists() ? maxHeight.toPixels(getParentBounds()) : -1.0f,
        };
    }

    void BoxModel::addListener(Listener& listener) const
    {
        const_cast<juce::ListenerList<Listener>*>(&listeners)->add(&listener);
    }

    void BoxModel::removeListener(Listener& listener) const
    {
        const_cast<juce::ListenerList<Listener>*>(&listeners)->remove(&listener);
    }

    void BoxModel::invalidate()
    {
        isValid = true;
        isValid = false;
    }

    juce::Rectangle<float> BoxModel::getParentBounds() const
    {
        if (state.getParent().isValid())
        {
            const Property<float> parentWidth{ state.getParent(), componentWidth.id };
            const Property<float> parentHeight{ state.getParent(), componentHeight.id };
            return { parentWidth.get(), parentHeight.get() };
        }

        return {};
    }

    float BoxModel::calculateComponentWidth() const
    {
        if (hasAutoWidth())
            return getPadding().getLeftAndRight() + getBorder().getLeftAndRight();

        return width.toPixels(getParentBounds());
    }

    float BoxModel::calculateComponentHeight() const
    {
        if (hasAutoHeight())
            return getPadding().getTopAndBottom() + getBorder().getTopAndBottom();

        return height.toPixels(getParentBounds());
    }

    void BoxModel::invalidateParent()
    {
        if (auto parent = state.getParent();
            parent.isValid())
        {
            Property<bool> parentIsValid{ parent, isValid.id };
            parentIsValid = true;
            parentIsValid = false;
        }
    }

    void BoxModel::valueTreeParentChanged(juce::ValueTree&)
    {
        if (!width.isPixels())
            componentWidth = calculateComponentWidth();
        if (!height.isPixels())
            componentHeight = calculateComponentHeight();
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
        callbackLock.clear();
    }
} // namespace jive

#if JIVE_UNIT_TESTS
    #include <jive_core/logging/jive_StringStreams.h>
    #include <jive_core/time/jive_Timer.h>

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
    }

private:
    void testWidthAndHeight()
    {
        beginTest("width and height");

        // Top-level component
        {
            juce::ValueTree state{ "Component" };
            jive::BoxModel boxModel{ state };
            expectEquals(boxModel.getWidth(), 0.0f);
            expectEquals(boxModel.getHeight(), 0.0f);

            state.setProperty("padding", "5 10 15 20", nullptr);
            expectEquals(boxModel.getWidth(), 10.0f + 20.0f);
            expectEquals(boxModel.getHeight(), 5.0f + 15.0f);

            state.setProperty("border-width", "6, 12, 18, 24", nullptr);
            expectEquals(boxModel.getWidth(), 10.0f + 20.0f + 12.0f + 24.0f);
            expectEquals(boxModel.getHeight(), 5.0f + 15.0f + 6.0f + 18.0f);

            state.setProperty("width", 123, nullptr);
            expectEquals(boxModel.getWidth(), 123.0f);

            state.setProperty("height", 987, nullptr);
            expectEquals(boxModel.getHeight(), 987.0f);
        }

        // Child component
        {
            juce::ValueTree parentState{
                "Component",
                {
                    { "width", 333 },
                    { "height", 888 },
                },
                {
                    juce::ValueTree{ "Component" },
                },
            };
            jive::BoxModel parentBoxModel{ parentState };
            expectEquals(parentBoxModel.getWidth(), 333.0f);
            expectEquals(parentBoxModel.getHeight(), 888.0f);

            auto state = parentState.getChild(0);
            jive::BoxModel boxModel{ state };
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

    void testContentBounds()
    {
        beginTest("content-bounds");

        juce::ValueTree parentState{
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
        jive::BoxModel box{ parentState.getChild(0) };
        expectEquals(box.getContentBounds(),
                     juce::Rectangle<float>{
                         30.0f + 20.0f,
                         30.0f + 5.0f,
                         200.0f - 30.0f - 30.0f - 10.0f - 20.0f,
                         150.0f - 30.0f - 30.0f - 5.0f - 15.0f,
                     });
    }

    struct Listener : public jive::BoxModel::Listener
    {
        void boxModelChanged(jive::BoxModel&) final
        {
            ++callbackCount;
        }

        int callbackCount = 0;
    };

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
            jive::BoxModel boxModel{ state };
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
            jive::BoxModel boxModel{ state };
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
            jive::BoxModel boxModel{ state };
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
            jive::BoxModel boxModel{ state };
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
};

static BoxModelUnitTest boxModelUnitTest;
#endif
