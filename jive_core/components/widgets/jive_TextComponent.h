#pragma once

#include <jive_core/components/canvases/jive_Canvas.h>

namespace jive
{
    class TextComponent : public juce::Component
    {
    public:
        class AccessibilityHandler;
        class AccessibilityInterface;

        struct Listener
        {
            JUCE_DECLARE_NON_COPYABLE(Listener)
            JUCE_DECLARE_NON_MOVEABLE(Listener)
            virtual ~Listener() = default;

            virtual void textFontChanged(TextComponent& text) = 0;
        };

        TextComponent();

        void paint(juce::Graphics& g) override;
        void lookAndFeelChanged() override;
        void parentHierarchyChanged() override;
        std::unique_ptr<juce::AccessibilityHandler> createAccessibilityHandler() override;

        [[nodiscard]] const juce::String& getText() const;
        void setText(const juce::String& text);

        void setWordWrap(juce::AttributedString::WordWrap wrap);
        [[nodiscard]] float getLineSpacing() const;

        void clearAttributes();
        void append(const juce::AttributedString& attributedStringToAppend);
        [[nodiscard]] juce::AttributedString getAttributedString() const;

        // The returned layout is only valid until the next call.
        [[nodiscard]] const juce::TextLayout& getTextLayout(float maxWidth) const;

        void addListener(Listener&) const;
        void removeListener(Listener&) const;

    private:
        struct CachedTextLayout
        {
            float maxWidth;
            juce::TextLayout layout;
        };

        void updateCachedStyles();
        void updateTextLayoutsSource(const juce::AttributedString& attributedString) const;

        static constexpr std::size_t maxNumCachedTextLayouts = 16;

        juce::String text;
        std::vector<juce::AttributedString> appendices;
        mutable std::optional<juce::Font> cachedFont;

        float lineSpacing{ 0.0f };
        juce::AttributedString::WordWrap wordWrap{ juce::AttributedString::WordWrap::byWord };

        mutable juce::ListenerList<Listener> listeners;

        mutable juce::AttributedString textLayoutsSource;
        mutable bool textLayoutsSourceMayBeOutOfDate{ true };
        mutable std::vector<CachedTextLayout> textLayouts;

        JUCE_LEAK_DETECTOR(TextComponent)
    };
} // namespace jive
