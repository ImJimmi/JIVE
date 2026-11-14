#include "jive_TextComponent.h"

#include <jive_style_sheets/jive_style_sheets.h>

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

            const auto direction = getReadingDirection(*this, styles);
            const auto defaultAlignment = direction == juce::AttributedString::ReadingDirection::rightToLeft
                                            ? juce::Justification::centredRight
                                            : juce::Justification::centredLeft;

            attributedString.setReadingDirection(direction);
            attributedString.setJustification(getTextAlignment(*this, styles, defaultAlignment));

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
    // Text that isn't wrapped is laid out the same at every width, except on
    // Windows with JUCE 7, so those layouts are only reused at the same width.
    [[nodiscard]] static auto canBeReusedAt(const juce::TextLayout& layout,
                                            float createdAtMaxWidth,
                                            float maxWidth,
                                            juce::AttributedString::WordWrap wordWrap)
    {
        if (juce::exactlyEqual(maxWidth, createdAtMaxWidth))
            return true;

        if (wordWrap == juce::AttributedString::WordWrap::none)
#if JUCE_MAJOR_VERSION < 8 && JUCE_WINDOWS
            return false;
#else
            return true;
#endif

        return layout.getWidth() <= maxWidth && maxWidth <= createdAtMaxWidth;
    }

    // JUCE only uses the reading direction for text without any strong
    // characters, so a mark is put at the front to make it the direction
    // JUCE detects.
    [[nodiscard]] static auto withDirectionMark(const juce::AttributedString& source)
    {
        using ReadingDirection = juce::AttributedString::ReadingDirection;

        if (source.getReadingDirection() == ReadingDirection::natural || source.getNumAttributes() == 0)
            return source;

        const auto mark = juce::String::charToString(source.getReadingDirection() == ReadingDirection::rightToLeft
                                                         ? static_cast<juce::juce_wchar>(0x200f)
                                                         : static_cast<juce::juce_wchar>(0x200e));
        const auto& first = source.getAttribute(0);

        juce::AttributedString result;
        result.setJustification(source.getJustification());
        result.setWordWrap(source.getWordWrap());
        result.setReadingDirection(source.getReadingDirection());
        result.setLineSpacing(source.getLineSpacing());
        result.append(mark, first.font, first.colour);

        for (auto i = 0; i < source.getNumAttributes(); i++)
        {
            const auto& attribute = source.getAttribute(i);
            result.append(source.getText().substring(attribute.range.getStart(), attribute.range.getEnd()),
                          attribute.font,
                          attribute.colour);
        }

        return result;
    }

#if JUCE_MAJOR_VERSION >= 8
    [[nodiscard]] static auto getHorizontalAlignment(const juce::AttributedString& source)
    {
        if (source.getJustification().testFlags(juce::Justification::horizontallyCentred))
            return 0.5f;

        if (source.getJustification().testFlags(juce::Justification::right))
            return 1.0f;

        return 0.0f;
    }

    // JUCE 8 shares a ligature's advance between the ligature's glyph and
    // placeholders for the other characters it replaces, and then leaves the
    // placeholders out of the layout. A run ending in a ligature loses the
    // rest of its width, so its line measures too narrow and gets clipped.
    static void restoreTrailingLigatureWidths(juce::TextLayout& layout, const juce::AttributedString& source)
    {
        const auto alignment = getHorizontalAlignment(source);
        auto changed = false;

        for (auto lineIndex = 0; lineIndex < layout.getNumLines(); lineIndex++)
        {
            auto& line = layout.getLine(lineIndex);
            const auto widthBefore = line.getLineBoundsX().getLength();

            for (auto* run : line.runs)
            {
                const auto runText = source
                                         .getText()
                                         .substring(run->stringRange.getStart(), run->stringRange.getEnd())
                                         .trimEnd();

                if (run->glyphs.isEmpty() || run->glyphs.size() >= runText.length())
                    continue;

                const auto byAnchor = [](const auto& a, const auto& b) {
                    return a.anchor.x < b.anchor.x;
                };
                const auto [first, last] = std::minmax_element(std::begin(run->glyphs), std::end(run->glyphs), byAnchor);
                const auto right = first->anchor.x + juce::GlyphArrangement::getStringWidth(run->font, runText);

                if (right > last->anchor.x + last->width)
                {
                    last->width = right - last->anchor.x;
                    changed = true;
                }
            }

            line.lineOrigin.x -= (line.getLineBoundsX().getLength() - widthBefore) * alignment;
        }

        if (changed)
            layout.recalculateSize();
    }
#endif

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
        const auto source = withDirectionMark(textLayoutsSource);
        created.layout.createLayout(source, maxWidth);
#if JUCE_MAJOR_VERSION >= 8
        restoreTrailingLigatureWidths(created.layout, source);
