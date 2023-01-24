#include <jive_layouts/jive_layouts.h>

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    Text::Text(std::unique_ptr<GuiItem> itemToDecorate)
        : GuiItemDecorator{ std::move(itemToDecorate) }
        , text{ state, "text" }
        , typefaceName{ state, "typeface-name" }
        , fontWeight{ state, "font-weight" }
        , fontHeight{ state, "font-height" }
        , fontStyle{ state, "font-style" }
        , kerning{ state, "kerning" }
        , horizontalScale{ state, "horizontal-scale" }
        , lineSpacing{ state, "line-spacing" }
        , justification{ state, "justification", juce::Justification::centredLeft }
        , wordWrap{ state, "word-wrap", juce::AttributedString::WordWrap::byWord }
        , direction{ state, "direction", juce::AttributedString::ReadingDirection::natural }
        , idealWidth{ state, "ideal-width" }
        , idealHeight{ state, "ideal-height" }
    {
        text.onValueChange = [this]() {
            updateTextComponent();
        };
        justification.onValueChange = [this]() {
            updateTextComponent();
        };
        wordWrap.onValueChange = [this]() {
            updateTextComponent();
        };
        direction.onValueChange = [this]() {
            updateTextComponent();
        };
        lineSpacing.onValueChange = [this]() {
            updateTextComponent();
        };
        typefaceName.onValueChange = [this]() {
            updateTextComponent();
        };
        fontWeight.onValueChange = [this]() {
            updateTextComponent();
        };
        fontHeight.onValueChange = [this]() {
            updateTextComponent();
        };
        fontStyle.onValueChange = [this]() {
            updateTextComponent();
        };
        kerning.onValueChange = [this]() {
            updateTextComponent();
        };
        horizontalScale.onValueChange = [this]() {
            updateTextComponent();
        };

        updateTextComponent();
    }

    //==================================================================================================================
    void Text::addChild(std::unique_ptr<GuiItem> /* child */)
    {
    }

    bool Text::isContainer() const
    {
        return false;
    }

    bool Text::isContent() const
    {
        return true;
    }

    //==================================================================================================================
    TextComponent& Text::getTextComponent()
    {
        return dynamic_cast<TextComponent&>(*component);
    }

    const TextComponent& Text::getTextComponent() const
    {
        return dynamic_cast<const TextComponent&>(*component);
    }

    //==================================================================================================================
    void Text::valueTreeChildAdded(juce::ValueTree& /* parentTree */, juce::ValueTree& /* child */)
    {
        updateTextComponent();
    }

    //==================================================================================================================
    juce::TextLayout Text::buildTextLayout(float maxWidth) const
    {
        if (maxWidth < 0.0f)
        {
            if (auto* parentItem = getParent())
            {
                if (!parentItem->boxModel.hasAutoWidth())
                    maxWidth = parentItem->boxModel.getContentBounds().getWidth();
            }
        }

        juce::TextLayout layout;
        layout.createLayout(getTextComponent().getAttributedString(), maxWidth);

        return layout;
    }

    void Text::updateFont()
    {
        juce::Font font;

        if (typefaceName.get().isNotEmpty())
            font.setTypefaceName(typefaceName);

        font.setTypefaceStyle(fontWeight.getOr("Regular"));
        font.setStyleFlags(parseFontStyleFlags(fontStyle));
        font.setExtraKerningFactor(kerning);
        font.setHorizontalScale(horizontalScale.getOr(1.0f));
        font = font.withPointHeight(fontHeight.getOr(12.0f));

        getTextComponent().setFont(font);
    }

    void Text::updateTextComponent()
    {
        getTextComponent().clearAttributes();

        getTextComponent().setText(text);
        getTextComponent().setJustification(justification);
        getTextComponent().setDirection(direction);
        getTextComponent().setLineSpacing(lineSpacing);

        for (auto childState : state)
        {
            Text childItem
            {
                std::make_unique<GuiItem>(std::make_unique<TextComponent>(),
                                          childState,
#if JIVE_GUI_ITEMS_HAVE_STYLE_SHEETS
                                          nullptr,
#endif
                                          this)
            };
            getTextComponent().append(childItem.getTextComponent().getAttributedString());
        }

        getTextComponent().setWordWrap(wordWrap);

        updateFont();

        const auto layout = buildTextLayout(std::numeric_limits<float>::max());
        idealWidth = std::ceil(layout.getWidth());
        // idealHeight = std::ceil(layout.getHeight());
        state.setProperty(
            "ideal-height",
            juce::var{ [this](const juce::var::NativeFunctionArgs& args) {
                const auto layout = buildTextLayout(args.arguments[0]);
                return std::ceil(layout.getHeight());
            } },
            nullptr);

        if (auto* parentItem = getParent())
        {
            if (!parentItem->isContainer())
                getTextComponent().setAccessible(false);

            parentItem->layOutChildren();
        }
    }

    //==================================================================================================================
    const Text* findFirstTextContent(const GuiItem& item)
    {
        if (auto* text = dynamic_cast<const Text*>(&item))
            return text;

        for (const auto& child : item)
        {
            auto* text = findFirstTextContent(child);

            if (text != nullptr)
                return text;
        }

        return nullptr;
    }
} // namespace jive

