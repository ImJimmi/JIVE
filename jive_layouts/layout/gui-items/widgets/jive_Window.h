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
        juce::DocumentWindow& getWindow();
        const juce::DocumentWindow& getWindow() const;

    protected:
        //==============================================================================================================
        void componentMovedOrResized(juce::Component& component, bool wasMoved, bool wasResized) override;

    private:
        //==============================================================================================================
        int getStyleFlags() const;

        //==============================================================================================================
        DocumentWindow window;

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
        Length width;
        Length height;

        //==============================================================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Window)
    };
} // namespace jive
