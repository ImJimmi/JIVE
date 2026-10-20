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
        updateCachedStyles();

        if (dynamic_cast<TextComponent*>(getParentComponent()) != nullptr)
            return;

        if (auto* laf = dynamic_cast<LookAndFeel*>(&getLookAndFeel()))
            laf->drawComponent(g, *this);

        const auto attributedString = getAttributedString();

        if (attributedString.getText().isEmpty() || !g.clipRegionIntersects(getLocalBounds()))
            return;

        updateTextLayoutsSource(attributedString);

        const auto bounds = getLocalBounds().toFloat();
        getTextLayout(bounds.getWidth()).draw(g, bounds);
    }

    void TextComponent::lookAndFeelChanged()
    {
        updateCachedStyles();
    }

    void TextComponent::parentHierarchyChanged()
    {
        updateCachedStyles();
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
            textLayoutsSourceMayBeOutOfDate = true;
            repaint();
        }
    }

    void TextComponent::setWordWrap(juce::AttributedString::WordWrap newWordWrap)
    {
        if (newWordWrap != wordWrap)
        {
            wordWrap = newWordWrap;
            textLayoutsSourceMayBeOutOfDate = true;
            repaint();
        }
    }

    float TextComponent::getLineSpacing() const
    {
        return lineSpacing;
    }

    void TextComponent::clearAttributes()
    {
        appendices.clear();
        textLayoutsSourceMayBeOutOfDate = true;
        repaint();
    }

    void TextComponent::append(const juce::AttributedString& attributedStringToAppend)
    {
        appendices.add(attributedStringToAppend);
        textLayoutsSourceMayBeOutOfDate = true;
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

            const auto foreground = getForegroundFill(*this, styles, detail::getV4Colour(detail::V4Colour::defaultText));

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

    // A layout created at some maximum width breaks its lines in the same
    // places at any narrower width that its longest line still fits in, so it
    // can be reused at any of those widths instead of shaping the text again.
    // Text that isn't wrapped is laid out the same at every width.
    [[nodiscard]] static auto canBeReusedAt(const juce::TextLayout& layout,
                                            float createdAtMaxWidth,
                                            float maxWidth,
                                            juce::AttributedString::WordWrap wordWrap)
    {
        return wordWrap == juce::AttributedString::WordWrap::none
            || juce::exactlyEqual(maxWidth, createdAtMaxWidth)
            || (layout.getWidth() <= maxWidth && maxWidth <= createdAtMaxWidth);
    }

    const juce::TextLayout& TextComponent::getTextLayout(float maxWidth) const
    {
        JIVE_TRACE("max-width", maxWidth);

        if (textLayoutsSourceMayBeOutOfDate)
            updateTextLayoutsSource(getAttributedString());

        const auto reusable = std::find_if(std::begin(textLayouts),
                                           std::end(textLayouts),
                                           [this, maxWidth](const auto& cached) {
                                               return canBeReusedAt(cached.layout,
                                                                    cached.maxWidth,
                                                                    maxWidth,
                                                                    textLayoutsSource.getWordWrap());
                                           });

        if (reusable != std::end(textLayouts))
        {
            std::rotate(reusable, std::next(reusable), std::end(textLayouts));
            return textLayouts.back().layout;
        }

        if (std::size(textLayouts) >= maxNumCachedTextLayouts)
            textLayouts.erase(std::begin(textLayouts));

        auto& created = textLayouts.emplace_back(CachedTextLayout{ maxWidth, {} });
        created.layout.createLayout(textLayoutsSource, maxWidth);

        return created.layout;
    }

    void TextComponent::updateTextLayoutsSource(const juce::AttributedString& attributedString) const
    {
        if (!isEquivalent(textLayoutsSource, attributedString))
        {
            textLayoutsSource = attributedString;
            textLayouts.clear();
        }

        textLayoutsSourceMayBeOutOfDate = false;
    }

    void TextComponent::updateCachedStyles()
    {
        textLayoutsSourceMayBeOutOfDate = true;

        if (auto* laf = dynamic_cast<LookAndFeel*>(&getLookAndFeel()))
        {
            const auto styles = laf->findMostApplicableStyles(*this);
            const auto newFont = getFont(*this, styles);
            const auto newLineSpacing = styles.find<float>("line-spacing").value_or(0.0f);

            if (newFont != cachedFont || !juce::exactlyEqual(newLineSpacing, lineSpacing))
            {
                cachedFont = newFont;
                lineSpacing = newLineSpacing;
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
        testLayoutsMatchFreshLayouts();
        testLayoutsAreReusedAcrossWidths();
        testLayoutsFollowChanges();
        testDefaultForeground();
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

        const juce::Font defaultFont{
    #if JUCE_MAJOR_VERSION >= 8
            juce::FontOptions{},
    #endif
        };
        const auto lineHeight = static_cast<int>(std::ceil(defaultFont.getHeight()));
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

    void expectEquivalent(const juce::TextLayout& actual,
                          const juce::TextLayout& expected,
                          const juce::String& context)
    {
        static constexpr auto tolerance = 0.01f;

        expectEquals(actual.getNumLines(), expected.getNumLines(), context);
        expectWithinAbsoluteError(actual.getWidth(), expected.getWidth(), tolerance, context);
        expectWithinAbsoluteError(actual.getHeight(), expected.getHeight(), tolerance, context);

        for (auto i = 0; i < std::min(actual.getNumLines(), expected.getNumLines()); i++)
        {
            const auto& actualLine = actual.getLine(i);
            const auto& expectedLine = expected.getLine(i);

            expect(actualLine.stringRange == expectedLine.stringRange, context);
            expectWithinAbsoluteError(actualLine.lineOrigin.x, expectedLine.lineOrigin.x, tolerance, context);
            expectWithinAbsoluteError(actualLine.lineOrigin.y, expectedLine.lineOrigin.y, tolerance, context);
        }
    }

    void testLayoutsMatchFreshLayouts()
    {
        beginTest("layouts match fresh layouts at every width");

        static constexpr auto widest = 260.0f;
        static constexpr auto narrowest = 30.0f;

        for (const juce::Justification justification : {
                 juce::Justification::centredLeft,
                 juce::Justification::centred,
                 juce::Justification::centredRight,
                 juce::Justification::horizontallyJustified,
             })
        {
            for (const auto wordWrap : {
                     juce::AttributedString::WordWrap::byWord,
                     juce::AttributedString::WordWrap::byChar,
                     juce::AttributedString::WordWrap::none,
                 })
            {
                jive::TextComponent text;
                jive::LookAndFeel lookAndFeel{ text };
                lookAndFeel.addStyles(jive::StyleSelector::makeInlineStyleSelector(text),
                                      jive::Styles{}.withTextAlignment(justification));
                text.setText("The quick brown fox jumps over the lazy dog. Pack my box with five dozen liquor jugs.");
                text.setWordWrap(wordWrap);

                const auto expectEquivalentToFreshLayout = [&](float width) {
                    juce::TextLayout fresh;
                    fresh.createLayout(text.getAttributedString(), width);

                    expectEquivalent(text.getTextLayout(width),
                                     fresh,
                                     "justification " + juce::String{ justification.getFlags() }
                                         + ", word-wrap " + juce::String{ static_cast<int>(wordWrap) }
                                         + ", width " + juce::String{ width });
                };

                for (auto width = widest; width > narrowest; width -= 3.7f)
                    expectEquivalentToFreshLayout(width);
                for (auto width = narrowest; width < widest; width += 2.9f)
                    expectEquivalentToFreshLayout(width);
            }
        }
    }

    void testLayoutsAreReusedAcrossWidths()
    {
        beginTest("layouts are reused at widths that don't change where their lines break");

        jive::TextComponent text;
        text.setText("Wrapping text that spans a few lines");

        const auto* wrapped = &text.getTextLayout(100.0f);
        expectGreaterThan(wrapped->getNumLines(), 1);
        expect(&text.getTextLayout((wrapped->getWidth() + 100.0f) / 2.0f) == wrapped);

        const auto* unwrapped = &text.getTextLayout(10000.0f);
        expectEquals(unwrapped->getNumLines(), 1);
        expect(&text.getTextLayout(unwrapped->getWidth() + 1.0f) == unwrapped);
    }

    void testLayoutsFollowChanges()
    {
        beginTest("layouts follow changes to the text, word-wrap, and styles");

        jive::TextComponent text;
        jive::LookAndFeel lookAndFeel{ text };

        text.setText("One two three four five six");
        expectGreaterThan(text.getTextLayout(60.0f).getNumLines(), 1);

        text.setWordWrap(juce::AttributedString::WordWrap::none);
        expectEquals(text.getTextLayout(60.0f).getNumLines(), 1);

        text.setText("One");
        expect(text.getTextLayout(60.0f).getLine(0).stringRange == juce::Range{ 0, 3 });

        text.append(juce::AttributedString{ " more" });
        expect(text.getTextLayout(60.0f).getLine(0).stringRange == juce::Range{ 0, 8 });

        const auto heightBefore = text.getTextLayout(60.0f).getHeight();
        lookAndFeel.addStyles(jive::StyleSelector::makeInlineStyleSelector(text),
                              jive::Styles{}.withFontPointSize(40.0f));
        text.sendLookAndFeelChange();
        expectGreaterThan(text.getTextLayout(60.0f).getHeight(), heightBefore * 2.0f);
    }

    void testDefaultForeground()
    {
        beginTest("defaults to JUCE's text colour when nothing styles it");

        jive::TextComponent text;
        jive::LookAndFeel lookAndFeel{ text };
        lookAndFeel.applyTheme(jive::Theme::juce);
        text.setText("Text");

        expect(text.getAttributedString().getAttribute(0).colour == jive::detail::getV4Colour(jive::detail::V4Colour::defaultText));
    }
};

static TextComponentUnitTest textComponentUnitTest;
#endif
