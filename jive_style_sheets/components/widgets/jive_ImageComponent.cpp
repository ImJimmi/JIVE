#include "jive_ImageComponent.h"

#include <jive_style_sheets/graphics/jive_CurrentColour.h>

namespace jive
{
    ImageComponent::ImageComponent()
    {
        setInterceptsMouseClicks(false, false);
    }

    template <typename Composite, typename Callback>
    static void forEachChild(Composite& composite, Callback&& callback)
    {
#if JIVE_JUCE_9
        for (auto i = 0; i < composite.getNumChildren(); i++)
            callback(composite.getChild(i));
#else
        for (auto* child : composite.getChildren())
        {
            if (auto* drawable = dynamic_cast<juce::Drawable*>(child))
                callback(*drawable);
        }
#endif
    }

    [[nodiscard]] static juce::AffineTransform getTransform(const juce::Drawable& drawable)
    {
#if JIVE_JUCE_9
        return drawable.getDrawableTransform();
#else
        return drawable.getTransform();
#endif
    }

    // Since JUCE 9, drawables aren't components so can't be hidden. Scaling
    // one down to nothing stops it drawing anything instead.
    static void hide(juce::Drawable& drawable)
    {
#if JIVE_JUCE_9
        drawable.setDrawableTransform(juce::AffineTransform::scale(0.0f));
#else
        drawable.setVisible(false);
#endif
    }

    [[nodiscard]] static bool isHidden(const juce::Drawable& drawable)
    {
#if JIVE_JUCE_9
        return getTransform(drawable).isSingularity();
#else
        return !drawable.isVisible();
#endif
    }

    static void hideTransparentChildren(juce::DrawableComposite& composite)
    {
        forEachChild(composite, [](juce::Drawable& child) {
            if (auto* shape = dynamic_cast<juce::DrawableShape*>(&child))
            {
                if (shape->getFill().isInvisible()
                    && (shape->getStrokeFill().isInvisible()
                        || juce::approximatelyEqual(shape->getStrokeType().getStrokeThickness(), 0.0f)))
                {
                    hide(*shape);
                }
            }
            else if (auto* compositeChild = dynamic_cast<juce::DrawableComposite*>(&child))
            {
                hideTransparentChildren(*compositeChild);
            }
        });
    }

    [[nodiscard]] static bool isCurrentColourStandIn(const juce::FillType& fill)
    {
        return fill.isColour() && fill.colour == getCurrentColourStandIn();
    }

    static void findShapesUsingCurrentColour(juce::Drawable& drawable,
                                             std::vector<juce::DrawableShape*>& fills,
                                             std::vector<juce::DrawableShape*>& strokes)
    {
        if (auto* shape = dynamic_cast<juce::DrawableShape*>(&drawable))
        {
            if (isCurrentColourStandIn(shape->getFill()))
                fills.push_back(shape);

            if (isCurrentColourStandIn(shape->getStrokeFill()))
                strokes.push_back(shape);
        }
        else if (auto* composite = dynamic_cast<juce::DrawableComposite*>(&drawable))
        {
            forEachChild(*composite, [&fills, &strokes](juce::Drawable& child) {
                findShapesUsingCurrentColour(child, fills, strokes);
            });
        }
    }

    static constexpr juce::uint32 firstSVGElementMarker = 0x020000;

    [[nodiscard]] static juce::String getMarker(std::size_t elementIndex, bool isStroke)
    {
        const auto rgb = firstSVGElementMarker + static_cast<juce::uint32>(elementIndex * 2) + (isStroke ? 1u : 0u);
        return "#" + juce::Colour{ 0xff000000 | rgb }.toDisplayString(false);
    }

    // Opacity attributes such as fill-opacity change a marker's alpha, so
    // only its colour identifies it.
    [[nodiscard]] static std::optional<std::size_t> findMarkedElement(const juce::FillType& fill,
                                                                      std::size_t numElements)
    {
        if (!fill.isColour())
            return std::nullopt;

        const auto rgb = fill.colour.getARGB() & 0x00ffffff;

        if (rgb < firstSVGElementMarker || rgb >= firstSVGElementMarker + numElements * 2)
            return std::nullopt;

        return static_cast<std::size_t>(rgb - firstSVGElementMarker) / 2;
    }

