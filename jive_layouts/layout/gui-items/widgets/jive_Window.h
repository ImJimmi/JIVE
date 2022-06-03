#pragma once

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    class Window : public GuiItemDecorator
    {
    public:
        //==============================================================================================================
        explicit Window(std::unique_ptr<GuiItem> itemToDecorate);

        //==============================================================================================================
        void addChild(std::unique_ptr<GuiItem> child) override;

        //==============================================================================================================
        juce::DocumentWindow& getWindow();
        const juce::DocumentWindow& getWindow() const;

    private:
        //==============================================================================================================
        int getStyleFlags() const;

        //==============================================================================================================
        TypedValue<bool> hasShadow;
        TypedValue<bool> isNative;
        TypedValue<bool> isResizable;
        TypedValue<bool> useCornerResizer;
        TypedValue<float> minWidth;
        TypedValue<float> minHeight;
        TypedValue<float> maxWidth;
        TypedValue<float> maxHeight;
        TypedValue<bool> isDraggable;
        TypedValue<bool> isFullScreen;
        TypedValue<bool> isMinimised;
        TypedValue<juce::String> name;
        TypedValue<float> titleBarHeight;
        TypedValue<juce::DocumentWindow::TitleBarButtons> titleBarButtons;

        //==============================================================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Window)
    };
} // namespace jive
