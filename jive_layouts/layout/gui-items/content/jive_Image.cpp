#include "jive_Image.h"

#include <jive_core/graphics/jive_CurrentColour.h>
#include <jive_layouts/layout/gui-items/jive_CommonGuiItem.h>

namespace jive
{
    Image::Image(std::unique_ptr<GuiItem> itemToDecorate)
        : GuiItemDecorator{ std::move(itemToDecorate) }
        , source{ state, ids::source() }
        , placement{ state, ids::placement() }
        , width{ state, ids::width() }
        , height{ state, ids::height() }
        , idealWidth{ state, ids::idealWidth() }
        , idealHeight{ state, ids::idealHeight() }
        , boxModel{ toType<CommonGuiItem>()->boxModel }
    {
        const BoxModel::ScopedCallbackLock boxModelLock{ jive::boxModel(*this) };

        source.onValueChange = [this] {
            updateImageComponentDrawable();
        };
        placement.onValueChange = [this] {
            if (auto* image = dynamic_cast<juce::ImageComponent*>(childComponent.get()))
                image->setImagePlacement(placement.getOr(juce::RectanglePlacement::centred));
        };

        updateImageComponentDrawable();
        boxModel.addListener(*this);
        state.addListener(this);
    }

    Image::~Image()
    {
        state.removeListener(this);
        boxModel.removeListener(*this);

        if (getComponent() != nullptr)
            getComponent()->removeComponentListener(this);
    }

    bool Image::isContainer() const
    {
        return false;
    }

    bool Image::isContent() const
    {
        return true;
    }

    static void removeUnserialisableProperties(juce::ValueTree tree)
    {
        for (auto i = 0; i < tree.getNumProperties(); ++i)
        {
            const auto name = tree.getPropertyName(i);

            if (tree[name].isArray()
                || tree[name].isMethod()
                || tree[name].isObject())
            {
                tree.removeProperty(name, nullptr);
                --i;
            }
        }

        for (auto i = 0; i < tree.getNumChildren(); ++i)
            removeUnserialisableProperties(tree.getChild(i));
    }

    Drawable Image::getDrawable() const
    {
        const auto isInlineSVG = state.getType().toString().compareIgnoreCase("svg") == 0;

        if (isInlineSVG)
        {
            const auto stringSafe = state.createCopy();
            removeUnserialisableProperties(stringSafe);

            return Drawable{ stringSafe.toXmlString() };
        }

        return source.get();
    }

    ImageComponent& Image::getImageComponent()
    {
        return dynamic_cast<ImageComponent&>(*getComponent());
    }

    const ImageComponent& Image::getImageComponent() const
    {
        return dynamic_cast<const ImageComponent&>(*getComponent());
    }

    void Image::componentMovedOrResized(juce::Component& componentThatWasMovedOrResized, bool, bool)
    {
        if (&componentThatWasMovedOrResized != getComponent().get())
            return;

        if (childComponent != nullptr)
        {
            childComponent->setBounds(getComponent()->getLocalBounds());

            if (auto* drawable = dynamic_cast<juce::Drawable*>(childComponent.get()))
            {
                drawable->
#if JIVE_JUCE_9
                    setDrawableTransformToFit
#else
                    setTransformToFit
#endif
                    (getComponent()->getLocalBounds().toFloat(),
                     placement.getOr(juce::RectanglePlacement::centred));
            }
        }
    }

    void Image::boxModelChanged(BoxModel& boxModelThatChanged)
    {
        if (boxModelThatChanged.state != state)
            return;

        if (childComponent != nullptr)
            childComponent->setBounds(getComponent()->getLocalBounds());

        idealWidth = calculateRequiredWidth();
        idealHeight = calculateRequiredHeight();
    }

