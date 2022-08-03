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

        const juce::ImageComponent& getImage() const;
        juce::ImageComponent& getImage();

    private:
        //==============================================================================================================
        TypedValue<juce::Image> image;
        TypedValue<juce::RectanglePlacement> placement;

        //==============================================================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Image)
    };
} // namespace jive