    static void findMarkedShapes(juce::Drawable& drawable,
                                 std::size_t numElements,
                                 std::unordered_map<juce::DrawableShape*, ImageComponent::MarkedShape>& markedShapes)
    {
        if (auto* shape = dynamic_cast<juce::DrawableShape*>(&drawable))
        {
            const auto fillElement = findMarkedElement(shape->getFill(), numElements);
            const auto strokeElement = findMarkedElement(shape->getStrokeFill(), numElements);

            if (fillElement.has_value() || strokeElement.has_value())
            {
                markedShapes[shape] = {
                    fillElement,
                    strokeElement,
                    shape->getFill().colour.getFloatAlpha() * shape->getFill().getOpacity(),
                    shape->getStrokeFill().colour.getFloatAlpha() * shape->getStrokeFill().getOpacity(),
                };
            }
        }
        else if (auto* composite = dynamic_cast<juce::DrawableComposite*>(&drawable))
        {
            forEachChild(*composite, [numElements, &markedShapes](juce::Drawable& child) {
                findMarkedShapes(child, numElements, markedShapes);
            });
        }
    }

    void ImageComponent::setDrawable(std::unique_ptr<juce::Drawable>&& d,
                                     juce::Rectangle<float> intrinsicBoundsToUse,
                                     juce::Rectangle<float> viewBoxToUse)
    {
#if JIVE_JUCE_9
        // A drawable component stops listening to its drawable when it's
        // destroyed, so has to go before the drawable does.
        drawableComponent.reset();
#endif

        drawable = std::move(d);
        intrinsicBounds = intrinsicBoundsToUse;
        viewBox = viewBoxToUse;

        currentColourFills.clear();
        currentColourStrokes.clear();
        markedShapes.clear();

        if (drawable != nullptr)
        {
            findShapesUsingCurrentColour(*drawable, currentColourFills, currentColourStrokes);
            findMarkedShapes(*drawable, svgElements.size(), markedShapes);
        }

        if (drawable != nullptr)
        {
#if JIVE_JUCE_9
            drawableComponent = std::make_unique<juce::DrawableComponent>(*drawable);
            addAndMakeVisible(*drawableComponent);
#else
            addAndMakeVisible(*drawable);
#endif
        }

        if (auto* composite = dynamic_cast<juce::DrawableComposite*>(drawable.get()))
            hideTransparentChildren(*composite);

        resized();
    }

    [[nodiscard]] static juce::FillType toFillType(const Fill& fill,
                                                   juce::Rectangle<float> bounds,
                                                   float opacity)
    {
        auto fillType = fill.toJuceFillType(bounds);
        fillType.setOpacity(fillType.getOpacity() * opacity);
        return fillType;
    }

    // Shapes are drawn in their own coordinates (since JUCE 9, an SVG's
    // shapes each have their own transform), so gradients are laid out over
    // the element's bounds as seen from each one.
    static void setStyles(juce::Drawable& drawable,
                          juce::Rectangle<float> bounds,
                          const std::optional<Fill>& fill,
                          const std::optional<Fill>& stroke,
                          const std::unordered_map<juce::DrawableShape*, ImageComponent::MarkedShape>& markedShapes,
                          const std::vector<ImageComponent::ElementStyles>& elementStyles)
    {
        if (isHidden(drawable))
            return;

        const auto localBounds = bounds.transformedBy(getTransform(drawable).inverted());

        if (auto* shape = dynamic_cast<juce::DrawableShape*>(&drawable))
        {
            const auto marked = markedShapes.find(shape);
            const auto* markedShape = marked == std::end(markedShapes) ? nullptr : &marked->second;

            if (markedShape != nullptr && markedShape->fillElement.has_value())
                shape->setFill(toFillType(elementStyles[*markedShape->fillElement].fill, localBounds, markedShape->fillOpacity));
            else if (fill.has_value())
                shape->setFill(fill->toJuceFillType(localBounds));

            if (markedShape != nullptr && markedShape->strokeElement.has_value())
                shape->setStrokeFill(toFillType(elementStyles[*markedShape->strokeElement].stroke, localBounds, markedShape->strokeOpacity));
            else if (stroke.has_value())
                shape->setStrokeFill(stroke->toJuceFillType(localBounds));

            return;
        }

        if (auto* composite = dynamic_cast<juce::DrawableComposite*>(&drawable))
        {
            forEachChild(*composite, [&](juce::Drawable& child) {
                setStyles(child, localBounds, fill, stroke, markedShapes, elementStyles);
            });
        }
    }