    void Image::valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& id)
    {
        forwardPropertyChange(tree, id, source, placement);

        if ((state.getType().toString().compareIgnoreCase("svg") == 0) && (!changingChild && childComponent != nullptr))

        {
            updateImageComponentDrawable();

            childComponent->setBounds(getComponent()->getLocalBounds());

            if (auto* drawable = dynamic_cast<juce::Drawable*>(childComponent.get()))
            {
                drawable->
#if JIVE_JUCE_9
                    setDrawableTransformToFit
#else
                    setTransformToFit
#endif
                    (getComponent()->getLocalBounds().toFloat(),
                     placement.getOr(juce::RectanglePlacement::centred));
            }
        }
    }

    float Image::calculateRequiredWidth() const
    {
        return getImageComponent().getMinimumRequiredWidth();
    }

    float Image::calculateRequiredHeight() const
    {
        return getImageComponent().getMinimumRequiredHeight();
    }

    static juce::Rectangle<float> getViewBox(const juce::XmlElement& svg)
    {
        juce::StringArray values;
        values.addTokens(svg.getStringAttribute("viewBox"), " ,", "");
        values.removeEmptyStrings();

        if (values.size() != 4)
            return {};

        return {
            values[0].getFloatValue(),
            values[1].getFloatValue(),
            values[2].getFloatValue(),
            values[3].getFloatValue(),
        };
    }

    static bool hasExplicitLength(const juce::XmlElement& svg, juce::StringRef attribute)
    {
        const auto length = svg.getStringAttribute(attribute).trim();

        return length.isNotEmpty() && !length.equalsIgnoreCase("auto");
    }

    static juce::Rectangle<float> getIntrinsicBounds(const juce::XmlElement& svg)
    {
        if (hasExplicitLength(svg, "width") || hasExplicitLength(svg, "height"))
            return {};

        return getViewBox(svg);
    }

    void Image::updateImageComponentDrawable()
    {
        const auto drawable = getDrawable();
        std::unique_ptr<juce::Drawable> result;
        juce::Rectangle<float> intrinsicBounds;
        juce::Rectangle<float> viewBox;

        if (drawable.isImage())
            result = std::make_unique<juce::DrawableImage>(static_cast<juce::Image>(drawable));

        if (drawable.isSVG())
        {
            const auto svgString = replaceCurrentColourWithStandIn(static_cast<juce::String>(drawable));
            const auto xml = juce::parseXML(svgString);
#if JIVE_JUCE_9
            result = juce::Drawable::createFromSVGString(svgString);
#else
            result = juce::Drawable::createFromSVG(*xml);
#endif

            intrinsicBounds = getIntrinsicBounds(*xml);
            viewBox = getViewBox(*xml);
        }

        getImageComponent().setDrawable(std::move(result), intrinsicBounds, viewBox);

        idealWidth = juce::String{ calculateRequiredWidth() } + "px";
        idealHeight = juce::String{ calculateRequiredHeight() } + "px";
    }
} // namespace jive

#if JIVE_UNIT_TESTS
    #include <jive_layouts/layout/interpreter/jive_Interpreter.h>

class ImageTest : public juce::UnitTest
{
public:
    ImageTest()
        : juce::UnitTest{ "jive::Image", "jive" }
    {
    }

    void runTest() final
    {
        testImage();
        testAutoSize();
        testChildComponent();
        testSVG();
        testInlineSVG();
        testSVGFillRespected();
        testSVGFillInherited();
        testSVGContentStaysPutWhenResized();
        testInlineSVGAutoSize();
        testSVGViewBoxScaledToFit();
        testSVGGradientFillSpansElement();
        testSVGShadowFollowsShapes();
        testSVGSourceReplaced();
        testSVGCurrentColourFollowsForeground();
    }

