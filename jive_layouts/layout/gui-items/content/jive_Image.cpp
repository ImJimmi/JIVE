#include <jive_layouts/jive_layouts.h>

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    Image::Image(std::unique_ptr<GuiItem> itemToDecorate)
        : GuiItemDecorator{ std::move(itemToDecorate) }
        , source{ state, "source" }
        , placement{ state, "placement", juce::RectanglePlacement::centred }
        , width{ state, "width" }
        , height{ state, "height" }
        , idealWidth{ state, "ideal-width" }
        , idealHeight{ state, "ideal-height" }
    {
        source.onValueChange = [this]() {
            setChildComponent(createChildComponent());
        };
        placement.onValueChange = [this]() {
            if (auto* image = dynamic_cast<juce::ImageComponent*>(childComponent.get()))
                image->setImagePlacement(placement);
        };

        setChildComponent(createChildComponent());
        component->setInterceptsMouseClicks(false, false);

        boxModel.addListener(*this);
    }

    //==================================================================================================================
    bool Image::isContainer() const
    {
        return false;
    }

    bool Image::isContent() const
    {
        return true;
    }

    Drawable Image::getDrawable() const
    {
        const auto isInlineSVG = state.getType().toString().compareIgnoreCase("svg") == 0;

        if (isInlineSVG)
            return Drawable{ state.toXmlString() };

        return source.get();
    }

    //==================================================================================================================
    void Image::componentMovedOrResized(juce::Component& componentThatWasMovedOrResized, bool wasMoved, bool wasResized)
    {
        GuiItemDecorator::componentMovedOrResized(componentThatWasMovedOrResized, wasMoved, wasResized);

        if (&componentThatWasMovedOrResized != component.get())
            return;

        if (childComponent != nullptr)
        {
            childComponent->setBounds(component->getLocalBounds());

            if (auto* drawable = dynamic_cast<juce::Drawable*>(childComponent.get()))
            {
                drawable->setTransformToFit(component->getLocalBounds().toFloat(),
                                            placement.get());
            }
        }
    }

    void Image::boxModelChanged(BoxModel& boxModelThatChanged)
    {
        GuiItemDecorator::boxModelChanged(boxModelThatChanged);

        if (boxModelThatChanged.state != state)
            return;

        if (childComponent != nullptr)
            childComponent->setBounds(component->getLocalBounds());

        idealWidth = juce::String{ calculateRequiredWidth() };
        idealHeight = juce::String{ calculateRequiredHeight() };
    }

    void Image::valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property)
    {
        GuiItemDecorator::valueTreePropertyChanged(tree, property);

        if (state.getType().toString().compareIgnoreCase("svg") == 0)
        {
            if (childComponent != nullptr)
            {
                setChildComponent(createChildComponent());

                childComponent->setBounds(component->getLocalBounds());

                if (auto* drawable = dynamic_cast<juce::Drawable*>(childComponent.get()))
                {
                    drawable->setTransformToFit(component->getLocalBounds().toFloat(),
                                                placement.get());
                }
            }
        }
    }

    //==================================================================================================================
    float Image::calculateAspectRatio(const juce::ImageComponent& image) const
    {
        return image.getImage().getBounds().toFloat().getAspectRatio();
    }

    float Image::calculateRequiredWidth(const juce::ImageComponent& image) const
    {
        if (boxModel.hasAutoHeight())
            return static_cast<float>(image.getImage().getWidth());

        return boxModel.getHeight() * calculateAspectRatio(image);
    }

    float Image::calculateRequiredWidth(const juce::Drawable& drawable) const
    {
        const auto drawableBounds = drawable.getDrawableBounds();

        if (boxModel.hasAutoHeight())
            return drawableBounds.getWidth();

        const auto scale = boxModel.getHeight() / drawableBounds.getHeight();
        return drawableBounds.getWidth() * scale;
    }

    float Image::calculateRequiredWidth() const
    {
        if (auto* drawable = dynamic_cast<juce::Drawable*>(childComponent.get()))
            return calculateRequiredWidth(*drawable);

        if (auto* image = dynamic_cast<juce::ImageComponent*>(childComponent.get()))
            return calculateRequiredWidth(*image);

        return 0.0f;
    }

    float Image::calculateRequiredHeight(const juce::ImageComponent& image) const
    {
        if (boxModel.hasAutoWidth())
            return static_cast<float>(image.getImage().getHeight());

        return boxModel.getWidth() / calculateAspectRatio(image);
    }

    float Image::calculateRequiredHeight(const juce::Drawable& drawable) const
    {
        const auto drawableBounds = drawable.getDrawableBounds();

        if (boxModel.hasAutoWidth())
            return drawableBounds.getHeight();

        const auto scale = boxModel.getWidth() / drawableBounds.getWidth();
        return drawableBounds.getHeight() * scale;
    }

    float Image::calculateRequiredHeight() const
    {
        if (auto* drawable = dynamic_cast<juce::Drawable*>(childComponent.get()))
            return calculateRequiredHeight(*drawable);

        if (auto* image = dynamic_cast<juce::ImageComponent*>(childComponent.get()))
            return calculateRequiredHeight(*image);

        return 0.0f;
    }

    std::unique_ptr<juce::ImageComponent> Image::createImageComponent(const juce::Image& image) const
    {
        auto imageComponent = std::make_unique<juce::ImageComponent>();

        imageComponent->setImage(image);
        imageComponent->setImagePlacement(placement);

        return imageComponent;
    }

    std::unique_ptr<juce::Drawable> Image::createSVG(const juce::String& svgString) const
    {
        const auto xml = juce::parseXML(svgString);
        return juce::Drawable::createFromSVG(*xml);
    }

    std::unique_ptr<juce::Component> Image::createChildComponent() const
    {
        const auto drawable = getDrawable();

        if (drawable.isImage())
            return createImageComponent(drawable);

        if (drawable.isSVG())
            return createSVG(drawable);

        return nullptr;
    }

    void Image::setChildComponent(std::unique_ptr<juce::Component> newComponent)
    {
        if (newComponent == nullptr)
        {
            childComponent = nullptr;
            return;
        }

        childComponent.reset();
        childComponent = std::move(newComponent);

        component->addAndMakeVisible(*childComponent);
        childComponent->setBounds(component->getLocalBounds());

        idealWidth = juce::String{ calculateRequiredWidth() };
        idealHeight = juce::String{ calculateRequiredHeight() };
    }
} // namespace jive

