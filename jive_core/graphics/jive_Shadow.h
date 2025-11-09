#pragma once

#include <jive_core/geometry/jive_BorderRadii.h>

#include <juce_graphics/juce_graphics.h>

namespace jive
{
    // Represents an outer box shadow that can be drawn for a given path.
    // Can be constructed implicitly from a CSS-syntax string.
    class Shadow
    {
    public:
        Shadow() = default;
        Shadow(const Shadow&) = default;
        Shadow(Shadow&&) = default;
        Shadow& operator=(const Shadow&) = default;
        Shadow& operator=(Shadow&&) = default;
        ~Shadow() = default;

        // Uses CSS syntax to parse the given string to a shadow.
        // See https://developer.mozilla.org/en-US/docs/Web/CSS/box-shadow
        Shadow(const juce::String&);
        Shadow(const char*);

        // Draws the shadow to the given graphics context using the given path.
        void draw(juce::Graphics&,
                  const juce::Path&) const;

        Shadow& setOffset(const juce::Point<int>& newOffset);
        [[nodiscard]] juce::Point<int> getOffset() const;

        Shadow& setColour(const juce::Colour& newColour);
        [[nodiscard]] juce::Colour getColour() const;

        Shadow& setBlurRadius(float newBlurRadius);
        [[nodiscard]] float getBlurRadius() const;

        Shadow& setSpreadRadius(float newSpreadRadius);
        [[nodiscard]] float getSpreadRadius() const;

        // Uses CSS syntax to parse the given string to a shadow.
        // See https://developer.mozilla.org/en-US/docs/Web/CSS/box-shadow
        [[nodiscard]] static Shadow fromString(const juce::String&);

        [[nodiscard]] friend bool operator==(const Shadow& lhs, const Shadow& rhs)
        {
            static constexpr auto tie = [](const Shadow& s) {
                return std::make_tuple(s.getOffset(), s.getColour(), s.getBlurRadius(), s.getSpreadRadius());
            };

            return tie(lhs) == tie(rhs);
        }

    private:
        juce::Point<int> offset;
        juce::Colour colour;
        float blurRadius{ 0.0f };
        float spreadRadius{ 0.0f };
    };

    // A component that rendered the given shadow as its graphics.
    // Useful for showing shadows behind other components so those components
    // don't need to draw outside their bounds.
    class ShadowComponent
        : public juce::Component
        , private juce::ComponentListener
    {
    public:
        explicit ShadowComponent(juce::Component& componentToAttachTo);
        ~ShadowComponent() final;

        void paint(juce::Graphics& g) final;

        void setShadow(const Shadow& newShadow);

    private:
        void componentMovedOrResized(juce::Component&, bool, bool) final;
        void componentVisibilityChanged(juce::Component&) final;
        void componentParentHierarchyChanged(juce::Component&) final;

        void updateBounds();
        void updateVisibility();
        void updateParent();

        juce::Component::SafePointer<juce::Component> component;
        Shadow shadow;
        BorderRadii<float> borderRadius;
    };

    // Wraps a shadow component in a reference-counted object so it can be
    // safely stored in another components var properties.
    // Probably not useful for anything other than CLAF's own shadow
    // implementation.
    struct ReferenceCountedShadowComponent : juce::ReferenceCountedObject
    {
        explicit ReferenceCountedShadowComponent(std::unique_ptr<ShadowComponent>&& up)
            : shadow{ std::move(up) }
        {
        }

        std::unique_ptr<ShadowComponent> shadow;
    };
} // namespace jive
