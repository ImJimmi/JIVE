#include <jive_layouts/jive_layouts.h>

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
        , componentSize{ state, "component-size" }
        , padding{ state, "padding" }
        , border{ state, "border-width" }
        , margin{ state, "margin" }
        , isValid{ state, "box-model-valid" }
    {
        if (!width.exists())
            width.setAuto();
        if (!height.exists())
            height.setAuto();
        if (!isValid.exists())
            isValid = true;
        if (!componentSize.exists())
        {
            componentSize = juce::Rectangle{
                calculateComponentWidth(),
                calculateComponentHeight(),
            };
        }

        const auto onBoxModelChanged = [this]() {
            isValid = true;
            listeners.call(&Listener::boxModelChanged, *this);
            invalidateParent();
        };
        const auto recalculateSize = [this, onBoxModelChanged]() {
            const auto sizeBefore = componentSize.get();
            componentSize = juce::Rectangle{
                calculateComponentWidth(),
                calculateComponentHeight(),
            };

            if (componentSize.get() == sizeBefore)
                onBoxModelChanged();
        };

        width.onValueChange = recalculateSize;
        height.onValueChange = recalculateSize;
        padding.onValueChange = recalculateSize;
        border.onValueChange = recalculateSize;
        margin.onValueChange = recalculateSize;
        minWidth.onValueChange = recalculateSize;
        minHeight.onValueChange = recalculateSize;
        maxWidth.onValueChange = recalculateSize;
        maxHeight.onValueChange = recalculateSize;

        idealWidth.onValueChange = onBoxModelChanged;
        idealHeight.onValueChange = onBoxModelChanged;
        componentSize.onValueChange = onBoxModelChanged;

        isValid.onValueChange = [this]() {
            if (!isValid.get())
                listeners.call(&Listener::boxModelInvalidated, *this);
        };
    }

    float BoxModel::getWidth() const
    {
        return componentSize.get().getWidth();
    }

    bool BoxModel::hasAutoWidth() const
    {
        return width.isAuto();
    }

    float BoxModel::getHeight() const
    {
        return componentSize.get().getHeight();
    }

    bool BoxModel::hasAutoHeight() const
    {
        return height.isAuto();
    }

    void BoxModel::setSize(float newWidth, float newHeight)
    {
        componentSize = juce::Rectangle{ newWidth, newHeight };

        if (!state.getParent().isValid())
        {
            width = juce::String{ juce::roundToInt(newWidth) };
            height = juce::String{ juce::roundToInt(newHeight) };
        }
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

    juce::Rectangle<float> BoxModel::getOuterBounds() const
    {
        return componentSize.get();
    }

    juce::Rectangle<float> BoxModel::getContentBounds() const
    {
        const auto bounds = padding
                                .get()
                                .subtractedFrom(border
                                                    .get()
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

    juce::Rectangle<float> BoxModel::getParentBounds() const
    {
        if (state.getParent().isValid())
        {
            const Property<juce::Rectangle<float>> parentSize{ state.getParent(), componentSize.id };
            return parentSize.get();
        }

        return juce::Rectangle<float>{};
    }

    float BoxModel::calculateComponentWidth() const
    {
        if (width.isAuto())
            return padding.get().getLeftAndRight() + border.get().getLeftAndRight();

        return width.toPixels(getParentBounds());
    }

    float BoxModel::calculateComponentHeight() const
    {
        if (height.isAuto())
            return padding.get().getTopAndBottom() + border.get().getTopAndBottom();

        return height.toPixels(getParentBounds());
    }

    void BoxModel::invalidateParent()
    {
        if (!state.getParent().isValid())
            return;

        BoxModel parent{ state.getParent() };

        parent.isValid = true;
        parent.isValid = false;
    }
} // namespace jive

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
        testContentBounds();
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
};

static BoxModelUnitTest boxModelUnitTest;
#endif
