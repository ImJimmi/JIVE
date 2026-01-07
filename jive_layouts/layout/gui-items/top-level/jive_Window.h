#pragma once

#include "jive_TopLevelGuiItem.h"

namespace jive
{
    class Window
        : public TopLevelGuiItem
        , public DocumentWindow
    {
    public:
        explicit Window(std::unique_ptr<GuiItem> itemToDecorate);
        ~Window() override;

        void replaceDecoratedItem(std::unique_ptr<GuiItem> newItem) override;

    private:
        int getStyleFlags() const;

        LookAndFeel jiveLookAndFeel{ *getComponent() };

        Property<bool> hasShadow;
        Property<bool> isNative;
        Property<bool> canBeResized;
        Property<bool> useCornerResizer;
        Property<float> minWidth;
        Property<float> minHeight;
        Property<float> maxWidth;
        Property<float> maxHeight;
        Property<bool> draggable;
        Property<bool> fullScreen;
        Property<bool> minimised;
        Property<bool> visibility;
        Property<juce::String> name;
        Property<float> windowTitleBarHeight;
        Property<juce::DocumentWindow::TitleBarButtons> windowTitleBarButtons;
        Property<Length> width;
        Property<Length> height;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Window)
    };
} // namespace jive
