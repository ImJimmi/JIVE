#pragma once

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    class ProgressBar
        : public GuiItemDecorator
        , public TextWidget
    {
    public:
        //==============================================================================================================
        explicit ProgressBar(std::unique_ptr<GuiItem> itemToDecorate);

        //==============================================================================================================
        NormalisedProgressBar& getProgressBar();
        const NormalisedProgressBar& getProgressBar() const;

    private:
        //==============================================================================================================
        TypedValue<double> value;
        TypedValue<bool> showPercentage;

        //==============================================================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProgressBar)
    };
} // namespace jive
