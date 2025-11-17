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
            virtual ~Listener() = default;

            virtual void textFontChanged(TextComponent& text) = 0;
        };

        TextComponent();
        JUCE_DECLARE_NON_COPYABLE(TextComponent)

        void paint(juce::Graphics& g) override;
        void lookAndFeelChanged() override;
        void parentHierarchyChanged() override;

        [[nodiscard]] const juce::String& getText() const;
        void setText(const juce::String& text);

        void setLineSpacing(float spacing);
        void setWordWrap(juce::AttributedString::WordWrap wrap);

        void clearAttributes();
        void append(const juce::AttributedString& attributedStringToAppend);
        [[nodiscard]] juce::AttributedString getAttributedString() const;

        void addListener(Listener&) const;
        void removeListener(Listener&) const;

    private:
        std::unique_ptr<juce::AccessibilityHandler> createAccessibilityHandler() override;

        void updateCachedFont();

        juce::String text;
        juce::Array<juce::AttributedString> appendices;
        mutable std::optional<juce::Font> cachedFont;

        float lineSpacing{ 0.0f };
        juce::AttributedString::WordWrap wordWrap{ juce::AttributedString::WordWrap::byWord };

        mutable juce::ListenerList<Listener> listeners;
    };
} // namespace jive
