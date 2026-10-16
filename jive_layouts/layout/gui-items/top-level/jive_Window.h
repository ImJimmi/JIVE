#pragma once

#include "jive_TopLevelGuiItem.h"

namespace jive
{
    class Window
        : public TopLevelGuiItem
        , public DocumentWindow
        , private juce::ValueTree::Listener
    {
    public:
        explicit Window(std::unique_ptr<GuiItem> itemToDecorate);
        ~Window() override;

        void replaceDecoratedItem(std::unique_ptr<GuiItem> newItem) override;

    private:
        int getStyleFlags() const;
        void valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property) final;

        LookAndFeel jiveLookAndFeel{ *getComponent() };

        PassiveProperty<bool> hasShadow;
        PassiveProperty<bool> isNative;
        PassiveProperty<bool> canBeResized;
        PassiveProperty<bool> useCornerResizer;
        PassiveProperty<float> minWidth;
        PassiveProperty<float> minHeight;
        PassiveProperty<float> maxWidth;
        PassiveProperty<float> maxHeight;
        PassiveProperty<bool> draggable;
        PassiveProperty<bool> fullScreen;
        PassiveProperty<bool> minimised;
        PassiveProperty<bool> visibility;
        PassiveProperty<juce::String> name;
        PassiveProperty<float> windowTitleBarHeight;
        PassiveProperty<juce::DocumentWindow::TitleBarButtons> windowTitleBarButtons;
        PassiveProperty<Length> width;
        PassiveProperty<Length> height;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Window)
    };
} // namespace jive
