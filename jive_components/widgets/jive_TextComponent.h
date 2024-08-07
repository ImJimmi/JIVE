#pragma once

#include <jive_components/canvases/jive_Canvas.h>

#include <juce_gui_basics/juce_gui_basics.h>

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

        void resized() override;

        void setDirection(juce::AttributedString::ReadingDirection direction);

        [[nodiscard]] juce::Font getFont() const;
        void setFont(const juce::Font& font);

        void setJustification(juce::Justification justification);

        void setLineSpacing(float spacing);

        [[nodiscard]] const juce::String& getText() const;
        void setText(const juce::String& text);

        [[nodiscard]] juce::Colour getTextColour() const;
        void setTextColour(juce::Colour newColour);

        void setWordWrap(juce::AttributedString::WordWrap wrap);

        void clearAttributes();
        void append(const juce::AttributedString& attributedStringToAppend);

        [[nodiscard]] juce::AttributedString getAttributedString() const;

        void addListener(Listener&) const;
        void removeListener(Listener&) const;

    private:
        std::unique_ptr<juce::AccessibilityHandler> createAccessibilityHandler() override;

        juce::AttributedString::ReadingDirection direction{ juce::AttributedString::ReadingDirection::natural };
        juce::Font font{
#if JUCE_MAJOR_VERSION >= 8
            juce::FontOptions{},
#endif
        };
        juce::Justification justification{ juce::Justification::topLeft };
        float lineSpacing{ 0.0f };
        juce::String text;
        std::optional<juce::Colour> textColour{ juce::Colours::black };
        juce::AttributedString::WordWrap wordWrap{ juce::AttributedString::WordWrap::byWord };
        juce::Array<juce::AttributedString> appendices;

        Canvas canvas;

        mutable juce::ListenerList<Listener> listeners;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TextComponent)
    };
} // namespace jive
