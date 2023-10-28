#include <jive_layouts/jive_layouts.h>

namespace jive
{
    Image::Image(std::unique_ptr<GuiItem> itemToDecorate)
        : GuiItemDecorator{ std::move(itemToDecorate) }
        , source{ state, "source" }
        , placement{ state, "placement" }
        , width{ state, "width" }
        , height{ state, "height" }
        , idealWidth{ state, "ideal-width" }
        , idealHeight{ state, "ideal-height" }
        , boxModel{ toType<CommonGuiItem>()->boxModel }
    {
        if (!placement.exists())
            placement = juce::RectanglePlacement::centred;

        source.onValueChange = [this]() {
            setChildComponent(createChildComponent());
        };
        placement.onValueChange = [this]() {
            if (auto* image = dynamic_cast<juce::ImageComponent*>(childComponent.get()))
                image->setImagePlacement(placement);
        };

        setChildComponent(createChildComponent());
        boxModel.addListener(*this);
        state.addListener(this);
    }

    Image::~Image()
    {
        state.removeListener(this);
        boxModel.removeListener(*this);

        if (component != nullptr)
            component->removeComponentListener(this);
    }

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

    void Image::componentMovedOrResized(juce::Component& componentThatWasMovedOrResized, bool, bool)
    {
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
        if (boxModelThatChanged.state != state)
            return;

        if (childComponent != nullptr)
            childComponent->setBounds(component->getLocalBounds());

        idealWidth = juce::String{ calculateRequiredWidth() };
        idealHeight = juce::String{ calculateRequiredHeight() };
    }

    void Image::valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier&)
    {
        if (state.getType().toString().compareIgnoreCase("svg") == 0)
        {
            if (!changingChild && childComponent != nullptr)
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
        const juce::ScopedValueSetter svs{ changingChild, true };

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

        component->addComponentListener(this);
        component->setInterceptsMouseClicks(false, false);
    }
} // namespace jive

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
            auto& imageItem = *dynamic_cast<jive::GuiItemDecorator&>(*item)
                                   .toType<jive::Image>();
            expect(imageItem.getDrawable().isEmpty());

            const juce::Image image{ juce::Image::ARGB, 120, 40, true };
            tree.setProperty("source", juce::VariantConverter<juce::Image>::toVar(image), nullptr);
            expect(imageItem.getDrawable().isImage());
            expectEquals(static_cast<juce::Image>(imageItem.getDrawable()).getWidth(), image.getWidth());
            expectEquals(static_cast<juce::Image>(imageItem.getDrawable()).getHeight(), image.getHeight());
            expect(static_cast<juce::Image>(imageItem.getDrawable()).getPixelData() == image.getPixelData());
            expect(dynamic_cast<juce::ImageComponent*>(imageItem.getComponent()->getChildComponent(0)) != nullptr);
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
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(tree);
            auto& imageItem = *dynamic_cast<jive::GuiItemDecorator&>(*item)
                                   .toType<jive::Image>();
            expect(imageItem.getDrawable().isImage());
            expectEquals(static_cast<juce::Image>(imageItem.getDrawable()).getWidth(), image.getWidth());
            expectEquals(static_cast<juce::Image>(imageItem.getDrawable()).getHeight(), image.getHeight());
            expect(static_cast<juce::Image>(imageItem.getDrawable()).getPixelData() == image.getPixelData());
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
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(tree);
            auto& imageItem = *dynamic_cast<jive::GuiItemDecorator&>(*item)
                                   .toType<jive::Image>();
            auto& image = dynamic_cast<juce::ImageComponent&>(*imageItem.getComponent()->getChildComponent(0));
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
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(tree);
            auto& imageItem = *dynamic_cast<jive::GuiItemDecorator&>(*item)
                                   .toType<jive::Image>();
            auto& image = dynamic_cast<juce::ImageComponent&>(*imageItem.getComponent()->getChildComponent(0));
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
            auto& item = *parent->getChildren()[0];
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
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(tree);
            auto& imageItem = *dynamic_cast<jive::GuiItemDecorator&>(*item)
                                   .toType<jive::Image>();
            expect(imageItem.getDrawable().isEmpty());

            tree.setProperty("source", svg, nullptr);
            expect(imageItem.getDrawable().isSVG());
            expect(dynamic_cast<juce::Drawable*>(imageItem.getComponent()->getChildComponent(0)) != nullptr);
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
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(tree);
            auto& imageItem = *dynamic_cast<jive::GuiItemDecorator&>(*item)
                                   .toType<jive::Image>();
            expect(imageItem.getDrawable().isSVG());
            expect(dynamic_cast<juce::Drawable*>(imageItem.getComponent()->getChildComponent(0)) != nullptr);
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
            auto& item = *parent->getChildren()[0];
            const auto& boxModel = jive::boxModel(item);
            expectEquals(boxModel.getWidth(), 80.0f);
            expectEquals(boxModel.getHeight(), 40.0f);
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
            auto& item = *parent->getChildren()[0];
            const auto& boxModel = jive::boxModel(item);
            expectEquals(boxModel.getWidth(), 155.0f);
            expectEquals(boxModel.getHeight(), 155.0f);
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
        auto& image = *dynamic_cast<jive::GuiItemDecorator*>(parent->getChildren()[0])
                           ->toType<jive::Image>();
        expect(image.getDrawable().isSVG());
    }
};

static ImageTest imageTest;
#endif
