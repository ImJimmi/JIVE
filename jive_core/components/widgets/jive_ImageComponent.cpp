#include "jive_ImageComponent.h"

namespace jive
{
    ImageComponent::ImageComponent()
    {
        setInterceptsMouseClicks(false, false);
    }

    static void hideTransparentChildren(juce::DrawableComposite& composite)
    {
        for (auto* child : composite.getChildren())
        {
            if (auto* shape = dynamic_cast<juce::DrawableShape*>(child))
            {
                if (shape->getFill().isInvisible()
                    && (shape->getStrokeFill().isInvisible()
                        || juce::approximatelyEqual(shape->getStrokeType().getStrokeThickness(), 0.0f)))
                {
                    shape->setVisible(false);
                }
            }
            else if (auto* compositeChild = dynamic_cast<juce::DrawableComposite*>(child))
            {
                hideTransparentChildren(*compositeChild);
            }
        }
    }

    void ImageComponent::setDrawable(std::unique_ptr<juce::Drawable>&& d,
                                     juce::Rectangle<float> intrinsicBoundsToUse,
                                     juce::Rectangle<float> viewBoxToUse)
    {
        drawable = std::move(d);
        intrinsicBounds = intrinsicBoundsToUse;
        viewBox = viewBoxToUse;

        if (drawable != nullptr)
            addAndMakeVisible(*drawable);

        if (auto* composite = dynamic_cast<juce::DrawableComposite*>(drawable.get()))
            hideTransparentChildren(*composite);

        resized();
    }

    static void setStyles(juce::Drawable& drawable,
                          const std::optional<juce::FillType>& fill,
                          const std::optional<juce::FillType>& stroke)
    {
        if (!drawable.isVisible())
            return;

        if (auto* shape = dynamic_cast<juce::DrawableShape*>(&drawable))
        {
            if (fill.has_value())
                shape->setFill(*fill);

            if (stroke.has_value())
                shape->setStrokeFill(*stroke);

            return;
        }

        if (auto* composite = dynamic_cast<juce::DrawableComposite*>(&drawable))
        {
            for (auto* child : composite->getChildren())
            {
                if (auto* drawableChild = dynamic_cast<juce::Drawable*>(child))
                {
                    setStyles(*drawableChild, fill, stroke);
                }
            }
        }
    }

    static juce::Path getVisibleOutline(const juce::Drawable& drawable)
    {
        if (!drawable.isVisible())
            return {};

        if (const auto* shape = dynamic_cast<const juce::DrawableShape*>(&drawable))
            return shape->getOutlineAsPath();

        juce::Path outline;

        for (auto* child : drawable.getChildren())
        {
            if (const auto* drawableChild = dynamic_cast<const juce::Drawable*>(child))
                outline.addPath(getVisibleOutline(*drawableChild));
        }

        outline.applyTransform(drawable.getTransform());
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

        // Shapes are drawn in the drawable's own coordinates (an SVG's
        // view-box), so gradients are laid out over the element's bounds as
        // seen from there.
        const auto bounds = getLocalBounds()
                                .toFloat()
                                .transformedBy(drawable->getTransform().inverted());
        const auto toFillType = [&bounds](const std::optional<Fill>& fill) -> std::optional<juce::FillType> {
            if (!fill.has_value())
                return std::nullopt;

            return fill->toJuceFillType(bounds);
        };
        setStyles(*drawable,
                  styles.fill.hasValue() ? toFillType(findFill(*this, styles)) : std::nullopt,
                  styles.stroke.hasValue() ? toFillType(findStroke(*this, styles)) : std::nullopt);

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
                composite->setBoundingBox(juce::RectanglePlacement{ juce::RectanglePlacement::centred }
                                              .appliedTo(viewBox, getLocalBounds().toFloat()));
            }

            return;
        }

        drawable->setBounds(getLocalBounds());
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
