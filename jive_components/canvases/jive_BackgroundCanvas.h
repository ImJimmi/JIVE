#pragma once

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    class BackgroundCanvas : public IgnoredComponent
    {
    public:
        //==============================================================================================================
        BackgroundCanvas() = default;

        //==============================================================================================================
        void paint(juce::Graphics& g) final;

        //==============================================================================================================
        Fill getFill() const;
        void setFill(const Fill& newFill);

        Fill getBorderFill() const;
        float getBorderWidth() const;
        void setBorder(const Fill& newFill, float newWidth);

    private:
        //==============================================================================================================
        Fill background{ juce::Colours::transparentBlack };
        Fill borderFill{ juce::Colours::transparentBlack };
        float borderWidth{ 0.0f };

        //==============================================================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BackgroundCanvas)
    };
} // namespace jive
