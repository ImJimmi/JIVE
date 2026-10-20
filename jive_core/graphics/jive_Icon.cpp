#include "jive_Icon.h"

#include "jive_CurrentColour.h"

#include <jive_core/compile-time-helpers/jive_JuceVersion.h>
#include <jive_core/files/jive_SourceFile.h>

namespace jive
{
    [[nodiscard]] static juce::String loadSVG(const juce::String& source)
    {
        if (source.trimStart().startsWithChar('<'))
            return source;

        if (const SourceFile file{ source }; file.getPath().endsWithIgnoreCase(".svg"))
            return file.loadAsString();

        return {};
    }

    [[nodiscard]] static std::unique_ptr<juce::Drawable> createDrawable(const juce::String& source,
                                                                        const juce::String& svg)
    {
        if (svg.isNotEmpty())
        {
            const auto stringWithReplacedColour = replaceCurrentColourWithStandIn(svg);

#if JIVE_JUCE_9
            return juce::Drawable::createFromSVGString(stringWithReplacedColour);
#else
            if (const auto xml = juce::parseXML(stringWithReplacedColour))
                return juce::Drawable::createFromSVG(*xml);

            return nullptr;
#endif
        }

        if (const SourceFile file{ source }; file.exists())
        {
            const auto data = file.loadAsData();
            return juce::Drawable::createFromImageData(data.getData(), data.getSize());
        }

        return nullptr;
    }

    // An SVG's drawable only covers the shapes inside it, but an icon should
    // keep any space around them that the SVG's own bounds give it.
    [[nodiscard]] static juce::Rectangle<float> getIntrinsicBounds(const juce::Drawable& drawable)
    {
        if (const auto* composite = dynamic_cast<const juce::DrawableComposite*>(&drawable);
            composite != nullptr && !composite->getContentArea().isEmpty())
        {
            return composite->getContentArea();
        }

        return drawable.getDrawableBounds();
    }

    Icon::Icon(const juce::String& iconSource)
        : source{ iconSource }
    {
        const auto svg = loadSVG(source);
        drawable = createDrawable(source, svg);
        usesCurrentColour = svg.containsIgnoreCase("currentColor");

        if (drawable != nullptr)
            intrinsicBounds = getIntrinsicBounds(*drawable);
    }

    const juce::String& Icon::getSource() const
    {
        return source;
    }

    bool Icon::isEmpty() const
    {
        return drawable == nullptr || intrinsicBounds.isEmpty();
    }

    float Icon::getAspectRatio() const
    {
        if (isEmpty())
            return 1.0f;

        return intrinsicBounds.getWidth() / intrinsicBounds.getHeight();
    }

    static void tint(juce::Drawable& drawable,
                     const std::optional<juce::FillType>& fill,
                     const std::optional<juce::FillType>& stroke)
    {
        if (auto* shape = dynamic_cast<juce::DrawableShape*>(&drawable))
        {
            if (fill.has_value() && !shape->getFill().isInvisible())
                shape->setFill(*fill);

            if (stroke.has_value()
                && !shape->getStrokeFill().isInvisible()
                && shape->getStrokeType().getStrokeThickness() > 0.0f)
            {
                shape->setStrokeFill(*stroke);
            }

            return;
        }

        if (auto* composite = dynamic_cast<juce::DrawableComposite*>(&drawable))
        {
#if JIVE_JUCE_9
            for (auto i = 0; i < composite->getNumChildren(); ++i)
            {
                auto* child = &composite->getChild(i);
#else
            for (auto* child : composite->getChildren())
            {
#endif
                if (auto* drawableChild = dynamic_cast<juce::Drawable*>(child))
                    tint(*drawableChild, fill, stroke);
            }
        }
    }

    void Icon::draw(juce::Graphics& g,
                    juce::Rectangle<float> area,
                    juce::Colour currentColour,
                    const std::optional<Fill>& fill,
                    const std::optional<Fill>& stroke) const
    {
        if (isEmpty())
            return;

        const auto transform = juce::RectanglePlacement{ juce::RectanglePlacement::centred }
                                   .getTransformToFit(intrinsicBounds, area);

        if (!usesCurrentColour && !fill.has_value() && !stroke.has_value())
        {
            drawable->draw(g, 1.0f, transform);
            return;
        }

        const auto toFillType = [this](const std::optional<Fill>& tintFill) -> std::optional<juce::FillType> {
            if (tintFill.has_value())
                return tintFill->toJuceFillType(intrinsicBounds);

            return std::nullopt;
        };

        const auto recoloured = drawable->createCopy();

        if (usesCurrentColour)
            recoloured->replaceColour(getCurrentColourStandIn(), currentColour);

        tint(*recoloured, toFillType(fill), toFillType(stroke));
        recoloured->draw(g, 1.0f, transform);
    }

    bool Icon::operator==(const Icon& other) const
    {
        return source == other.source;
    }

    bool Icon::operator!=(const Icon& other) const
    {
        return !(*this == other);
    }
} // namespace jive

namespace juce
{
    jive::Icon VariantConverter<jive::Icon>::fromVar(const var& v)
    {
        return jive::Icon{ v.toString() };
    }

