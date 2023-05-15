#pragma once

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    class BackgroundCanvas : public IgnoredComponent
    {
    public:
        //==============================================================================================================
        BackgroundCanvas();

        //==============================================================================================================
        void paint(juce::Graphics& g) final;
        void resized() final;

        //==============================================================================================================
        Fill getFill() const;
        void setFill(const Fill& newFill);

        Fill getBorderFill() const;
        void setBorderFill(const Fill& newFill);
        float getBorderWidth() const;
        void setBorderWidth(float newWidth);
        BorderRadii<float> getBorderRadii() const;
        void setBorderRadii(BorderRadii<float> radii);

    private:
        //==============================================================================================================
        void updateShape();

        //==============================================================================================================
        Fill background{ juce::Colours::transparentBlack };
        Fill borderFill{ juce::Colours::transparentBlack };
        float borderWidth{ 0.0f };
        BorderRadii<float> borderRadii;

        juce::Path shape;

        //==============================================================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BackgroundCanvas)
    };
} // namespace jive
