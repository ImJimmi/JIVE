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
        void componentMovedOrResized(juce::Component&, bool, bool) override;
        void boxModelChanged(BoxModel& boxModelThatChanged) override;
        void valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier& id) override;

    private:
        //==============================================================================================================
        float calculateAspectRatio(const juce::ImageComponent& image) const;
        float calculateRequiredWidth(const juce::ImageComponent& image) const;
        float calculateRequiredWidth(const juce::Drawable& drawable) const;
        float calculateRequiredWidth() const;
        float calculateRequiredHeight(const juce::ImageComponent& image) const;
        float calculateRequiredHeight(const juce::Drawable& drawable) const;
        float calculateRequiredHeight() const;

        std::unique_ptr<juce::ImageComponent> createImageComponent(const juce::Image& image) const;
        std::unique_ptr<juce::Drawable> createSVG(const juce::String& svgString) const;
        std::unique_ptr<juce::Component> createChildComponent() const;
        void setChildComponent(std::unique_ptr<juce::Component> newComponent);

        //==============================================================================================================
        TypedValue<Drawable> source;
        std::unique_ptr<juce::Component> childComponent;

        TypedValue<juce::RectanglePlacement> placement;
        Length width;
        Length height;
        Length idealWidth;
        Length idealHeight;

        //==============================================================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Image)
    };
} // namespace jive