    var VariantConverter<jive::Icon>::toVar(const jive::Icon& icon)
    {
        return icon.getSource();
    }
} // namespace juce

#if JIVE_UNIT_TESTS
    #include <jive_core/testing/jive_UnitTest.h>

[[nodiscard]] static juce::Image drawIcon(const jive::Icon& icon,
                                          const std::optional<jive::Fill>& fill,
                                          const std::optional<jive::Fill>& stroke)
{
    juce::Image image{ juce::Image::ARGB, 20, 20, true };
    juce::Graphics g{ image };
    icon.draw(g, image.getBounds().toFloat(), juce::Colours::green, fill, stroke);
    return image;
}

static jive::UnitTest iconSourcesTest{
    "jive",
    "jive::Icon",
    "Icons load from SVG markup or a file, and keep their SVG's bounds",
    [](auto& test) {
        const jive::Icon fromMarkup{ R"(
            <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 40 10">
                <rect x="10" width="5" height="5" />
            </svg>
        )" };
        test.expect(!fromMarkup.isEmpty());
        test.expectEquals(fromMarkup.getAspectRatio(), 4.0f);

        const auto file = juce::File::createTempFile(".svg");
        file.replaceWithText(R"(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 10 20"><rect width="10" height="20" /></svg>)");
        const jive::Icon fromFile{ file.getFullPathName() };
        test.expect(!fromFile.isEmpty());
        test.expectEquals(fromFile.getAspectRatio(), 0.5f);
        file.deleteFile();

        test.expect(jive::Icon{}.isEmpty());
        test.expect(jive::Icon{ "missing.svg" }.isEmpty());
        test.expect(jive::Icon{ "<html />" }.isEmpty());
    },
};

static jive::UnitTest iconTintTest{
    "jive",
    "jive::Icon",
    "A fill or stroke only recolours what's already visible",
    [](auto& test) {
        const jive::Icon filled{ R"(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 20 20"><rect width="20" height="20" fill="black" /></svg>)" };
        test.expect(drawIcon(filled, std::nullopt, std::nullopt).getPixelAt(10, 10) == juce::Colours::black);
        test.expect(drawIcon(filled, juce::Colours::red, std::nullopt).getPixelAt(10, 10) == juce::Colours::red);

        const jive::Icon stroked{ R"(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 20 20"><rect x="2" y="2" width="16" height="16" fill="none" stroke="black" stroke-width="2" /></svg>)" };
        const auto strokedImage = drawIcon(stroked, juce::Colours::red, juce::Colours::blue);
        test.expect(strokedImage.getPixelAt(10, 10).isTransparent());
        test.expect(strokedImage.getPixelAt(2, 10) == juce::Colours::blue);
    },
};

static jive::UnitTest iconCurrentColourTest{
    "jive",
    "jive::Icon",
    "currentColor is drawn in the colour the icon is drawn with",
    [](auto& test) {
        const jive::Icon icon{ R"(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 20 20" fill="none" stroke="currentColor" stroke-width="2"><rect x="2" y="2" width="16" height="16" /></svg>)" };
        test.expect(drawIcon(icon, std::nullopt, std::nullopt).getPixelAt(2, 10) == juce::Colours::green);
        test.expect(drawIcon(icon, std::nullopt, juce::Colours::blue).getPixelAt(2, 10) == juce::Colours::blue);
        test.expect(drawIcon(icon, std::nullopt, std::nullopt).getPixelAt(10, 10).isTransparent());
    },
};
#endif
