#pragma once

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    class Image : public GuiItemDecorator
    {
    public:
        //==============================================================================================================
        explicit Image(std::unique_ptr<GuiItem> itemToDecorate);

        //==============================================================================================================
        bool isContainer() const override;
        bool isContent() const override;

        Drawable getDrawable() const;

    protected:
        //==============================================================================================================
        void componentMovedOrResized(juce::Component& component, bool wasMoved, bool wasResized) override;

    private:
        //==============================================================================================================
        float calculateAspectRatio(const juce::ImageComponent& image) const;
        float calculateAutoWidth(const juce::ImageComponent& image) const;
        float calculateAutoWidth(const juce::Drawable& drawable) const;
        float calculateAutoWidth() const;
        float calculateAutoHeight(const juce::ImageComponent& image) const;
        float calculateAutoHeight(const juce::Drawable& drawable) const;
        float calculateAutoHeight() const;
        std::unique_ptr<juce::ImageComponent> createImageComponent(const juce::Image& image) const;
        std::unique_ptr<juce::Drawable> createSVG(const juce::String& svgString) const;
        std::unique_ptr<juce::Component> createChildComponent() const;
        void setChildComponent(std::unique_ptr<juce::Component> newComponent);

        //==============================================================================================================
        void updateExplicitSize();

        //==============================================================================================================
        TypedValue<Drawable> source;
        TypedValue<juce::RectanglePlacement> placement;

        std::unique_ptr<juce::Component> childComponent;

        //==============================================================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Image)
    };
} // namespace jive