//======================================================================================================================
#if JIVE_UNIT_TESTS
class TextTest : public juce::UnitTest
{
public:
    TextTest()
        : juce::UnitTest{ "jive::Text", "jive" }
    {
    }

    void runTest() final
    {
        testText();
        testFont();
        testJustification();
        testWordWrap();
        testReadingDirection();
        testLineSpacing();
        testNested();
        testAutoSize();
    }

private:
    std::unique_ptr<jive::Text> createText(juce::ValueTree tree)
    {
        jive::Interpreter interpreter;
        return std::unique_ptr<jive::Text>{
            dynamic_cast<jive::Text*>(interpreter.interpret(tree).release())
        };
    }

    void testText()
    {
        beginTest("text");

        {
            juce::ValueTree tree{
                "Text",
                {
                    { "width", 222 },
                    { "height", 333 },
                },
            };
            auto item = createText(tree);
            expectEquals(item->getTextComponent().getAttributedString().getNumAttributes(), 0);

            tree.setProperty("text", "Some text...", nullptr);
            expectEquals(item->getTextComponent().getAttributedString().getNumAttributes(), 1);
            expectEquals<juce::String>(item->getTextComponent().getAttributedString().getText(), "Some text...");
        }
        {
            juce::ValueTree tree{
                "Text",
                {
                    { "width", 222 },
                    { "height", 333 },
                    { "text", "Not empty" },
                },
            };
            auto item = createText(tree);
            expectEquals(item->getTextComponent().getAttributedString().getNumAttributes(), 1);
            expectEquals<juce::String>(item->getTextComponent().getAttributedString().getText(), "Not empty");
        }
    }

    void testFont()
    {
        beginTest("font");

        {
            juce::ValueTree tree{
                "Text",
                {
                    { "width", 222 },
                    { "height", 333 },
                    { "text", "The font of all knowledge if Comic Sans" },
                },
            };
            auto item = createText(tree);
            expectEquals(item->getTextComponent().getAttributedString().getNumAttributes(), 1);
            expect(item->getTextComponent()
                       .getAttributedString()
                       .getAttribute(0)
                       .font
                   == juce::Font{ 0.f }.withPointHeight(12.f));

            tree.setProperty("typeface-name", "Helvetica", nullptr);
            expectEquals(item->getTextComponent().getAttributedString().getNumAttributes(), 1);
            expectEquals<juce::String>(item->getTextComponent()
                                           .getAttributedString()
                                           .getAttribute(0)
                                           .font
                                           .getTypefaceName(),
                                       "Helvetica");

            tree.setProperty("font-weight", "Thin", nullptr);
            expectEquals(item->getTextComponent().getAttributedString().getNumAttributes(), 1);
            expectEquals<juce::String>(item->getTextComponent()
                                           .getAttributedString()
                                           .getAttribute(0)
                                           .font
                                           .getTypefaceStyle(),
                                       "Thin");

            tree.setProperty("font-height", 35.f, nullptr);
            expectEquals(item->getTextComponent().getAttributedString().getNumAttributes(), 1);
            expectEquals(item->getTextComponent()
                             .getAttributedString()
                             .getAttribute(0)
                             .font
                             .getHeight(),
                         juce::Font{ "Helvetica", 0.f, 0 }
                             .withPointHeight(35.f)
                             .getHeight());

            tree.setProperty("font-style", "bold italic underlined", nullptr);
            expectEquals(item->getTextComponent().getAttributedString().getNumAttributes(), 1);
            expect(item->getTextComponent()
                       .getAttributedString()
                       .getAttribute(0)
                       .font
                       .isBold());
            expect(item->getTextComponent()
                       .getAttributedString()
                       .getAttribute(0)
                       .font
                       .isItalic());
            expect(item->getTextComponent()
                       .getAttributedString()
                       .getAttribute(0)
                       .font
                       .isUnderlined());

            tree.setProperty("kerning", 3.14f, nullptr);
            expectEquals(item->getTextComponent().getAttributedString().getNumAttributes(), 1);
            expectEquals(item->getTextComponent()
                             .getAttributedString()
                             .getAttribute(0)
                             .font
                             .getExtraKerningFactor(),
                         3.14f);

            tree.setProperty("horizontal-scale", 1.414f, nullptr);
            expectEquals(item->getTextComponent().getAttributedString().getNumAttributes(), 1);
            expectEquals(item->getTextComponent()
                             .getAttributedString()
                             .getAttribute(0)
                             .font
                             .getHorizontalScale(),
                         1.414f);
        }
    }