    void testSVGCurrentColourFollowsForeground()
    {
        beginTest("svg / currentColor is drawn in the element's foreground, and follows changes to it");

        juce::Component root;
        root.setSize(100, 100);
        jive::LookAndFeel lookAndFeel{ root };
        lookAndFeel.applyTheme(jive::Theme::juce);
        lookAndFeel.addStyles(".red", jive::Styles{}.withForeground(juce::Colours::red));
        lookAndFeel.addStyles(".blue", jive::Styles{}.withForeground(juce::Colours::blue));

        jive::Interpreter interpreter;
        auto state = jive::parseXML(R"(
            <Component class="red" width="100" height="100">
                <svg width="20" height="40">
                    <rect width="10" height="20" fill="currentColor" />
                    <rect x="10" width="10" height="20" fill="#00FF00" />
                    <path d="M 0 30 L 20 30" fill="none" stroke="currentColor" stroke-width="10" />
                </svg>
            </Component>
        )");
        auto parent = interpreter.interpret(state);
        auto* parentComponent = parent->getComponent().get();
        root.addAndMakeVisible(*parentComponent);
        parentComponent->setBounds(0, 0, 100, 100);

        auto* svgComponent = parent->getChildren()[0]->getComponent().get();
        const auto snapshot = [svgComponent] {
            return svgComponent->createComponentSnapshot(svgComponent->getLocalBounds());
        };
        expectEquals(snapshot().getPixelAt(5, 10), juce::Colours::red);
        expectEquals(snapshot().getPixelAt(15, 10), juce::Colour{ 0xff00ff00 });
        expectEquals(snapshot().getPixelAt(10, 30), juce::Colours::red);

        state.setProperty("class", "blue", nullptr);
        expectEquals(snapshot().getPixelAt(5, 10), juce::Colours::blue);
        expectEquals(snapshot().getPixelAt(15, 10), juce::Colour{ 0xff00ff00 });
        expectEquals(snapshot().getPixelAt(10, 30), juce::Colours::blue);

        root.removeChildComponent(parentComponent);
    }

