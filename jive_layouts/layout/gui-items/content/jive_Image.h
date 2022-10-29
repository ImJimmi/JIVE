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

        float calculateAutoWidth() const override;
        float calculateAutoHeight() const override;

        Drawable getDrawable() const;

    protected:
        //==============================================================================================================
        void boxModelChanged(BoxModel& boxModelThatChanged) override;

    private:
        //==============================================================================================================
        float calculateAspectRatio(const juce::ImageComponent& image) const;
        float calculateRequiredWidth(const juce::ImageComponent& image) const;
        float calculateRequiredWidth(const juce::Drawable& drawable) const;
        float calculateRequiredHeight(const juce::ImageComponent& image) const;
        float calculateRequiredHeight(const juce::Drawable& drawable) const;
        float calculateRequiredHeight() const;
        std::unique_ptr<juce::ImageComponent> createImageComponent(const juce::Image& image) const;
        std::unique_ptr<juce::Drawable> createSVG(const juce::String& svgString) const;
        std::unique_ptr<juce::Component> createChildComponent() const;
        void setChildComponent(std::unique_ptr<juce::Component> newComponent);

        //==============================================================================================================
        TypedValue<Drawable> source;
        TypedValue<juce::RectanglePlacement> placement;

        std::unique_ptr<juce::Component> childComponent;

        //==============================================================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Image)
    };
} // namespace jive
