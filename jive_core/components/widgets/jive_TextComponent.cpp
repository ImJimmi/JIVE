#include "jive_TextComponent.h"

#include <jive_core/jive_core.h>

namespace jive
{
    TextComponent::TextComponent()
    {
        setInterceptsMouseClicks(false, false);
    }

    void TextComponent::paint(juce::Graphics& g)
    {
        JIVE_TRACE();
        updateCachedFont();

        if (dynamic_cast<TextComponent*>(getParentComponent()) != nullptr)
            return;

        if (auto* laf = dynamic_cast<LookAndFeel*>(&getLookAndFeel()))
            laf->drawComponent(g, *this);

        const auto attributedString = getAttributedString();

        if (attributedString.getText().isEmpty() || !g.clipRegionIntersects(getLocalBounds()))
            return;

        const auto bounds = getLocalBounds().toFloat();
        getLayoutToDraw(attributedString, bounds.getWidth()).draw(g, bounds);
    }

    void TextComponent::lookAndFeelChanged()
    {
        updateCachedFont();
    }

    void TextComponent::parentHierarchyChanged()
    {
        updateCachedFont();
    }

    const juce::String& TextComponent::getText() const
    {
        return text;
    }

    void TextComponent::setText(const juce::String& newText)
    {
        if (newText != text)
        {
            text = newText;
            repaint();
        }
    }

    void TextComponent::setLineSpacing(float newLineSpacing)
    {
        if (!juce::approximatelyEqual(newLineSpacing, lineSpacing))
        {
            lineSpacing = newLineSpacing;
            repaint();
        }
        updateCachedFont();
    }

    void TextComponent::setWordWrap(juce::AttributedString::WordWrap newWordWrap)
    {
        if (newWordWrap != wordWrap)
        {
            wordWrap = newWordWrap;
            repaint();
        }
    }

    void TextComponent::clearAttributes()
    {
        appendices.clear();
        repaint();
    }

    void TextComponent::append(const juce::AttributedString& attributedStringToAppend)
    {
        appendices.add(attributedStringToAppend);
        repaint();
    }

    juce::AttributedString TextComponent::getAttributedString() const
    {
        juce::AttributedString attributedString;

        attributedString.setText(text);
        attributedString.setLineSpacing(lineSpacing);
        attributedString.setWordWrap(wordWrap);

        for (const auto& appendix : appendices)
            attributedString.append(appendix);

        if (auto* laf = dynamic_cast<LookAndFeel*>(&getLookAndFeel()))
        {
            const auto styles = laf->findMostApplicableStyles(*this);

            attributedString.setReadingDirection(getReadingDirection(*this, styles));
            attributedString.setJustification(getTextAlignment(*this, styles));

            if (cachedFont.has_value())
                attributedString.setFont(*cachedFont);

            const auto foreground = getForegroundFill(*this, styles);

            if (foreground.isColour())
                attributedString.setColour(foreground.colour);
            else if (foreground.isGradient())
                attributedString.setColour(foreground.gradient->getColour(0));
        }

        return attributedString;
    }

    void TextComponent::addListener(Listener& listener) const
    {
        listeners.add(&listener);
    }

    void TextComponent::removeListener(Listener& listener) const
    {
        listeners.remove(&listener);
    }

    class TextComponent::AccessibilityInterface : public juce::AccessibilityTextValueInterface
    {
    public:
        explicit AccessibilityInterface(TextComponent& componentToRepresent)
            : textComponent{ componentToRepresent }
        {
        }

        bool isReadOnly() const override
        {
            return true;
        }

        juce::String getCurrentValueAsString() const override
        {
            return textComponent.getText();
        }

        void setValueAsString(const juce::String& newValue) override
        {
            textComponent.setText(newValue);
        }

    private:
        TextComponent& textComponent;
    };

    class TextComponent::AccessibilityHandler : public juce::AccessibilityHandler
    {
    public:
        explicit AccessibilityHandler(TextComponent& componentToRepresent)
            : juce::AccessibilityHandler{
                componentToRepresent,
                juce::AccessibilityRole::staticText,
                juce::AccessibilityActions{},
                {
                    std::make_unique<AccessibilityInterface>(componentToRepresent),
                },
            }
            , textComponent{ componentToRepresent }
        {
        }

        juce::String getTitle() const override
        {
            if (textComponent.getTitle().isNotEmpty())
                return textComponent.getTitle();

            return textComponent.getText();
        }

        juce::String getDescription() const override
        {
            if (textComponent.getDescription().isNotEmpty())
                return textComponent.getDescription();

            if (textComponent.getTitle().isNotEmpty())
                return textComponent.getText();

            return "";
        }

