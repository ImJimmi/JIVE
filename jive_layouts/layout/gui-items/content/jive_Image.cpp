#include "jive_Image.h"

#include <jive_layouts/layout/gui-items/jive_CommonGuiItem.h>

namespace jive
{
    Image::Image(std::unique_ptr<GuiItem> itemToDecorate)
        : GuiItemDecorator{ std::move(itemToDecorate) }
        , source{ state, "source" }
        , placement{ state, "placement" }
        , width{ state, "width" }
        , height{ state, "height" }
        , idealWidth{ state, "jive::ideal-width" }
        , idealHeight{ state, "jive::ideal-height" }
        , boxModel{ toType<CommonGuiItem>()->boxModel }
    {
        const BoxModel::ScopedCallbackLock boxModelLock{ jive::boxModel(*this) };

        source.onValueChange = [this]() {
            updateImageComponentDrawable();
        };
        placement.onValueChange = [this]() {
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

    Drawable Image::getDrawable() const
    {
        const auto isInlineSVG = state.getType().toString().compareIgnoreCase("svg") == 0;

        if (isInlineSVG)
        {
            auto stringSafe = state.createCopy();

            for (auto i = 0; i < stringSafe.getNumProperties(); i++)
            {
                const auto name = stringSafe.getPropertyName(i);

                if (stringSafe[name].isArray()
                    || stringSafe[name].isMethod()
                    || stringSafe[name].isObject())
                {
                    stringSafe.removeProperty(name, nullptr);
                    i--;
                }
            }

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
                drawable->setTransformToFit(getComponent()->getLocalBounds().toFloat(),
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

    void Image::valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier&)
    {
        if (state.getType().toString().compareIgnoreCase("svg") == 0)
        {
            if (!changingChild && childComponent != nullptr)
            {
                updateImageComponentDrawable();

                childComponent->setBounds(getComponent()->getLocalBounds());

                if (auto* drawable = dynamic_cast<juce::Drawable*>(childComponent.get()))
                {
                    drawable->setTransformToFit(getComponent()->getLocalBounds().toFloat(),
                                                placement.getOr(juce::RectanglePlacement::centred));
                }
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

    void Image::updateImageComponentDrawable()
    {
        const auto drawable = getDrawable();
        std::unique_ptr<juce::Drawable> result;

        if (drawable.isImage())
            result = std::make_unique<juce::DrawableImage>(static_cast<juce::Image>(drawable));

        if (drawable.isSVG())
        {
            const auto xml = juce::parseXML(static_cast<juce::String>(drawable));
            result = juce::Drawable::createFromSVG(*xml);
        }

        getImageComponent().setDrawable(std::move(result));

        idealWidth = juce::String{ calculateRequiredWidth() } + "px";
        idealHeight = juce::String{ calculateRequiredHeight() } + "px";
    }
} // namespace jive

#if JIVE_UNIT_TESTS
    #include <jive_layouts/layout/jive_Interpreter.h>

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

    void testChildComponent()
    {
        beginTest("child-getComponent()");

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
