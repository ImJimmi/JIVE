#pragma once

#include "jive_TopLevelGuiItem.h"

namespace jive
{
    using TitleBarHeight = Number<float, struct TitleBarHeightTag>;

    class Window
        : public TopLevelGuiItem
        , public DocumentWindow
        , private juce::ValueTree::Listener
    {
    public:
        explicit Window(std::unique_ptr<GuiItem> itemToDecorate);
        ~Window() override;

        void replaceDecoratedItem(std::unique_ptr<GuiItem> newItem) override;

        void paint(juce::Graphics& g) override;
        void resized() override;

    private:
        int getStyleFlags() const;
        void applyTheme();
        [[nodiscard]] juce::Rectangle<int> getDisplayArea() const;
        void updateResizeLimits();
        [[nodiscard]] float getDefaultTopPadding() const;
        void updateIntegratedTitleBar();
        void updateTitleBarColour();
        void updateBackdrop();
        void applyBackdrop(std::optional<Backdrop> backdrop);
        void moved() override;
        void parentSizeChanged() override;
        void valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property) final;

        static constexpr auto defaultMacTitleBarHeight = 28.0f;

        // Read by resized(), which the look-and-feel below triggers.
        PassiveProperty<bool> integratedTitleBar;
        PassiveProperty<TitleBarHeight> windowTitleBarHeight;

        // Attached to the window itself, which outlives every component a
        // reload swaps in, so that transitions keep being repainted.
        LookAndFeel jiveLookAndFeel{ *this };

        PassiveProperty<bool> hasShadow;
        PassiveProperty<bool> isNative;
        PassiveProperty<bool> canBeResized;
        PassiveProperty<bool> useCornerResizer;
        PassiveProperty<Length> minWidth;
        PassiveProperty<Length> minHeight;
        PassiveProperty<Length> maxWidth;
        PassiveProperty<Length> maxHeight;
        PassiveProperty<bool> draggable;
        PassiveProperty<bool> fullScreen;
        PassiveProperty<bool> minimised;
        PassiveProperty<bool> visibility;
        PassiveProperty<juce::String> name;
        PassiveProperty<juce::DocumentWindow::TitleBarButtons> windowTitleBarButtons;
        PassiveProperty<Length> width;
        PassiveProperty<Length> height;

        juce::Rectangle<int> resizeLimitsArea;

        class TitleBarDragger : public juce::MouseListener
        {
        public:
            explicit TitleBarDragger(Window& owner);

            void mouseDown(const juce::MouseEvent& event) final;

        private:
            Window& window;
        };

        TitleBarDragger titleBarDragger{ *this };
#if JUCE_MAC || JUCE_WINDOWS
        std::optional<Backdrop> appliedBackdrop;
        void* backdropHandle = nullptr;
#endif
#if JUCE_MAC
        void* integratedTitleBarHandle = nullptr;
#elif JUCE_WINDOWS
        std::optional<juce::Colour> appliedTitleBarColour;
        void* titleBarColourHandle = nullptr;
#endif

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Window)
    };
} // namespace jive
