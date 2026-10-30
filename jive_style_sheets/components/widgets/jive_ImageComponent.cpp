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

        if (drawable != nullptr)
            findShapesUsingCurrentColour(*drawable, currentColourFills, currentColourStrokes);

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

    // Shapes are drawn in their own coordinates (since JUCE 9, an SVG's
    // shapes each have their own transform), so gradients are laid out over
    // the element's bounds as seen from each one.
    static void setStyles(juce::Drawable& drawable,
                          juce::Rectangle<float> bounds,
                          const std::optional<Fill>& fill,
                          const std::optional<Fill>& stroke)
    {
        if (isHidden(drawable))
            return;

        const auto localBounds = bounds.transformedBy(getTransform(drawable).inverted());

        if (auto* shape = dynamic_cast<juce::DrawableShape*>(&drawable))
        {
            if (fill.has_value())
                shape->setFill(fill->toJuceFillType(localBounds));

            if (stroke.has_value())
                shape->setStrokeFill(stroke->toJuceFillType(localBounds));

            return;
        }

        if (auto* composite = dynamic_cast<juce::DrawableComposite*>(&drawable))
        {
            forEachChild(*composite, [&](juce::Drawable& child) {
                setStyles(child, localBounds, fill, stroke);
            });
        }
    }

    static juce::Path getVisibleOutline(const juce::Drawable& drawable)
    {
        if (isHidden(drawable))
            return {};

        if (const auto* shape = dynamic_cast<const juce::DrawableShape*>(&drawable))
            return shape->getOutlineAsPath();

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

        setStyles(*drawable,
                  getLocalBounds().toFloat(),
                  styles.fill.hasValue() ? findFill(*this, styles) : std::nullopt,
                  styles.stroke.hasValue() ? findStroke(*this, styles) : std::nullopt);

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