    static juce::Path getVisibleOutline(const juce::Drawable& drawable)
    {
        if (isHidden(drawable))
            return {};

        if (const auto* shape = dynamic_cast<const juce::DrawableShape*>(&drawable))
        {
            if (shape->getFill().isInvisible() && shape->getStrokeFill().isInvisible())
                return {};

            // A stroked shape's outline is only its stroke, leaving out its fill.
            auto outline = shape->getOutlineAsPath();

            if (const auto* path = dynamic_cast<const juce::DrawablePath*>(shape);
                path != nullptr && !shape->getFill().isInvisible() && !shape->getStrokeFill().isInvisible())
            {
                outline.addPath(path->getPath(), getTransform(*shape));
            }

            return outline;
        }

        juce::Path outline;

        if (const auto* composite = dynamic_cast<const juce::DrawableComposite*>(&drawable))
        {
            forEachChild(*composite, [&outline](const juce::Drawable& child) {
                outline.addPath(getVisibleOutline(child));
            });
        }

        outline.applyTransform(getTransform(drawable));
        return outline;
    }

    void ImageComponent::paint(juce::Graphics&)
    {
        if (drawable == nullptr)
            return;

        auto* laf = dynamic_cast<LookAndFeel*>(&getLookAndFeel());

        if (laf == nullptr)
            return;

        const auto styles = laf->findMostApplicableStyles(*this);

        // As in CSS, currentColor is the foreground colour.
        const auto foreground = getForegroundFill(*this, styles, detail::getV4Colour(detail::V4Colour::defaultText));
        const auto currentColour = foreground.isGradient() ? foreground.gradient->getColour(0) : foreground.colour;

        for (auto* shape : currentColourFills)
            shape->setFill(currentColour);

        for (auto* shape : currentColourStrokes)
            shape->setStrokeFill(currentColour);

        const auto fill = styles.fill.hasValue() ? findFill(*this, styles) : std::nullopt;
        const auto stroke = styles.stroke.hasValue() ? findStroke(*this, styles) : std::nullopt;

        setStyles(*drawable,
                  getLocalBounds().toFloat(),
                  fill,
                  stroke,
                  markedShapes,
                  resolveElementStyles(*laf, fill, stroke, currentColour));

        if (styles.shadow.hasValue())
            getProperties().set(ids::shadowPath(), getVisibleOutline(*drawable).toString());

        laf->updateShadow<ImageComponent>(*this, styles);
    }

    void ImageComponent::resized()
    {
        if (drawable == nullptr)
            return;

        // An SVG's drawable positions itself around its content, keeping the
        // offset between the origin of its view-box and the first thing it
        // draws. Setting its bounds would throw that offset away, moving
        // everything it draws to the top-left, so its view-box is transformed
        // to fit instead.
        if (auto* composite = dynamic_cast<juce::DrawableComposite*>(drawable.get()))
        {
            if (!viewBox.isEmpty() && !getLocalBounds().isEmpty())
            {
                // Since JUCE 9, the content area is the SVG's width and height,
                // with its view-box fitted inside, rather than the view-box.
                const juce::RectanglePlacement centred{ juce::RectanglePlacement::centred };
                const auto contentArea = composite->getContentArea();
                const auto viewBoxInContentArea = centred.appliedTo(viewBox, contentArea);
                composite->setBoundingBox(contentArea.transformedBy(centred.getTransformToFit(viewBoxInContentArea,
                                                                                              getLocalBounds().toFloat())));
            }

            return;
        }

#if JIVE_JUCE_9
        drawableComponent
#else
        drawable
#endif
            ->setBounds(getLocalBounds());
    }

    [[nodiscard]] static bool isShape(const juce::Identifier& type)
    {
        static const juce::StringArray shapes{ "path", "rect", "circle", "ellipse", "line", "polyline", "polygon" };
        return shapes.contains(type.toString());
    }

    [[nodiscard]] static juce::String findMarkupPaint(juce::ValueTree element, const juce::Identifier& attribute)
    {
        for (; element.isValid(); element = element.getParent())
        {
            if (element.hasProperty(attribute))
                return element[attribute].toString().trim();
        }

        return {};
    }

    // Like CSS, an SVG's shapes are filled black and not stroked by default.
    [[nodiscard]] static Fill parseMarkupPaint(const juce::String& paint,
                                               const Fill& defaultPaint,
                                               const Fill& currentColour)
    {
        if (paint.isEmpty())
            return defaultPaint;

        if (paint.equalsIgnoreCase("none"))
            return juce::Colours::transparentBlack;

        if (paint.equalsIgnoreCase("currentColor"))
            return currentColour;

        return Fill::fromString(paint).value_or(defaultPaint);
    }

