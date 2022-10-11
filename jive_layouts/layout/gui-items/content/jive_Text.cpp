#include <jive_layouts/jive_layouts.h>

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    Text::Text(std::unique_ptr<GuiItem> itemToDecorate)
        : GuiItemDecorator{ std::move(itemToDecorate) }
        , text{ state, "text" }
        , typefaceName{ state, "typeface-name" }
        , fontWeight{ state, "font-weight", "Regular" }
        , fontHeight{ state, "font-height", 12.f }
        , fontStyle{ state, "font-style" }
        , kerning{ state, "kerning" }
        , horizontalScale{ state, "horizontal-scale", 1.0f }
        , lineSpacing{ state, "line-spacing" }
        , justification{ state, "justification", juce::Justification::centredLeft }
        , wordWrap{ state, "word-wrap", juce::AttributedString::WordWrap::byWord }
        , direction{ state, "direction", juce::AttributedString::ReadingDirection::natural }
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
            updateFont();
        };
        fontWeight.onValueChange = [this]() {
            updateFont();
        };
        fontHeight.onValueChange = [this]() {
            updateFont();
        };
        fontStyle.onValueChange = [this]() {
            updateFont();
        };
        kerning.onValueChange = [this]() {
            updateFont();
        };
        horizontalScale.onValueChange = [this]() {
            updateFont();
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

    void Text::componentParentHierarchyChanged(juce::Component& componentWhoseParentHierarchyChanged)
    {
        if (&componentWhoseParentHierarchyChanged != component.get())
            return;

        if (auto* parentItem = getParent())
            parentItem->informContentChanged();
    }

    //==================================================================================================================
    juce::TextLayout Text::buildTextLayout() const
    {
        auto maxWidth = std::numeric_limits<float>::max();

        if (auto* parentItem = getParent())
        {
            if (!parentItem->hasAutoWidth())
                maxWidth = parentItem->boxModel.getContentBounds().getWidth();
        }

        juce::TextLayout layout;
        layout.createLayout(getTextComponent().getAttributedString(), maxWidth);

        return layout;
    }

    int getFontStyleFlags(const juce::String& styleString)
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

    void Text::updateFont()
    {
        juce::Font font;

        if (typefaceName.get().isNotEmpty())
            font.setTypefaceName(typefaceName);

        font.setTypefaceStyle(fontWeight);
        font.setStyleFlags(getFontStyleFlags(fontStyle));
        font.setExtraKerningFactor(kerning);
        font.setHorizontalScale(horizontalScale);
        font = font.withPointHeight(fontHeight);

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
            Text childItem{ std::make_unique<GuiItem>(std::make_unique<TextComponent>(),
                                                      childState) };
            getTextComponent().append(childItem.getTextComponent().getAttributedString());
        }

        getTextComponent().setWordWrap(wordWrap);

        updateFont();

        if (auto* parentItem = getParent())
        {
            parentItem->informContentChanged();

            if (!parentItem->isContainer())
                getTextComponent().setAccessible(false);
        }

        updateExplicitSize();
    }

    void Text::updateExplicitSize()
    {
        auto textLayout = buildTextLayout();

        if (hasAutoWidth())
            boxModel.setWidth(juce::jmax(boxModel.getWidth(), std::ceil(textLayout.getWidth())));

        if (hasAutoHeight())
            boxModel.setHeight(juce::jmax(boxModel.getHeight(), std::ceil(textLayout.getHeight())));
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
        testContentChanged();
    }

private:
    std::unique_ptr<jive::Text> createText(juce::ValueTree tree)
    {
        jive::Interpreter interpreter;
        return std::unique_ptr<jive::Text>(dynamic_cast<jive::Text*>(interpreter.interpret(tree).release()));
    }

    void testText()
    {
        beginTest("text");

        {
            juce::ValueTree tree{ "Text" };
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
            juce::ValueTree tree{ "Text" };
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
                {},
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
                    // { "width", 10000.0f },
                    // { "height", 10000.0f },
                },
                {
                    textTree,
                },
            };
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(tree);
            const auto font = dynamic_cast<jive::TextComponent&>(*item->getChild(0).getComponent())
                                  .getAttributedString()
                                  .getAttribute(0)
                                  .font;

            expectEquals(item->getChild(0).boxModel.getWidth(),
                         std::ceil(font.getStringWidthFloat("This side up.")));
            expectEquals(item->getChild(0).boxModel.getHeight(), std::ceil(font.getHeight()));

            textTree.setProperty("text", "This one spans\nmultiple lines.", nullptr);
            expectEquals(item->getChild(0).boxModel.getWidth(),
                         std::ceil(std::max({ font.getStringWidthFloat("This one spans"),
                                              font.getStringWidthFloat("multiple lines.") })));
            expectEquals(item->getChild(0).boxModel.getHeight(), std::ceil(font.getHeight()) * 2.0f);

            textTree.setProperty("width", 100.0f, nullptr);
            textTree.setProperty("text",
                                 "A very very very very very very very very very "
                                 "very very long line.",
                                 nullptr);
            expectGreaterThan(item->getChild(0).boxModel.getHeight(), font.getHeight());
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
            auto& item = dynamic_cast<jive::Text&>(parent->getChild(0));
            expectLessOrEqual(item.boxModel.getWidth(), 40.0f);
            expectGreaterThan(item.boxModel.getWidth(), 0.0f);
        }
    }

    void testContentChanged()
    {
        beginTest("parent-content-changed");

        class SpyGuiItem : public jive::GuiItem
        {
        public:
            using jive::GuiItem::GuiItem;

            std::function<void()> onContentChanged = nullptr;

        protected:
            void contentChanged() final
            {
                if (onContentChanged != nullptr)
                    onContentChanged();
            }
        };

        juce::ValueTree tree{
            "Spy",
            {},
            {
                juce::ValueTree{
                    "Text",
                    {
                        { "text", "Some text..." },
                    },
                },
            },
        };
        SpyGuiItem item{
            std::make_unique<juce::Component>(),
            tree,
        };

        auto parentContentChangedCalled = false;
        item.onContentChanged = [&parentContentChangedCalled]() {
            parentContentChangedCalled = true;
        };

        item.addChild(std::make_unique<jive::Text>(std::make_unique<jive::GuiItem>(std::make_unique<jive::TextComponent>(),
                                                                                   tree.getChild(0),
                                                                                   &item)));
        expect(parentContentChangedCalled);
        parentContentChangedCalled = false;

        tree.getChild(0).setProperty("text", "Some more text...", nullptr);
        expect(parentContentChangedCalled);
    }
};

static TextTest textTest;
#endif