#endif

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
    #include <JiveTestFonts.h>

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
        testReadingDirection();
        testTrailingLigatures();
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

    [[nodiscard]] static auto getFullStopX(const juce::TextLayout& layout, const juce::Font& font)
    {
        juce::AttributedString fullStop;
        fullStop.append(".", font);

        juce::TextLayout fullStopLayout;
        fullStopLayout.createLayout(fullStop, 1000.0f);
        const auto fullStopGlyph = fullStopLayout.getLine(0).runs.getFirst()->glyphs.getReference(0).glyphCode;

        for (const auto* run : layout.getLine(0).runs)
        {
            for (const auto& glyph : run->glyphs)
            {
                if (glyph.glyphCode == fullStopGlyph)
                    return glyph.anchor.x;
            }
        }

        return std::numeric_limits<float>::quiet_NaN();
    }

    void testReadingDirection()
    {
        beginTest("the reading direction decides which end trailing punctuation sits");

        const auto arabic = juce::String{ juce::CharPointer_UTF8{ "\xd9\x85\xd8\xb1\xd8\xad\xd8\xa8\xd8\xa7." } };

        for (const auto direction : { juce::AttributedString::ReadingDirection::leftToRight,
                                      juce::AttributedString::ReadingDirection::rightToLeft })
        {
            jive::TextComponent text;
            jive::LookAndFeel lookAndFeel{ text };
            lookAndFeel.addStyles(jive::StyleSelector::makeInlineStyleSelector(text),
                                  jive::Styles{}.withReadingDirection(direction));
            text.sendLookAndFeelChange();
            text.setText(arabic);

            expect(text.getAttributedString().getReadingDirection() == direction);

            const auto& layout = text.getTextLayout(1000.0f);
            const auto fullStopX = getFullStopX(layout, text.getAttributedString().getAttribute(0).font);
            const auto width = layout.getLine(0).getLineBoundsX();

            if (direction == juce::AttributedString::ReadingDirection::leftToRight)
                expectGreaterThan(fullStopX, (width.getStart() + width.getEnd()) / 2.0f);
    #if JUCE_MAJOR_VERSION >= 8
            else
                expectLessThan(fullStopX, (width.getStart() + width.getEnd()) / 2.0f);
    #endif
        }
    }

    // Rubik replaces "ff" with a single ligature glyph.
    struct LigatureText
    {
        explicit LigatureText(const juce::String& string,
                              juce::Justification justification = juce::Justification::centredLeft)
        {
            lookAndFeel.addStyles(jive::StyleSelector::makeInlineStyleSelector(text),
                                  jive::Styles{}
                                      .withFontFamily("Rubik")
                                      .withFontPointSize(13.0f)
                                      .withTextAlignment(justification));
            text.sendLookAndFeelChange();
            text.setText(string);
        }

        [[nodiscard]] juce::Font getFont() const
        {
            return text.getAttributedString().getAttribute(0).font;
        }

        [[nodiscard]] float getNaturalWidth(const juce::String& string) const
        {
            return juce::GlyphArrangement::getStringWidth(getFont(), string);
        }

        const juce::Typeface::Ptr typeface = juce::Typeface::createSystemTypefaceFor(jive_test_fonts::RubikRegular_ttf,
                                                                                     static_cast<std::size_t>(jive_test_fonts::RubikRegular_ttfSize));
        jive::TextComponent text;
        jive::LookAndFeel lookAndFeel{ text };
    };

    [[nodiscard]] static int findRightmostInk(const juce::Image& image)
    {
        for (auto x = image.getWidth() - 1; x >= 0; x--)
        {
            for (auto y = 0; y < image.getHeight(); y++)
            {
                if (image.getPixelAt(x, y).getAlpha() > 0)
                    return x;
            }
        }

        return -1;
    }

    void testTrailingLigatures()
    {
        beginTest("ligatures / a line ending in a ligature is as wide as its text");
        {
            LigatureText ligature{ "Cutoff" };
            const auto naturalWidth = ligature.getNaturalWidth("Cutoff");

            const auto& layout = ligature.text.getTextLayout(65535.0f);
            expectEquals(layout.getNumLines(), 1);
            expectWithinAbsoluteError(layout.getWidth(), naturalWidth, 0.05f);
        }

        beginTest("ligatures / aligned lines ending in a ligature stay within the layout's width");
        for (const juce::Justification justification : {
                 juce::Justification::centredLeft,
                 juce::Justification::centred,
                 juce::Justification::centredRight,
             })
        {
            LigatureText ligature{ "Cutoff", justification };
            const auto width = std::ceil(ligature.getNaturalWidth("Cutoff"));
            const auto& line = ligature.text.getTextLayout(width).getLine(0);
            const auto bounds = line.getLineBoundsX();

            expectGreaterOrEqual(bounds.getStart(), -0.01f);
            expectLessOrEqual(bounds.getEnd(), width + 0.01f);
            expectWithinAbsoluteError(bounds.getLength(), ligature.getNaturalWidth("Cutoff"), 0.05f);
        }

        beginTest("ligatures / wrapped lines ending in a ligature are as wide as their text");
        {
            LigatureText ligature{ "off off off off" };
            const auto& layout = ligature.text.getTextLayout(std::ceil(ligature.getNaturalWidth("off off")) + 1.0f);
            expectGreaterThan(layout.getNumLines(), 1);

            for (auto i = 0; i < layout.getNumLines(); i++)
            {
                const auto& line = layout.getLine(i);
                const auto lineText = ligature.text.getAttributedString().getText().substring(line.stringRange.getStart(), line.stringRange.getEnd()).trim();
                expectWithinAbsoluteError(line.getLineBoundsX().getLength(), ligature.getNaturalWidth(lineText), 0.05f, lineText);
            }
        }

        beginTest("ligatures / text sized to its natural width draws every letter");
        {
            LigatureText ligature{ "Cutoff" };
            const auto& naturalLayout = ligature.text.getTextLayout(65535.0f);
            const auto width = static_cast<int>(std::ceil(naturalLayout.getWidth())) + 1;
            const auto height = static_cast<int>(std::ceil(naturalLayout.getHeight()));
            ligature.text.setSize(width, height);

            juce::Image reference{ juce::Image::ARGB, width + 20, height, true };
            {
                juce::Graphics g{ reference };
                g.setColour(juce::Colours::black);
                g.setFont(ligature.getFont());
                g.drawSingleLineText("Cutoff", 0, juce::roundToInt(ligature.getFont().getAscent()));
            }

            expectWithinAbsoluteError(findRightmostInk(paint(ligature.text)), findRightmostInk(reference), 1);
        }
    }
};

static TextComponentUnitTest textComponentUnitTest;
#endif