    void testJustification()
    {
        beginTest("justification");

        {
            juce::ValueTree tree{
                "Text",
                {
                    { "width", 222 },
                    { "height", 333 },
                    { "text", "Do not read." },
                },
            };
            auto item = createText(tree);
            expectEquals(item->getTextComponent().getAttributedString().getNumAttributes(), 1);
            expect(item->getTextComponent().getAttributedString().getJustification()
                   == juce::Justification::centredLeft);

            tree.setProperty("justification", "top-left", nullptr);
            expect(item->getTextComponent().getAttributedString().getJustification()
                   == juce::Justification::topLeft);
        }
        {
            juce::ValueTree tree{
                "Text",
                {
                    { "width", 222 },
                    { "height", 333 },
                    { "justification", "bottom-right" },
                    { "text", "Do read." },
                },
            };
            auto item = createText(tree);
            expectEquals(item->getTextComponent().getAttributedString().getNumAttributes(), 1);
            expect(item->getTextComponent().getAttributedString().getJustification()
                   == juce::Justification::bottomRight);
        }
    }

    void testWordWrap()
    {
        beginTest("word-wrap");

        {
            juce::ValueTree tree{
                "Text",
                {
                    { "width", 222 },
                    { "height", 333 },
                    { "text", "All that glitters is gold." },
                },
            };
            auto item = createText(tree);
            expectEquals(item->getTextComponent().getAttributedString().getWordWrap(),
                         juce::AttributedString::WordWrap::byWord);

            tree.setProperty("word-wrap", "none", nullptr);
            expectEquals(item->getTextComponent().getAttributedString().getWordWrap(),
                         juce::AttributedString::WordWrap::none);
        }
        {
            juce::ValueTree tree{
                "Text",
                {
                    { "width", 222 },
                    { "height", 333 },
                    { "text", "Please stand by." },
                    { "word-wrap", "none" },
                },
            };
            auto item = createText(tree);
            expectEquals(item->getTextComponent().getAttributedString().getWordWrap(),
                         juce::AttributedString::WordWrap::none);
        }
    }

    void testReadingDirection()
    {
        beginTest("direction");

        {
            juce::ValueTree tree{
                "Text",
                {
                    { "width", 222 },
                    { "height", 333 },
                    { "text", "One step at a time." },
                },
            };
            auto item = createText(tree);
            expectEquals(item->getTextComponent().getAttributedString().getReadingDirection(),
                         juce::AttributedString::ReadingDirection::natural);

            tree.setProperty("direction", "left-to-right", nullptr);
            expectEquals(item->getTextComponent().getAttributedString().getReadingDirection(),
                         juce::AttributedString::ReadingDirection::leftToRight);
        }
        {
            juce::ValueTree tree{
                "Text",
                {
                    { "width", 222 },
                    { "height", 333 },
                    { "text", "Chicken tendies." },
                    { "direction", "right-to-left" },
                },
            };
            auto item = createText(tree);
            expectEquals(item->getTextComponent().getAttributedString().getReadingDirection(),
                         juce::AttributedString::ReadingDirection::rightToLeft);
        }
    }

    void testLineSpacing()
    {
        beginTest("line-spacing");

        {
            juce::ValueTree tree{
                "Text",
                {
                    { "width", 222 },
                    { "height", 333 },
                    { "text", "Bottle o' rum." },
                },
            };
            auto item = createText(tree);
            expectEquals(item->getTextComponent().getAttributedString().getLineSpacing(),
                         0.0f);

            tree.setProperty("line-spacing", 1.234f, nullptr);
            expectEquals(item->getTextComponent().getAttributedString().getLineSpacing(),
                         1.234f);
        }
        {
            juce::ValueTree tree{
                "Text",
                {
                    { "width", 222 },
                    { "height", 333 },
                    { "text", "First of all..." },
                    { "line-spacing", 4.389f },
                },
            };
            auto item = createText(tree);
            expectEquals(item->getTextComponent().getAttributedString().getLineSpacing(),
                         4.389f);
        }
    }

