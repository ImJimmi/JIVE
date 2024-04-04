#pragma once

#include <jive_layouts/layout/gui-items/jive_GuiItemDecorator.h>

#include <jive_components/jive_components.h>

namespace jive
{
    class Window : public GuiItemDecorator
    {
    public:
        explicit Window(std::unique_ptr<GuiItem> itemToDecorate);
        ~Window();

        juce::DocumentWindow& getWindow();
        const juce::DocumentWindow& getWindow() const;

    private:
        int getStyleFlags() const;

        DocumentWindow window;
        LookAndFeel lookAndFeel;

        Property<bool> hasShadow;
        Property<bool> isNative;
        Property<bool> isResizable;
        Property<bool> useCornerResizer;
        Property<float> minWidth;
        Property<float> minHeight;
        Property<float> maxWidth;
        Property<float> maxHeight;
        Property<bool> isDraggable;
        Property<bool> isFullScreen;
        Property<bool> isMinimised;
        Property<juce::String> name;
        Property<float> titleBarHeight;
        Property<juce::DocumentWindow::TitleBarButtons> titleBarButtons;
        Length width;
        Length height;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Window)
    };
} // namespace jive
