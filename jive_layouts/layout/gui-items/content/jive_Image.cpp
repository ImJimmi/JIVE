#include <jive_layouts/jive_layouts.h>

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    Image::Image(std::unique_ptr<GuiItem> itemToDecorate)
        : GuiItemDecorator{ std::move(itemToDecorate) }
        , source{ tree, "source" }
        , placement{ tree, "placement", juce::RectanglePlacement::centred }
    {
        source.onValueChange = [this]() {
            updateChildComponent();
        };
        placement.onValueChange = [this]() {
            updatePlacement();
        };

        updateChildComponent();
        getComponent().setInterceptsMouseClicks(false, false);
    }

    //==================================================================================================================
    float Image::getWidth() const
    {
        if (hasAutoWidth())
        {
            if (auto* image = dynamic_cast<juce::ImageComponent*>(childComponent.get()))
            {
                return image->getImage().getWidth();
            }
            if (auto* drawable = dynamic_cast<juce::Drawable*>(childComponent.get()))
            {
                return drawable->getDrawableBounds().getWidth();
            }

            return 0.0f;
        }

        return GuiItemDecorator::getWidth();
    }

    float Image::getHeight() const
    {
        if (hasAutoHeight())
        {
            if (auto* image = dynamic_cast<juce::ImageComponent*>(childComponent.get()))
            {
                return image->getImage().getHeight();
            }
            if (auto* drawable = dynamic_cast<juce::Drawable*>(childComponent.get()))
            {
                return drawable->getDrawableBounds().getHeight();
            }

            return 0.0f;
        }

        return GuiItemDecorator::getHeight();
    }

    bool Image::isContainer() const
    {
        return false;
    }

    bool Image::isContent() const
    {
        return true;
    }

    Drawable Image::getDrawable()
    {
        return source.get();
    }

    //==================================================================================================================
    void Image::componentMovedOrResized(juce::Component& componentThatMovedOrResized, bool wasMoved, bool wasResized)
    {
        GuiItemDecorator::componentMovedOrResized(componentThatMovedOrResized, wasMoved, wasResized);

        if (&componentThatMovedOrResized != &getComponent())
            return;
        if (!wasResized)
            return;

        updateChildBounds();
    }

    //==================================================================================================================
    void Image::updateChildComponent()
    {
        const auto drawable = getDrawable();

        if (drawable.isImage())
        {
            auto image = std::make_unique<juce::ImageComponent>();
            image->setImage(static_cast<juce::Image>(drawable));
            childComponent = std::move(image);

            updatePlacement();
        }
        else if (drawable.isSVG())
        {
            auto xml = juce::parseXML(drawable);
            childComponent = juce::Drawable::createFromSVG(*xml);
        }

        if (childComponent != nullptr)
            getComponent().addAndMakeVisible(*childComponent);

        updateChildBounds();

        if (auto* parent = getParent())
            parent->informContentChanged();
    }

    void Image::updateChildBounds()
    {
        if (childComponent != nullptr)
            childComponent->setBounds(getComponent().getLocalBounds());
    }

    void Image::updatePlacement()
    {
        if (auto* image = dynamic_cast<juce::ImageComponent*>(childComponent.get()))
        {
            image->setImagePlacement(placement);
        }
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
        testContentChanged();
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
            juce::ValueTree tree{ "Image" };
            auto item = createImage(tree);
            expectEquals(item->getDrawable(), jive::Drawable{});

            const juce::Image image{ juce::Image::ARGB, 120, 40, true };
            tree.setProperty("source", juce::VariantConverter<juce::Image>::toVar(image), nullptr);
            expect(item->getDrawable().isImage());
            expectEquals(static_cast<juce::Image>(item->getDrawable()).getWidth(), image.getWidth());
            expectEquals(static_cast<juce::Image>(item->getDrawable()).getHeight(), image.getHeight());
            expect(static_cast<juce::Image>(item->getDrawable()).getPixelData() == image.getPixelData());
            expect(dynamic_cast<juce::ImageComponent*>(item->getComponent().getChildComponent(0)) != nullptr);
        }
        {
            const juce::Image image{ juce::Image::ARGB, 643, 111, true };
            juce::ValueTree tree{
                "Image",
                {
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
                    { "source", juce::VariantConverter<juce::Image>::toVar(juce::Image{ juce::Image::ARGB, 51, 54, true }) },
                },
            };
            auto item = createImage(tree);
            auto& image = dynamic_cast<juce::ImageComponent&>(*item->getComponent().getChildComponent(0));
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
                    { "source", juce::VariantConverter<juce::Image>::toVar(juce::Image{ juce::Image::ARGB, 51, 54, true }) },
                    { "placement", "stretch fill increase-only" },
                },
            };
            auto item = createImage(tree);
            auto& image = dynamic_cast<juce::ImageComponent&>(*item->getComponent().getChildComponent(0));
            expect(image.getImagePlacement().testFlags(juce::RectanglePlacement::stretchToFit
                                                       + juce::RectanglePlacement::fillDestination
                                                       + juce::RectanglePlacement::onlyIncreaseInSize));
        }
    }

    void testChildComponent()
    {
        beginTest("child-component");

        {
            juce::ValueTree tree{
                "Image",
                {
                    {
                        "source",
                        juce::VariantConverter<juce::Image>::toVar(juce::Image{ juce::Image::ARGB, 78, 27, true }),
                    },
                    { "width", 489 },
                    { "height", 307 },
                },
            };
            auto item = createImage(tree);
            expectEquals(item->getComponent().getNumChildComponents(), 1);

            auto& child = *item->getComponent().getChildComponent(0);
            expectEquals(child.getBounds(),
                         { 0, 0, item->getComponent().getWidth(), item->getComponent().getHeight() });

            tree.setProperty("width", 134, nullptr);
            tree.setProperty("height", 590, nullptr);
            expectEquals(child.getBounds(),
                         { 0, 0, item->getComponent().getWidth(), item->getComponent().getHeight() });
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
            juce::ValueTree tree{ "Image" };
            auto item = createImage(tree);
            expectEquals(item->getDrawable(), jive::Drawable{});

            tree.setProperty("source", svg, nullptr);
            expect(item->getDrawable().isSVG());
            expect(dynamic_cast<juce::Drawable*>(item->getComponent().getChildComponent(0)) != nullptr);
        }
        {
            const juce::Image image{ juce::Image::ARGB, 643, 111, true };
            juce::ValueTree tree{
                "Image",
                {
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
            expect(dynamic_cast<juce::Drawable*>(item->getComponent().getChildComponent(0)) != nullptr);
        }
    }

    void testAutoSize()
    {
        beginTest("auto-size");

        {
            juce::ValueTree tree{
                "Image",
                {
                    {
                        "source",
                        juce::VariantConverter<juce::Image>::toVar(juce::Image{ juce::Image::ARGB, 78, 27, true }),
                    },
                },
            };
            auto item = createImage(tree);
            expectEquals(item->getWidth(), 78.0f);
            expectEquals(item->getHeight(), 27.0f);
        }
        {
            juce::ValueTree tree{
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
            };
            auto item = createImage(tree);
            expectEquals(item->getWidth(), 155.0f);
            expectEquals(item->getHeight(), 155.0f);
        }
    }

    void testContentChanged()
    {
        beginTest("parent-content-changed");

        class SpyGuiItem : public jive::GuiItem
        {
        public:
            using jive::GuiItem::GuiItem;

            std::function<void()> onContentChanged = nullptr;

        protected:
            void contentChanged() final
            {
                if (onContentChanged != nullptr)
                    onContentChanged();
            }
        };

        juce::Image image{ juce::Image::ARGB, 10, 10, false };
        juce::ValueTree tree{
            "Spy",
            {},
            {
                juce::ValueTree{
                    "Image",
                    {
                        { "source", juce::VariantConverter<juce::Image>::toVar(image) },
                    },
                },
            },
        };
        SpyGuiItem item{
            std::make_unique<juce::Component>(),
            tree,
        };

        auto parentContentChangedCalled = false;
        item.onContentChanged = [&parentContentChangedCalled]() {
            parentContentChangedCalled = true;
        };

        item.addChild(std::make_unique<jive::Image>(std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(),
                                                                                    tree.getChild(0),
                                                                                    &item)));
        expect(parentContentChangedCalled);
        parentContentChangedCalled = false;

        tree.getChild(0).setProperty("source", "<svg/>", nullptr);
        expect(parentContentChangedCalled);
    }
};

static ImageTest imageTest;
#endif