//======================================================================================================================
#if JIVE_UNIT_TESTS
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
        testPlacement();
        testAutoSize();
        testChildComponent();
        testSVG();
        testInlineSVG();
    }

private:
    std::unique_ptr<jive::Image> createImage(juce::ValueTree tree)
    {
        jive::Interpreter interpreter;
        return std::unique_ptr<jive::Image>{
            dynamic_cast<jive::Image*>(interpreter.interpret(tree).release()),
        };
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
            auto item = createImage(tree);
            expectEquals(item->getDrawable(), jive::Drawable{});

            const juce::Image image{ juce::Image::ARGB, 120, 40, true };
            tree.setProperty("source", juce::VariantConverter<juce::Image>::toVar(image), nullptr);
            expect(item->getDrawable().isImage());
            expectEquals(static_cast<juce::Image>(item->getDrawable()).getWidth(), image.getWidth());
            expectEquals(static_cast<juce::Image>(item->getDrawable()).getHeight(), image.getHeight());
            expect(static_cast<juce::Image>(item->getDrawable()).getPixelData() == image.getPixelData());
            expect(dynamic_cast<juce::ImageComponent*>(item->getComponent()->getChildComponent(0)) != nullptr);
        }
        {
            const juce::Image image{ juce::Image::ARGB, 643, 111, true };
            juce::ValueTree tree{
                "Image",
                {
                    { "width", 222 },
                    { "height", 333 },
                    { "source", juce::VariantConverter<juce::Image>::toVar(image) },
                },
            };
            auto item = createImage(tree);
            expect(item->getDrawable().isImage());
            expectEquals(static_cast<juce::Image>(item->getDrawable()).getWidth(), image.getWidth());
            expectEquals(static_cast<juce::Image>(item->getDrawable()).getHeight(), image.getHeight());
            expect(static_cast<juce::Image>(item->getDrawable()).getPixelData() == image.getPixelData());
        }
    }

    void testPlacement()
    {
        beginTest("placement");

        {
            juce::ValueTree tree{
                "Image",
                {
                    { "width", 222 },
                    { "height", 333 },
                    { "source", juce::VariantConverter<juce::Image>::toVar(juce::Image{ juce::Image::ARGB, 51, 54, true }) },
                },
            };
            auto item = createImage(tree);
            auto& image = dynamic_cast<juce::ImageComponent&>(*item->getComponent()->getChildComponent(0));
            expect(image.getImagePlacement().testFlags(juce::RectanglePlacement::centred));

            tree.setProperty("placement", "top right reduce-only", nullptr);
            expect(image.getImagePlacement().testFlags(juce::RectanglePlacement::yTop
                                                       + juce::RectanglePlacement::xRight
                                                       + juce::RectanglePlacement::onlyReduceInSize));
        }
        {
            juce::ValueTree tree{
                "Image",
                {
                    { "width", 222 },
                    { "height", 333 },
                    { "source", juce::VariantConverter<juce::Image>::toVar(juce::Image{ juce::Image::ARGB, 51, 54, true }) },
                    { "placement", "stretch fill increase-only" },
                },
            };
            auto item = createImage(tree);
            auto& image = dynamic_cast<juce::ImageComponent&>(*item->getComponent()->getChildComponent(0));
            expect(image.getImagePlacement().testFlags(juce::RectanglePlacement::stretchToFit
                                                       + juce::RectanglePlacement::fillDestination
                                                       + juce::RectanglePlacement::onlyIncreaseInSize));
        }
    }

    void testChildComponent()
    {
        beginTest("child-component");

        {
            juce::Image image{ juce::Image::ARGB, 78, 27, true };
            juce::ValueTree parentState{
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
            auto& item = parent->getChild(0);
            expectEquals(item.getComponent()->getNumChildComponents(), 1);
            expectEquals(item.getComponent()->getLocalBounds(), image.getBounds());

            auto& childComponent = *item.getComponent()->getChildComponent(0);
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
            auto item = createImage(tree);
            expectEquals(item->getDrawable(), jive::Drawable{});

            tree.setProperty("source", svg, nullptr);
            expect(item->getDrawable().isSVG());
            expect(dynamic_cast<juce::Drawable*>(item->getComponent()->getChildComponent(0)) != nullptr);
        }
        {
            const juce::Image image{ juce::Image::ARGB, 643, 111, true };
            juce::ValueTree tree{
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
            auto item = createImage(tree);
            expect(item->getDrawable().isSVG());
            expect(dynamic_cast<juce::Drawable*>(item->getComponent()->getChildComponent(0)) != nullptr);
        }
    }

    void testAutoSize()
    {
        beginTest("auto-size");

        {
            juce::ValueTree tree{
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
            auto& item = parent->getChild(0);
            expectEquals(item.boxModel.getWidth(), 80.0f);
            expectEquals(item.boxModel.getHeight(), 40.0f);
        }
        {
            juce::ValueTree tree{
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
            auto& item = parent->getChild(0);
            expectEquals(item.boxModel.getWidth(), 155.0f);
            expectEquals(item.boxModel.getHeight(), 155.0f);
        }
    }

    void testInlineSVG()
    {
        beginTest("inline SVG");

        juce::ValueTree state{
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
        auto& image = *dynamic_cast<jive::GuiItemDecorator*>(&parent->getChild(0))
                           ->toType<jive::Image>();
        expect(image.getDrawable().isSVG());
    }
};

static ImageTest imageTest;
#endif
