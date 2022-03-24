#include <jive_layouts/jive_layouts.h>

//======================================================================================================================
namespace jive
{
    TextWidget::TextWidget(juce::ValueTree tree)
        : widgetTree{ tree }
        , text{ tree, "text" }
        , typefaceName{ tree, "typeface-name" }
        , fontWeight{ tree, "font-weight", "Regular" }
        , fontHeight{ tree, "font-height", 12.f }
        , fontStyle{ tree, "font-style", "plain" }
        , justification{ tree, "justification", juce::Justification::centredLeft }
    {
        text.onValueChange = [this]() {
            if (onTextChanged != nullptr)
                onTextChanged();
        };

        typefaceName.onValueChange = [this]() {
            updateFont();

            if (onFontChanged != nullptr)
                onFontChanged();
        };
        fontWeight.onValueChange = [this]() {
            updateFont();

            if (onFontChanged != nullptr)
                onFontChanged();
        };
        fontHeight.onValueChange = [this]() {
            updateFont();

            if (onFontChanged != nullptr)
                onFontChanged();
        };
        fontStyle.onValueChange = [this]() {
            updateFont();

            if (onFontChanged != nullptr)
                onFontChanged();
        };
        updateFont();

        justification.onValueChange = [this]() {
            if (onJustificationChanged != nullptr)
                onJustificationChanged();
        };
    }

    //==================================================================================================================
    juce::String TextWidget::getText() const
    {
        return text;
    }

    juce::Font TextWidget::getFont() const
    {
        return font;
    }

    juce::Justification TextWidget::getTextJustification() const
    {
        return justification;
    }

    //==================================================================================================================
    int parseFontStyleFlags(const juce::String& styleString)
    {
        int flags = juce::Font::plain;
        const auto tokens = juce::StringArray::fromTokens(styleString, false);

        if (tokens.contains("bold"))
            flags += juce::Font::bold;
        if (tokens.contains("italic"))
            flags += juce::Font::italic;
        if (tokens.contains("underlined"))
            flags += juce::Font::underlined;

        return flags;
    }

    void TextWidget::updateFont()
    {
        if (typefaceName.get().isNotEmpty())
            font.setTypefaceName(typefaceName);

        font.setTypefaceStyle(fontWeight);
        font.setStyleFlags(parseFontStyleFlags(fontStyle));
        font = font.withPointHeight(fontHeight);
    }
} // namespace jive

//======================================================================================================================
#if JIVE_UNIT_TESTS
class TextWidgetUnitTest : public juce::UnitTest
{
public:
    TextWidgetUnitTest()
        : juce::UnitTest{ "jive::TextWidget", "jive" }
    {
    }

    void runTest() final
    {
        testText();
        testFont();
        testJustification();
    }

private:
    void testText()
    {
        beginTest("text");

        {
            juce::ValueTree tree{ "Label" };
            jive::TextWidget widget{ tree };

            expect(widget.getText().isEmpty());

            tree.setProperty("text", "Some Text", nullptr);

            expect(widget.getText() == "Some Text");
        }
        {
            juce::ValueTree tree{ "Label", { { "text", "Not empty" } } };
            jive::TextWidget widget{ tree };

            expect(widget.getText() == "Not empty");
        }
    }

    void testFont()
    {
        beginTest("font");

        {
            juce::ValueTree tree{ "Label" };
            jive::TextWidget widget{ tree };

            expect(widget.getFont() == juce::Font{ 0.f }.withPointHeight(12.f));

            tree.setProperty("typeface-name", "Helvetica", nullptr);

            expect(widget.getFont().getTypefaceName() == "Helvetica");

            tree.setProperty("font-weight", "Thin", nullptr);

            expect(widget.getFont().getTypefaceStyle() == "Thin");

            tree.setProperty("font-height", 35.f, nullptr);

            expect(widget.getFont().getHeight() == juce::Font{ "Helvetica", 0.f, 0 }.withPointHeight(35.f).getHeight());

            tree.setProperty("font-style", "bold italic underlined", nullptr);

            expect(widget.getFont().isBold());
            expect(widget.getFont().isItalic());
            expect(widget.getFont().isUnderlined());
        }
        {
            juce::ValueTree tree{
                "Label",
                { { "typeface-name", "Georgia" },
                  { "font-height", 23.4 },
                  { "font-style", "bold" } }
            };
            jive::TextWidget widget{ tree };

            expect(widget.getFont() == juce::Font{ "Georgia", 0.f, 0 }.withPointHeight(23.4).withStyle(juce::Font::bold));
        }
    }

    void testJustification()
    {
        beginTest("justification");

        {
            juce::ValueTree tree{ "Label" };
            jive::TextWidget widget{ tree };

            expect(widget.getTextJustification() == juce::Justification::centredLeft);

            tree.setProperty("justification", "top-left", nullptr);

            expect(widget.getTextJustification() == juce::Justification::topLeft);
        }
        {
            juce::ValueTree tree{ "Label", { { "justification", "bottom-right" } } };
            jive::TextWidget widget{ tree };

            expect(widget.getTextJustification() == juce::Justification::bottomRight);
        }
    }
};

static TextWidgetUnitTest textWidgetUnitTest;
#endif