    private:
        TextComponent& textComponent;
    };

    std::unique_ptr<juce::AccessibilityHandler> TextComponent::createAccessibilityHandler()
    {
        return std::make_unique<AccessibilityHandler>(*this);
    }

    [[nodiscard]] static auto isEquivalent(const juce::AttributedString& a, const juce::AttributedString& b)
    {
        if (a.getText() != b.getText()
            || a.getJustification() != b.getJustification()
            || a.getWordWrap() != b.getWordWrap()
            || a.getReadingDirection() != b.getReadingDirection()
            || !juce::exactlyEqual(a.getLineSpacing(), b.getLineSpacing())
            || a.getNumAttributes() != b.getNumAttributes())
        {
            return false;
        }

        for (auto i = 0; i < a.getNumAttributes(); i++)
        {
            const auto& first = a.getAttribute(i);
            const auto& second = b.getAttribute(i);

            if (first.range != second.range
                || first.colour != second.colour
                || first.font != second.font)
            {
                return false;
            }
        }

        return true;
    }

    // Shaping text is expensive, and most repaints draw exactly the same text
    // as last time. When only the width has changed (e.g. while the window is
    // being resized), text that fits without wrapping is laid out identically
    // at any width, so its unwrapped layout can be reused as well.
    const juce::TextLayout& TextComponent::getLayoutToDraw(const juce::AttributedString& attributedString,
                                                           float width)
    {
        if (!isEquivalent(lastDrawnString, attributedString))
        {
            lastDrawnString = attributedString;
            unwrappedLayout.reset();
            wrappedLayoutWidth.reset();
        }
        else if (wrappedLayoutWidth.has_value() && juce::exactlyEqual(*wrappedLayoutWidth, width))
        {
            return wrappedLayout;
        }
        else
        {
            if (!unwrappedLayout.has_value())
            {
                unwrappedLayout.emplace();
                unwrappedLayout->createLayout(attributedString,
                                              static_cast<float>(std::numeric_limits<juce::uint16>::max()));
            }

            if (unwrappedLayout->getWidth() < width)
                return *unwrappedLayout;
        }

        wrappedLayout.createLayout(attributedString, width);
        wrappedLayoutWidth = width;

        return wrappedLayout;
    }

    void TextComponent::updateCachedFont()
    {
        if (auto* laf = dynamic_cast<LookAndFeel*>(&getLookAndFeel()))
        {
            if (auto newFont = getFont(*this, laf->findMostApplicableStyles(*this));
                newFont != cachedFont)
            {
                cachedFont = newFont;
                listeners.call(&Listener::textFontChanged, *this);
            }
        }
    }
} // namespace jive

#if JIVE_UNIT_TESTS
class TextComponentUnitTest : public juce::UnitTest
{
public:
    TextComponentUnitTest()
        : juce::UnitTest{ "jive::TextComponent", "jive" }
    {
    }

    void runTest() final
    {
        testRewrapsWhenNarrowed();
    }

private:
    [[nodiscard]] static auto hasInkBetween(const juce::Image& image, int top, int bottom)
    {
        for (auto y = top; y < bottom; y++)
        {
            for (auto x = 0; x < image.getWidth(); x++)
            {
                if (image.getPixelAt(x, y).getAlpha() > 0)
                    return true;
            }
        }

        return false;
    }

    [[nodiscard]] static auto paint(jive::TextComponent& text)
    {
        juce::Image image{ juce::Image::ARGB, text.getWidth(), text.getHeight(), true };
        juce::Graphics g{ image };
        text.paintEntireComponent(g, true);

        return image;
    }

    void testRewrapsWhenNarrowed()
    {
        beginTest("re-wraps when narrowed after being drawn");

        jive::TextComponent text;
        text.setText("Wrapping text");

        const auto lineHeight = static_cast<int>(std::ceil(juce::Font{ juce::FontOptions{} }.getHeight()));
        const auto secondLineTop = lineHeight + 2;
        const auto secondLineBottom = lineHeight * 2;

        text.setSize(1000, lineHeight * 3);
        expect(!hasInkBetween(paint(text), secondLineTop, secondLineBottom));

        text.setSize(1001, lineHeight * 3);
        expect(!hasInkBetween(paint(text), secondLineTop, secondLineBottom));

        text.setSize(40, lineHeight * 3);
        expect(hasInkBetween(paint(text), secondLineTop, secondLineBottom));

        text.setSize(1000, lineHeight * 3);
        expect(!hasInkBetween(paint(text), secondLineTop, secondLineBottom));
    }
};

static TextComponentUnitTest textComponentUnitTest;
#endif