    void testNested()
    {
        beginTest("nested");

        {
            juce::ValueTree tree{
                "Text",
                {
                    { "width", 222 },
                    { "height", 333 },
                },
            };
            auto item = createText(tree);
            expectEquals(item->getTextComponent().getAttributedString().getNumAttributes(),
                         0);

            tree.appendChild(juce::ValueTree{
                                 "Text",
                                 {
                                     { "text", "Nested." },
                                 },
                             },
                             nullptr);
            expectEquals<juce::String>(item->getTextComponent().getAttributedString().getText(),
                                       "Nested.");
            expectEquals(item->getComponent()->getNumChildComponents(), 0);
            tree.appendChild(juce::ValueTree{
                                 "Text",
                                 {
                                     { "text", " Sentence two." },
                                 },
                             },
                             nullptr);
            expectEquals<juce::String>(item->getTextComponent().getAttributedString().getText(),
                                       "Nested. Sentence two.");
            expectEquals(item->getComponent()->getNumChildComponents(), 0);
        }
        {
            juce::ValueTree tree{
                "Text",
                {
                    { "width", 222 },
                    { "height", 333 },
                    { "text", "Setup... " },
                },
                {
                    juce::ValueTree{
                        "Text",
                        {
                            { "text", "Conflict... " },
                        },
                    },
                    juce::ValueTree{
                        "Text",
                        {
                            { "text", "Revolution!" },
                        },
                    },
                }
            };
            auto item = createText(tree);
            expectEquals<juce::String>(item->getTextComponent().getAttributedString().getText(),
                                       "Setup... Conflict... Revolution!");
        }
        {
            juce::ValueTree tree{
                "Component",
                {
                    { "width", 222 },
                    { "height", 333 },
                },
                {
                    juce::ValueTree{
                        "Text",
                        {
                            { "text", "123" },
                        },
                        {
                            juce::ValueTree{
                                "Text",
                                {
                                    { "text", "456" },
                                },
                            },
                        },
                    },
                },
            };
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(tree);
            expectEquals(item->getComponent()->getNumChildComponents(),
                         1);
            expectEquals(item->getChild(0).getComponent()->getNumChildComponents(),
                         0);
            expectEquals(item->getChild(0).getNumChildren(),
                         0);
        }
    }

    void testAutoSize()
    {
        beginTest("auto size");

        {
            juce::ValueTree textTree{
                "Text",
                {
                    { "text", "This side up." },
                },
            };
            juce::ValueTree tree{
                "Component",
                {
                    { "width", 222 },
                    { "height", 333 },
                    { "align-items", "flex-start" },
                },
                {
                    textTree,
                },
            };
            jive::Interpreter interpreter;
            auto parentItem = interpreter.interpret(tree);
            auto& item = dynamic_cast<jive::Text&>(parentItem->getChild(0));
            const auto font = dynamic_cast<jive::TextComponent&>(*item.getComponent())
                                  .getAttributedString()
                                  .getAttribute(0)
                                  .font;

            expectEquals(item.boxModel.getWidth(),
                         std::ceil(font.getStringWidthFloat("This side up.")));
            expectEquals(item.boxModel.getHeight(), std::ceil(font.getHeight()));

            textTree.setProperty("text", "This one spans\nmultiple lines.", nullptr);
            expectEquals(item.boxModel.getWidth(),
                         std::ceil(std::max({ font.getStringWidthFloat("This one spans"),
                                              font.getStringWidthFloat("multiple lines.") })));
            expectEquals(item.boxModel.getHeight(), std::ceil(font.getHeight()) * 2.0f);

            textTree.setProperty("width", 100.0f, nullptr);
            textTree.setProperty("text",
                                 "A very very very very very very very very very "
                                 "very very long line.",
                                 nullptr);
            expectGreaterThan(item.boxModel.getHeight(), font.getHeight());
        }
        {
            juce::ValueTree tree{
                "Component",
                {
                    { "width", 240.0f },
                    { "height", 1000.0f },
                    { "padding", 50.0f },
                    { "border-width", 50.0f },
                },
                {
                    juce::ValueTree{
                        "Text",
                        {
                            { "text", "One two three four five six" },
                        },
                    },
                },
            };
            jive::Interpreter interpreter;
            auto parent = interpreter.interpret(tree);
            auto& item = parent->getChild(0);
            expectLessOrEqual(item.boxModel.getWidth(), 40.0f);
            expectGreaterThan(item.boxModel.getWidth(), 0.0f);
        }
    }
};

static TextTest textTest;
#endif
