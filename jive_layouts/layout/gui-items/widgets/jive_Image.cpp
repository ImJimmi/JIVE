#include <jive_layouts/jive_layouts.h>

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    Image::Image(std::unique_ptr<GuiItem> itemToDecorate)
        : GuiItemDecorator{ std::move(itemToDecorate) }
        , image{ tree, "image" }
        , placement{ tree, "placement", juce::RectanglePlacement::centred }
    {
        image.onValueChange = [this]() {
            getImage().setImage(image);
        };
        getImage().setImage(image);

        placement.onValueChange = [this]() {
            getImage().setImagePlacement(placement);
        };
        getImage().setImagePlacement(placement);
    }

    //==================================================================================================================
    float Image::getWidth() const
    {
        if (hasAutoWidth() && getImage().getImage().isValid())
            return getImage().getImage().getWidth();

        return GuiItemDecorator::getWidth();
    }

    float Image::getHeight() const
    {
        if (hasAutoHeight() && getImage().getImage().isValid())
            return getImage().getImage().getHeight();

        return GuiItemDecorator::getHeight();
    }

    bool Image::isContainer() const
    {
        return false;
    }

    const juce::ImageComponent& Image::getImage() const
    {
        return *dynamic_cast<const juce::ImageComponent*>(&getComponent());
    }

    juce::ImageComponent& Image::getImage()
    {
        return *dynamic_cast<juce::ImageComponent*>(&getComponent());
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
            expectEquals(item->getImage().getImage(), juce::Image{});

            const juce::Image image{ juce::Image::ARGB, 120, 40, true };
            tree.setProperty("image", juce::VariantConverter<juce::Image>::toVar(image), nullptr);
            expectEquals(item->getImage().getImage(), image);
            expect(item->getImage().getImage().getPixelData() == image.getPixelData());
        }
        {
            const juce::Image image{ juce::Image::ARGB, 643, 111, true };
            juce::ValueTree tree{
                "Image",
                {
                    { "image", juce::VariantConverter<juce::Image>::toVar(image) },
                },
            };
            auto item = createImage(tree);
            expectEquals(item->getImage().getImage(), image);
            expect(item->getImage().getImage().getPixelData() == image.getPixelData());
        }
    }

    void testPlacement()
    {
        beginTest("placement");

        {
            juce::ValueTree tree{ "Image" };
            auto item = createImage(tree);
            expect(item->getImage().getImagePlacement().testFlags(juce::RectanglePlacement::centred));

            tree.setProperty("placement", "top right reduce-only", nullptr);
            expect(item->getImage().getImagePlacement().testFlags(juce::RectanglePlacement::yTop
                                                                  + juce::RectanglePlacement::xRight
                                                                  + juce::RectanglePlacement::onlyReduceInSize));
        }
        {
            juce::ValueTree tree{
                "Image",
                {
                    { "placement", "stretch fill increase-only" },
                },
            };
            auto item = createImage(tree);
            expect(item->getImage().getImagePlacement().testFlags(juce::RectanglePlacement::stretchToFit
                                                                  + juce::RectanglePlacement::fillDestination
                                                                  + juce::RectanglePlacement::onlyIncreaseInSize));
        }
    }

    void testAutoSize()
    {
        beginTest("auto-size");

        const juce::Image image{ juce::Image::ARGB, 78, 27, true };
        juce::ValueTree tree{
            "Image",
            {
                { "image", juce::VariantConverter<juce::Image>::toVar(image) },
            },
        };
        auto item = createImage(tree);
        expectEquals(item->getWidth(), 78.0f);
        expectEquals(item->getHeight(), 27.0f);
    }
};

static ImageTest imageTest;
#endif
