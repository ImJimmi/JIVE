#include <jive_layouts/jive_layouts.h>

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    juce::BorderSize<int> toNearestInt(juce::BorderSize<float> border)
    {
        return juce::BorderSize<int>{
            juce::roundToInt(border.getTop()),
            juce::roundToInt(border.getLeft()),
            juce::roundToInt(border.getBottom()),
            juce::roundToInt(border.getRight()),
        };
    }

    Label::Label(std::unique_ptr<GuiItem> itemToDecorate)
        : GuiItemDecorator{ std::move(itemToDecorate) }
        , text{ tree, "text" }
        , typefaceName{ tree, "typeface-name" }
        , fontWeight{ tree, "font-weight", "Regular" }
        , fontHeight{ tree, "font-height", 12.f }
        , fontStyle{ tree, "font-style", "plain" }
        , justification{ tree, "justification", juce::Justification::centredLeft }
        , border{ tree, "border-width" }
    {
        text.onValueChange = [this]() {
            getLabel().setText(text, juce::sendNotification);
        };
        getLabel().setText(text, juce::sendNotification);

        justification.onValueChange = [this]() {
            getLabel().setJustificationType(justification);
        };
        getLabel().setJustificationType(justification);

        typefaceName.onValueChange = [this]() {
            fontChanged();
        };
        fontWeight.onValueChange = [this]() {
            fontChanged();
        };
        fontHeight.onValueChange = [this]() {
            fontChanged();
        };
        fontStyle.onValueChange = [this]() {
            fontChanged();
        };
        fontChanged();

        border.onValueChange = [this]() {
            getLabel().setBorderSize(toNearestInt(border));
        };
        getLabel().setBorderSize(toNearestInt(border));
    }

    //==================================================================================================================
    bool Label::isContainer() const
    {
        return false;
    }

    float Label::getWidth() const
    {
        if (hasAutoWidth())
        {
            const auto textWidth = getFont().getStringWidthFloat(text);
            const auto borderWidth = getBoxModel().getBorder().getLeftAndRight();

            return textWidth + borderWidth;
        }

        return GuiItemDecorator::getWidth();
    }

    float Label::getHeight() const
    {
        if (hasAutoHeight())
        {
            const auto textHeight = getFont().getHeight();
            const auto borderHeight = getLabel().getBorderSize().getTopAndBottom();

            return textHeight + borderHeight;
        }

        return GuiItemDecorator::getHeight();
    }

    //==================================================================================================================
    juce::Label& Label::getLabel()
    {
        auto* label = dynamic_cast<juce::Label*>(&getComponent());
        jassert(label != nullptr);

        return *label;
    }

    const juce::Label& Label::getLabel() const
    {
        const auto* label = dynamic_cast<const juce::Label*>(&getComponent());
        jassert(label != nullptr);

        return *label;
    }

    juce::Font Label::getFont() const
    {
        return getLabel().getFont();
    }

    juce::String Label::getText() const
    {
        return text;
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

    void Label::fontChanged()
    {
        juce::Font font;

        if (typefaceName.get().isNotEmpty())
            font.setTypefaceName(typefaceName);

        font.setTypefaceStyle(fontWeight);
        font.setStyleFlags(parseFontStyleFlags(fontStyle));
        font = font.withPointHeight(fontHeight);

        getLabel().setFont(font);
    }
} // namespace jive

//======================================================================================================================
#if JIVE_UNIT_TESTS
class LabelUnitTest : public juce::UnitTest
{
public:
    LabelUnitTest()
        : juce::UnitTest{ "jive::Label", "jive" }
    {
    }

    void runTest() final
    {
        testText();
        testFont();
        testJustification();
        testBorder();
        testAutoSize();
    }

private:
    std::unique_ptr<jive::Label> createLabel(juce::ValueTree tree)
    {
        return std::make_unique<jive::Label>(std::make_unique<jive::GuiItem>(std::make_unique<juce::Label>(), tree));
    }