    void testSVGSourceReplaced()
    {
        beginTest("svg / replacing the source shows the new one");

        juce::Component root;
        root.setSize(100, 100);
        const jive::LookAndFeel lookAndFeel{ root };

        jive::Interpreter interpreter;
        auto state = jive::parseXML(R"(
            <Component width="100" height="100">
                <Image width="100" height="100" />
            </Component>
        )");
        auto parent = interpreter.interpret(state);
        auto* parentComponent = parent->getComponent().get();
        root.addAndMakeVisible(*parentComponent);
        parentComponent->setBounds(0, 0, 100, 100);

        const auto setSource = [&state](const juce::String& colour) {
            state.getChild(0).setProperty("source",
                                          R"(<svg viewBox="0 0 10 10" width="100" height="100"><rect width="10" height="10" fill=")"
                                              + colour
                                              + R"(" /></svg>)",
                                          nullptr);
        };
        const auto centrePixel = [&parent] {
            auto* imageComponent = parent->getChildren()[0]->getComponent().get();
            return imageComponent->createComponentSnapshot(imageComponent->getLocalBounds()).getPixelAt(50, 50);
        };
        setSource("red");
        setSource("blue");
        expectEquals(centrePixel(), juce::Colours::blue);

        state.getChild(0).removeProperty("source", nullptr);
        expectEquals(centrePixel(), juce::Colours::transparentBlack);

        root.removeChildComponent(parentComponent);
    }

    void testSVGGradientFillSpansElement()
    {
        beginTest("svg / a gradient fill spans the element, whatever the size of its view-box");

        juce::Component root;
        root.setSize(100, 100);
        jive::LookAndFeel lookAndFeel{ root };
        lookAndFeel.addStyles("#tinted",
                              jive::Styles{}
                                  .withFill(jive::fromVar<jive::Fill>("linear-gradient(to right, #FF0000, #0000FF)")));

        jive::Interpreter interpreter;
        auto parent = interpreter.interpret(jive::parseXML(R"(
            <Component width="100" height="100">
                <svg id="tinted" viewBox="0 0 10 10" width="100" height="100">
                    <rect width="10" height="10" />
                </svg>
            </Component>
        )"));
        auto* parentComponent = parent->getComponent().get();
        root.addAndMakeVisible(*parentComponent);
        parentComponent->setBounds(0, 0, 100, 100);

        auto* svgComponent = parent->getChildren()[0]->getComponent().get();
        const auto snapshot = svgComponent->createComponentSnapshot(svgComponent->getLocalBounds());
        expectGreaterThan(snapshot.getPixelAt(5, 50).getRed(), snapshot.getPixelAt(5, 50).getBlue());
        expectGreaterThan(snapshot.getPixelAt(95, 50).getBlue(), snapshot.getPixelAt(95, 50).getRed());

        root.removeChildComponent(parentComponent);
    }

    void testSVGShadowFollowsShapes()
    {
        beginTest("svg / a shadow is cast from the outline of its shapes, rather than its bounds");

        juce::Component root;
        root.setSize(100, 100);
        jive::LookAndFeel lookAndFeel{ root };
        lookAndFeel.addStyles("#backdrop",
                              jive::Styles{}
                                  .withBackground(juce::Colours::white));
        lookAndFeel.addStyles("#glowing",
                              jive::Styles{}
                                  .withFill(juce::Colours::blue)
                                  .withShadow(jive::fromVar<jive::Shadow>("0 0 0 5px #FF0000")));

        jive::Interpreter interpreter;
        auto parent = interpreter.interpret(jive::parseXML(R"(
            <Component id="backdrop" width="100" height="100">
                <svg id="glowing" viewBox="0 0 10 10" width="50" height="50">
                    <rect x="4" y="4" width="2" height="2" />
                </svg>
            </Component>
        )"));
        auto* parentComponent = parent->getComponent().get();
        root.addAndMakeVisible(*parentComponent);
        parentComponent->setBounds(0, 0, 100, 100);

        // The image asks for its shadow while painting, and the shadow is
        // added on the next frame, so it's only drawn from then on.
        juce::ignoreUnused(parentComponent->createComponentSnapshot(parentComponent->getLocalBounds()));
        lookAndFeel.simulateVBlank();
        const auto snapshot = parentComponent->createComponentSnapshot(parentComponent->getLocalBounds());
        expectEquals(snapshot.getPixelAt(25, 25), juce::Colours::blue);
        expectEquals(snapshot.getPixelAt(17, 25), juce::Colours::red);
        expectEquals(snapshot.getPixelAt(5, 5), juce::Colours::white);

        root.removeChildComponent(parentComponent);
    }

    void testSVGViewBoxScaledToFit()
    {
        beginTest("svg / a view-box is scaled to fit the element, keeping its aspect ratio");

        juce::Component root;
        root.setSize(100, 100);
        const jive::LookAndFeel lookAndFeel{ root };

        jive::Interpreter interpreter;
        auto parent = interpreter.interpret(jive::parseXML(R"(
            <Component width="100" height="100">
                <svg viewBox="0 0 200 100" width="100" height="100">
                    <rect x="100" y="0" width="100" height="100" fill="blue" />
                </svg>
            </Component>
        )"));
        auto* parentComponent = parent->getComponent().get();
        root.addAndMakeVisible(*parentComponent);
        parentComponent->setBounds(0, 0, 100, 100);

        auto* svgComponent = parent->getChildren()[0]->getComponent().get();
        const auto snapshot = svgComponent->createComponentSnapshot(svgComponent->getLocalBounds());
        expectEquals(snapshot.getPixelAt(75, 50), juce::Colours::blue);
        expectEquals(snapshot.getPixelAt(75, 30), juce::Colours::blue);
        expectNotEquals(snapshot.getPixelAt(25, 50), juce::Colours::blue);
        expectNotEquals(snapshot.getPixelAt(75, 10), juce::Colours::blue);
        expectNotEquals(snapshot.getPixelAt(75, 90), juce::Colours::blue);

        root.removeChildComponent(parentComponent);
    }

    void testInlineSVGAutoSize()
    {
        beginTest("svg / an inline SVG with no explicit size uses its view-box");

        jive::Interpreter interpreter;
        auto parent = interpreter.interpret(jive::parseXML(R"(
            <Component width="500" height="500" align-items="flex-start">
                <svg viewBox="0 0 24 24">
                    <rect x="5" y="7" width="10" height="8" />
                </svg>
            </Component>
        )"));
        const auto& boxModel = jive::boxModel(*parent->getChildren()[0]);
        expectEquals(boxModel.getWidth(), 24.0f);
        expectEquals(boxModel.getHeight(), 24.0f);
    }

private:
    #if JIVE_JUCE_9
    using DrawableComponent = juce::DrawableComponent;
    #else
    using DrawableComponent = juce::Drawable;
    #endif

    void testSVGFillRespected()
    {
        beginTest("svg / an explicit fill is respected rather than overridden by the theme");

        juce::Component root;
        root.setSize(100, 100);
        const jive::LookAndFeel lookAndFeel{ root };

        jive::Interpreter interpreter;
        auto parent = interpreter.interpret(jive::parseXML(R"(
            <Component width="100" height="100">
                <svg width="20" height="20">
                    <rect width="20" height="20" fill="blue" />
                </svg>
            </Component>
        )"));
        auto* parentComponent = parent->getComponent().get();
        root.addAndMakeVisible(*parentComponent);
        parentComponent->setBounds(0, 0, 100, 100);

        auto* svgComponent = parent->getChildren()[0]->getComponent().get();
        const auto snapshot = svgComponent->createComponentSnapshot(svgComponent->getLocalBounds());
        expectEquals(snapshot.getPixelAt(10, 10), juce::Colours::blue);

        root.removeChildComponent(parentComponent);
    }

    void testSVGContentStaysPutWhenResized()
    {
        beginTest("svg / content keeps its position within the view-box when resized");

        juce::Component root;
        root.setSize(100, 100);
        const jive::LookAndFeel lookAndFeel{ root };

        jive::Interpreter interpreter;
        auto state = jive::parseXML(R"(
            <Component width="100" height="100">
                <svg viewBox="0 0 100 100" width="100%" height="100%">
                    <rect x="50" y="50" width="50" height="50" fill="blue" />
                </svg>
            </Component>
        )");
        auto parent = interpreter.interpret(state);
        auto* parentComponent = parent->getComponent().get();
        root.addAndMakeVisible(*parentComponent);
        parentComponent->setBounds(0, 0, 100, 100);

        auto* svgComponent = parent->getChildren()[0]->getComponent().get();
        const auto rectIsInTheBottomRightQuadrant = [svgComponent] {
            const auto snapshot = svgComponent->createComponentSnapshot(svgComponent->getLocalBounds());
            return snapshot.getPixelAt(75, 75) == juce::Colours::blue
                && snapshot.getPixelAt(25, 25) != juce::Colours::blue;
        };
        expect(rectIsInTheBottomRightQuadrant());

        state.setProperty("width", 40, nullptr);
        state.setProperty("height", 40, nullptr);
        state.setProperty("width", 100, nullptr);
        state.setProperty("height", 100, nullptr);

        expect(rectIsInTheBottomRightQuadrant());

        root.removeChildComponent(parentComponent);
    }

    void testSVGFillInherited()
    {
        beginTest("svg / a fill specified for an ancestor is inherited");

        juce::Component root;
        root.setSize(100, 100);
        jive::LookAndFeel lookAndFeel{ root };
        lookAndFeel.addStyles("#tinted",
                              jive::Styles{}
                                  .withFill(juce::Colours::red));

        jive::Interpreter interpreter;
        auto parent = interpreter.interpret(jive::parseXML(R"(
            <Component id="tinted" width="100" height="100">
                <svg width="20" height="20">
                    <rect width="20" height="20" />
                </svg>
            </Component>
        )"));
        auto* parentComponent = parent->getComponent().get();
        root.addAndMakeVisible(*parentComponent);
        parentComponent->setBounds(0, 0, 100, 100);

        auto* svgComponent = parent->getChildren()[0]->getComponent().get();
        const auto snapshot = svgComponent->createComponentSnapshot(svgComponent->getLocalBounds());
        expectEquals(snapshot.getPixelAt(10, 10), juce::Colours::red);

        root.removeChildComponent(parentComponent);
    }

    void testImage()
    {
        beginTest("image");

        {
            juce::ValueTree tree{
                "Image",
                {
                    { "width", 222 },
                    { "height", 333 },
                }
            };
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(tree);
            const auto& imageItem = *dynamic_cast<jive::GuiItemDecorator&>(*item)
                                         .toType<jive::Image>();
            expect(imageItem.getDrawable().isEmpty());

            const juce::Image image{ juce::Image::ARGB, 120, 40, true };
            tree.setProperty("source", juce::VariantConverter<juce::Image>::toVar(image), nullptr);
            expect(imageItem.getDrawable().isImage());
            expectEquals(static_cast<juce::Image>(imageItem.getDrawable()).getWidth(), image.getWidth());
            expectEquals(static_cast<juce::Image>(imageItem.getDrawable()).getHeight(), image.getHeight());
            expect(static_cast<juce::Image>(imageItem.getDrawable()).getPixelData() == image.getPixelData());
        }
        {
            const juce::Image image{ juce::Image::ARGB, 643, 111, true };
            const juce::ValueTree tree{
                "Image",
                {
                    { "width", 222 },
                    { "height", 333 },
                    { "source", juce::VariantConverter<juce::Image>::toVar(image) },
                },
            };
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(tree);
            const auto& imageItem = *dynamic_cast<jive::GuiItemDecorator&>(*item)
                                         .toType<jive::Image>();
            expect(imageItem.getDrawable().isImage());
            expectEquals(static_cast<juce::Image>(imageItem.getDrawable()).getWidth(), image.getWidth());
            expectEquals(static_cast<juce::Image>(imageItem.getDrawable()).getHeight(), image.getHeight());
            expect(static_cast<juce::Image>(imageItem.getDrawable()).getPixelData() == image.getPixelData());
        }
    }

    void testChildComponent()
    {
        beginTest("child-getComponent()");

        {
            const juce::Image image{ juce::Image::ARGB, 78, 27, true };
            const juce::ValueTree parentState{
                "Component",
                {
                    { "width", 489 },
                    { "height", 307 },
                    { "align-items", "flex-start" },
                },
                {
                    juce::ValueTree{
                        "Image",
                        {
                            {
                                "source",
                                juce::VariantConverter<juce::Image>::toVar(image),
                            },
                        },
                    },
                },
            };
            jive::Interpreter interpreter;
            auto parent = interpreter.interpret(parentState);
            auto& item = *parent->getChildren()[0];
            expectEquals(item.getComponent()->getNumChildComponents(), 1);
            expectEquals(item.getComponent()->getLocalBounds(), image.getBounds());

            const auto& childComponent = *item.getComponent()->getChildComponent(0);
            expectEquals(childComponent.getBounds(), item.getComponent()->getLocalBounds());

            parentState.getChild(0).setProperty("width", 134, nullptr);
            parentState.getChild(0).setProperty("height", 590, nullptr);
            expectEquals(childComponent.getBounds(), item.getComponent()->getLocalBounds());
        }
    }

    void testSVG()
    {
        beginTest("svg");

        {
            static constexpr auto svg = R"(
                <svg height="210" width="500">
                    <polygon points="100,10 40,198 190,78 10,78 160,198"
                             style="fill:lime;stroke:purple;stroke-width:5;fill-rule:evenodd;"/>
                </svg>
            )";
            juce::ValueTree tree{
                "Image",
                {
                    { "width", 222 },
                    { "height", 333 },
                }
            };
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(tree);
            auto& imageItem = *dynamic_cast<jive::GuiItemDecorator&>(*item)
                                   .toType<jive::Image>();
            expect(imageItem.getDrawable().isEmpty());

            tree.setProperty("source", svg, nullptr);
            expect(imageItem.getDrawable().isSVG());
            expect(dynamic_cast<DrawableComponent*>(imageItem.getComponent()->getChildComponent(0)) != nullptr);
        }
        {
            const juce::Image image{ juce::Image::ARGB, 643, 111, true };
            const juce::ValueTree tree{
                "Image",
                {
                    { "width", 222 },
                    { "height", 333 },
                    {
                        "source",
                        R"(
                            <svg height="400" width="450">
                                <path id="lineAB"
                                      d="M 100 350 l 150 -300"
                                      stroke="red"
                                      stroke-width="3"
                                      fill="none" />
                                <path id="lineBC"
                                      d="M 250 50 l 150 300"
                                      stroke="red"
                                      stroke-width="3"
                                      fill="none" />
                                <path d="M 100 350 q 150 -300 300 0"
                                      stroke="blue"
                                      stroke-width="5"
                                      fill="none" />
                            </svg>
                        )",
                    },
                },
            };
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(tree);
            auto& imageItem = *dynamic_cast<jive::GuiItemDecorator&>(*item)
                                   .toType<jive::Image>();
            expect(imageItem.getDrawable().isSVG());
            expect(dynamic_cast<DrawableComponent*>(imageItem.getComponent()->getChildComponent(0)) != nullptr);
        }
    }

    void testAutoSize()
    {
        beginTest("auto-size");

        {
            const juce::ValueTree tree{
                "Component",
                {
                    { "width", 222 },
                    { "height", 333 },
                    { "align-items", "flex-start" },
                },
                {
                    juce::ValueTree{
                        "Image",
                        {
                            {
                                "source",
                                juce::VariantConverter<juce::Image>::toVar(juce::Image{
                                    juce::Image::ARGB,
                                    80,
                                    40,
                                    true,
                                }),
                            },
                        },
                    },
                }
            };
            jive::Interpreter interpreter;
            auto parent = interpreter.interpret(tree);
            auto& item = *parent->getChildren()[0];
            const auto& boxModel = jive::boxModel(item);
            expectEquals(boxModel.getWidth(), 80.0f);
            expectEquals(boxModel.getHeight(), 40.0f);
        }
        {
            const juce::ValueTree tree{
                "Component",
                {
                    { "width", 222 },
                    { "height", 333 },
                    { "align-items", "flex-start" },
                },
                {
                    juce::ValueTree{
                        "Image",
                        {
                            {
                                "source",
                                R"(
                                    <svg width="400" height="180">
                                        <rect x="50"
                                            y="20"
                                            rx="20"
                                            ry="20"
                                            width="150"
                                            height="150"
                                            style="fill:red;stroke:black;stroke-width:5;opacity:0.5" />
                                    </svg>
                                )",
                            },
                        },
                    },
                }
            };
            jive::Interpreter interpreter;
            auto parent = interpreter.interpret(tree);
            auto& item = *parent->getChildren()[0];
            const auto& boxModel = jive::boxModel(item);
            expectEquals(boxModel.getWidth(), 155.0f);
            expectEquals(boxModel.getHeight(), 155.0f);
        }
        {
            const juce::ValueTree tree{
                "Component",
                {
                    { "width", 222 },
                    { "height", 333 },
                    { "align-items", "flex-start" },
                },
                {
                    juce::ValueTree{
                        "Image",
                        {
                            {
                                "source",
                                R"(
                                    <svg viewBox="0 0 24 24">
                                        <rect x="5"
                                              y="7"
                                              width="10"
                                              height="8" />
                                    </svg>
                                )",
                            },
                        },
                    },
                }
            };
            jive::Interpreter interpreter;
            auto parent = interpreter.interpret(tree);
            auto& item = *parent->getChildren()[0];
            const auto& boxModel = jive::boxModel(item);
            expectEquals(boxModel.getWidth(), 24.0f);
            expectEquals(boxModel.getHeight(), 24.0f);
        }
    }

    void testInlineSVG()
    {
        beginTest("inline SVG");

        const juce::ValueTree state{
            "Component",
            {
                { "width", 300 },
                { "height", 200 },
            },
            {
                juce::ValueTree::fromXml(R"(
                    <svg></svg>
                )"),
            },
        };
        jive::Interpreter interpreter;
        auto parent = interpreter.interpret(state);
        const auto& image = *dynamic_cast<jive::GuiItemDecorator*>(parent->getChildren()[0])
                                 ->toType<jive::Image>();
        expect(image.getDrawable().isSVG());
    }
};

static ImageTest imageTest;
#endif