    // A paint server such as a url() can't be swapped for a colour and back.
    [[nodiscard]] static bool canBeMarked(const juce::String& paint)
    {
        return !paint.containsIgnoreCase("url(");
    }

    void ImageComponent::setSVGElements(juce::ValueTree svg)
    {
        svgElements.clear();

        const std::function<void(juce::ValueTree, juce::Component&)> addElements = [this, &addElements](juce::ValueTree parent,
                                                                                                        juce::Component& parentStandIn) {
            for (auto element : parent)
            {
                auto standIn = std::make_unique<juce::Component>();
                standIn->setComponentID(element["id"].toString());
                standIn->getProperties().set(ids::classes(), element["class"]);
                standIn->getProperties().set(ids::type(), element.getType().toString());
                standIn->setInterceptsMouseClicks(false, false);
                parentStandIn.addChildComponent(*standIn);

                auto& standInRef = *standIn;
                const auto index = svgElements.size();
                svgElements.push_back({ std::move(standIn), {}, {} });

                if (isShape(element.getType()))
                {
                    const auto fill = findMarkupPaint(element, "fill");
                    const auto stroke = findMarkupPaint(element, "stroke");

                    if (canBeMarked(fill))
                    {
                        svgElements[index].markupFill = fill;
                        element.setProperty("fill", getMarker(index, false), nullptr);
                    }

                    if (canBeMarked(stroke))
                    {
                        svgElements[index].markupStroke = stroke;
                        element.setProperty("stroke", getMarker(index, true), nullptr);
                    }
                }

                addElements(element, standInRef);
            }
        };

        if (svg.isValid())
            addElements(svg, *this);
    }

    // A style that matches an element takes precedence over its markup, as does
    // one it inherits, unless the markup makes it transparent. That keeps
    // things such as a background rectangle with fill="none" from being
    // filled by a style meant for the SVG's other shapes.
    std::vector<ImageComponent::ElementStyles> ImageComponent::resolveElementStyles(const LookAndFeel& laf,
                                                                                    const std::optional<Fill>& svgFill,
                                                                                    const std::optional<Fill>& svgStroke,
                                                                                    const Fill& currentColour) const
    {
        std::vector<ElementStyles> result;
        result.reserve(svgElements.size());

        std::unordered_map<const juce::Component*, std::pair<std::optional<Fill>, std::optional<Fill>>> resolved;
        resolved[this] = { svgFill, svgStroke };

        for (const auto& element : svgElements)
        {
            const auto& standIn = *element.standIn;
            const auto styles = laf.findMostApplicableStyles(standIn);
            const auto fill = styles.fill.hasValue() ? findFill(standIn, styles) : std::nullopt;
            const auto stroke = styles.stroke.hasValue() ? findStroke(standIn, styles) : std::nullopt;
            const auto& [parentFill, parentStroke] = resolved[standIn.getParentComponent()];
            resolved[&standIn] = { fill, stroke };

            const auto markupFill = parseMarkupPaint(element.markupFill, juce::Colours::black, currentColour);
            const auto markupStroke = parseMarkupPaint(element.markupStroke, juce::Colours::transparentBlack, currentColour);
            const auto isTransparent = markupFill.toJuceFillType().isInvisible()
                                    && markupStroke.toJuceFillType().isInvisible();

            const auto choose = [isTransparent](const std::optional<Fill>& style,
                                                const std::optional<Fill>& inherited,
                                                const Fill& markup) {
                if (!style.has_value() || (isTransparent && style == inherited))
                    return markup;

                return *style;
            };

            result.push_back({
                choose(fill, parentFill, markupFill),
                choose(stroke, parentStroke, markupStroke),
            });
        }

        return result;
    }

    float ImageComponent::getMinimumRequiredWidth() const
    {
        if (!intrinsicBounds.isEmpty())
            return intrinsicBounds.getWidth();

        if (drawable == nullptr)
            return 0.0f;

        return drawable->getDrawableBounds().getWidth();
    }

    float ImageComponent::getMinimumRequiredHeight() const
    {
        if (!intrinsicBounds.isEmpty())
            return intrinsicBounds.getHeight();

        if (drawable == nullptr)
            return 0.0f;

        return drawable->getDrawableBounds().getHeight();
    }
} // namespace jive