    void testText()
    {
        beginTest("text");

        {
            juce::ValueTree tree{ "Label" };
            auto label = createLabel(tree);

            expect(label->getText().isEmpty());
            expect(label->getLabel().getText() == label->getText());

            tree.setProperty("text", "Some Text", nullptr);

            expect(label->getText() == "Some Text");
            expect(label->getLabel().getText() == label->getText());
        }
        {
            juce::ValueTree tree{ "Label", { { "text", "Not empty" } } };
            auto label = createLabel(tree);

            expect(label->getText() == "Not empty");
            expect(label->getLabel().getText() == label->getText());
        }
    }

    void testFont()
    {
        beginTest("font");

        {
            juce::ValueTree tree{ "Label" };
            auto label = createLabel(tree);

            expect(label->getFont() == juce::Font{ 0.f }.withPointHeight(12.f));
            expect(label->getLabel().getFont() == label->getFont());

            tree.setProperty("typeface-name", "Helvetica", nullptr);

            expect(label->getFont().getTypefaceName() == "Helvetica");
            expect(label->getLabel().getFont() == label->getFont());

            tree.setProperty("font-weight", "Thin", nullptr);

            expect(label->getFont().getTypefaceStyle() == "Thin");
            expect(label->getLabel().getFont() == label->getFont());

            tree.setProperty("font-height", 35.f, nullptr);

            expect(label->getFont().getHeight() == juce::Font{ "Helvetica", 0.f, 0 }.withPointHeight(35.f).getHeight());
            expect(label->getLabel().getFont() == label->getFont());

            tree.setProperty("font-style", "bold italic underlined", nullptr);

            expect(label->getFont().isBold());
            expect(label->getFont().isItalic());
            expect(label->getFont().isUnderlined());
            expect(label->getLabel().getFont() == label->getFont());
        }
        {
            juce::ValueTree tree{
                "Label",
                { { "typeface-name", "Georgia" },
                  { "font-height", 23.4 },
                  { "font-style", "bold" } }
            };
            auto label = createLabel(tree);

            expect(label->getFont() == juce::Font{ "Georgia", 0.f, 0 }.withPointHeight(23.4).withStyle(juce::Font::bold));
        }
    }

    void testJustification()
    {
        beginTest("justification");

        {
            juce::ValueTree tree{ "Label" };
            auto label = createLabel(tree);

            expect(label->getLabel().getJustificationType() == juce::Justification::centredLeft);

            tree.setProperty("justification", "top-left", nullptr);

            expect(label->getLabel().getJustificationType() == juce::Justification::topLeft);
        }
        {
            juce::ValueTree tree{ "Label", { { "justification", "bottom-right" } } };
            auto label = createLabel(tree);

            expect(label->getLabel().getJustificationType() == juce::Justification::bottomRight);
        }
    }

    void testBorder()
    {
        beginTest("border");

        juce::ValueTree tree{ "Label" };
        auto label = createLabel(tree);

        expect(label->getLabel().getBorderSize().getTop() == 0);
        expect(label->getLabel().getBorderSize().getRight() == 0);
        expect(label->getLabel().getBorderSize().getBottom() == 0);
        expect(label->getLabel().getBorderSize().getLeft() == 0);

        tree.setProperty("border-width", "5 10 20 40", nullptr);

        expect(label->getLabel().getBorderSize().getTop() == 5);
        expect(label->getLabel().getBorderSize().getRight() == 10);
        expect(label->getLabel().getBorderSize().getBottom() == 20);
        expect(label->getLabel().getBorderSize().getLeft() == 40);
    }

    void testAutoSize()
    {
        beginTest("auto size");

        juce::ValueTree tree{
            "Label",
            { { "text", "Some text" },
              { "border-width", 30 },
              { "padding", 5 } }
        };
        auto label = createLabel(tree);

        const auto boxModel = label->getBoxModel();
        const auto borderWidth = boxModel.getBorder().getLeftAndRight();
        const auto textWidth = label->getFont().getStringWidthFloat(label->getText());
        const auto expectedWidth = borderWidth + textWidth;
        expect(label->getWidth() == expectedWidth);

        const auto borderHeight = boxModel.getBorder().getTopAndBottom();
        const auto textHeight = label->getFont().getHeight();
        const auto expectedHeight = borderHeight + textHeight;
        expect(label->getHeight() == expectedHeight);
    }
};

static LabelUnitTest labelUnitTest;
#endif
