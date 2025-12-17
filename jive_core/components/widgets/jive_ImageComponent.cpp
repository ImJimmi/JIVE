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

    void ImageComponent::setDrawable(std::unique_ptr<juce::Drawable>&& d)
    {
        drawable = std::move(d);

        if (drawable != nullptr)
            addAndMakeVisible(*drawable);

        if (auto* composite = dynamic_cast<juce::DrawableComposite*>(drawable.get()))
            hideTransparentChildren(*composite);
    }

    static void setStyles(juce::Drawable& drawable,
                          ImageComponent& image,
                          const Styles& styles)
    {
        if (!drawable.isVisible())
            return;

        if (auto* shape = dynamic_cast<juce::DrawableShape*>(&drawable))
        {
            if (styles.fill.hasValue())
                shape->setFill(getFill(image, styles));

            if (styles.stroke.hasValue())
                shape->setStrokeFill(getStroke(image, styles));

            return;
        }

        if (auto* composite = dynamic_cast<juce::DrawableComposite*>(&drawable))
        {
            for (auto* child : composite->getChildren())
            {
                if (auto* drawableChild = dynamic_cast<juce::Drawable*>(child))
                {
                    setStyles(*drawableChild, image, styles);
                }
            }
        }
    }

    void ImageComponent::paint(juce::Graphics&)
    {
        if (drawable != nullptr)
        {
            if (auto* laf = dynamic_cast<LookAndFeel*>(&getLookAndFeel()))
            {
                const auto styles = laf->findMostApplicableStyles(*this);
                setStyles(*drawable, *this, styles);
            }
        }
    }

    void ImageComponent::resized()
    {
        if (drawable != nullptr)
            drawable->setBounds(getLocalBounds());
    }

    float ImageComponent::getMinimumRequiredWidth() const
    {
        if (drawable == nullptr)
            return 0.0f;

        return drawable->getDrawableBounds().getWidth();
    }

    float ImageComponent::getMinimumRequiredHeight() const
    {
        if (drawable == nullptr)
            return 0.0f;

        return drawable->getDrawableBounds().getHeight();
    }
} // namespace jive
