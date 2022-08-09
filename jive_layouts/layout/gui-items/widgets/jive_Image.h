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
        float getWidth() const override;
        float getHeight() const override;

        bool isContainer() const override;

        Drawable getDrawable();

    protected:
        //==============================================================================================================
        void componentMovedOrResized(juce::Component& component, bool wasMoved, bool wasResized) override;

    private:
        //==============================================================================================================
        void updateChildComponent();
        void updateChildBounds();
        void updatePlacement();

        //==============================================================================================================
        TypedValue<Drawable> source;
        TypedValue<juce::RectanglePlacement> placement;

        std::unique_ptr<juce::Component> childComponent;

        //==============================================================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Image)
    };
} // namespace jive
