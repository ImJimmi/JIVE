#include <jive_layouts/jive_layouts.h>

namespace jive
{
    Text::Text(std::unique_ptr<GuiItem> itemToDecorate)
        : GuiItemDecorator{ std::move(itemToDecorate) }
        , text{ state, "text" }
        , lineSpacing{ state, "line-spacing" }
        , justification{ state, "justification" }
        , wordWrap{ state, "word-wrap" }
        , direction{ state, "direction" }
        , idealWidth{ state, "ideal-width" }
        , idealHeight{ state, "ideal-height" }
    {
        const BoxModel::ScopedCallbackLock boxModelLock{ boxModel(*this) };

        if (!justification.exists())
            justification = juce::Justification::centredLeft;
        if (!wordWrap.exists())
            wordWrap = juce::AttributedString::WordWrap::byWord;
        if (!direction.exists())
            direction = juce::AttributedString::ReadingDirection::natural;

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

        state.setProperty("ideal-height",
                          juce::var{ [this](const juce::var::NativeFunctionArgs& args) {
                              const auto layout = buildTextLayout(args.arguments[0]);
                              return std::ceil(layout.getHeight());
                          } },
                          nullptr);

        updateTextComponent();
        getTextComponent().addListener(*this);
    }

    Text::~Text()
    {
        getTextComponent().removeListener(*this);
    }

    void Text::insertChild(std::unique_ptr<GuiItem> child, int index)
    {
        GuiItemDecorator::insertChild(std::move(child), index);
        updateTextComponent();
    }

    void Text::setChildren(std::vector<std::unique_ptr<GuiItem>>&& newChildren)
    {
        GuiItemDecorator::setChildren(std::move(newChildren));
        updateTextComponent();
    }

    bool Text::isContainer() const
    {
        return false;
    }

    bool Text::isContent() const
    {
        return true;
    }

    TextComponent& Text::getTextComponent()
    {
        return dynamic_cast<TextComponent&>(*getComponent());
    }

    const TextComponent& Text::getTextComponent() const
    {
        return dynamic_cast<const TextComponent&>(*getComponent());
    }

    void Text::textFontChanged(TextComponent&)
    {
        updateTextComponent();
    }

    juce::TextLayout Text::buildTextLayout(float maxWidth) const
    {
        for (auto* parentItem = getParent();
             maxWidth < 0.0f && parentItem != nullptr;
             parentItem = parentItem->getParent())
        {
            if (const auto& parentBoxModel = dynamic_cast<const GuiItemDecorator*>(getParent())->toType<CommonGuiItem>()->boxModel;
                !parentBoxModel.hasAutoWidth())
            {
                maxWidth = parentBoxModel.getContentBounds().getWidth();
            }
        }

        juce::TextLayout layout;
        layout.createLayout(getTextComponent().getAttributedString(), maxWidth);

        return layout;
    }

    void Text::updateTextComponent()
    {
        getTextComponent().setDirection(direction);
        getTextComponent().setJustification(justification);
        getTextComponent().setLineSpacing(lineSpacing);
        getTextComponent().setText(text);
        getTextComponent().setWordWrap(wordWrap);
        getTextComponent().clearAttributes();

        for (auto* child : getChildren())
        {
            if (const auto* nestedText = dynamic_cast<const GuiItemDecorator*>(child)
                                             ->toType<const Text>())
            {
                getTextComponent()
                    .append(nestedText
                                ->getTextComponent()
                                .getAttributedString());
            }
            else
            {
                jassertfalse;
            }
        }

        idealWidth = std::ceil(buildTextLayout(static_cast<float>(std::numeric_limits<juce::uint16>::max()))
                                   .getWidth());

        if (auto* parentItem = getParent())
        {
            if (!parentItem->isContainer())
                getTextComponent().setAccessible(false);
            else
                parentItem->state.setProperty("box-model-valid", false, nullptr);
        }
    }

    const Text* findFirstTextContent(const GuiItem& item)
    {
        if (auto* text = dynamic_cast<const Text*>(&item))
            return text;

        for (const auto* child : item.getChildren())
        {
            auto* text = findFirstTextContent(*child);

            if (text != nullptr)
                return text;
        }

        return nullptr;
    }
} // namespace jive

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
        testJustification();
        testWordWrap();
        testReadingDirection();
        testLineSpacing();
        testNested();
        testAutoSize();
    }

private:
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
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(tree);
            auto& text = *dynamic_cast<jive::GuiItemDecorator&>(*item)
                              .toType<jive::Text>();
            expectEquals(text.getTextComponent().getAttributedString().getNumAttributes(), 0);

            tree.setProperty("text", "Some text...", nullptr);
            expectEquals(text.getTextComponent().getAttributedString().getNumAttributes(), 1);
            expectEquals<juce::String>(text.getTextComponent().getAttributedString().getText(), "Some text...");
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
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(tree);
            auto& text = *dynamic_cast<jive::GuiItemDecorator&>(*item)
                              .toType<jive::Text>();
            expectEquals(text.getTextComponent().getAttributedString().getNumAttributes(), 1);
            expectEquals<juce::String>(text.getTextComponent().getAttributedString().getText(), "Not empty");
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
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(tree);
            auto& text = *dynamic_cast<jive::GuiItemDecorator&>(*item)
                              .toType<jive::Text>();
            expectEquals(text.getTextComponent().getAttributedString().getNumAttributes(), 1);
            expect(text.getTextComponent().getAttributedString().getJustification()
                   == juce::Justification::centredLeft);

            tree.setProperty("justification", "top-left", nullptr);
            expect(text.getTextComponent().getAttributedString().getJustification()
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
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(tree);
            auto& text = *dynamic_cast<jive::GuiItemDecorator&>(*item)
                              .toType<jive::Text>();
            expectEquals(text.getTextComponent().getAttributedString().getNumAttributes(), 1);
            expect(text.getTextComponent().getAttributedString().getJustification()
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
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(tree);
            auto& text = *dynamic_cast<jive::GuiItemDecorator&>(*item)
                              .toType<jive::Text>();
            expectEquals(text.getTextComponent().getAttributedString().getWordWrap(),
                         juce::AttributedString::WordWrap::byWord);

            tree.setProperty("word-wrap", "none", nullptr);
            expectEquals(text.getTextComponent().getAttributedString().getWordWrap(),
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
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(tree);
            auto& text = *dynamic_cast<jive::GuiItemDecorator&>(*item)
                              .toType<jive::Text>();
            expectEquals(text.getTextComponent().getAttributedString().getWordWrap(),
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
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(tree);
            auto& text = *dynamic_cast<jive::GuiItemDecorator&>(*item)
                              .toType<jive::Text>();
            expectEquals(text.getTextComponent().getAttributedString().getReadingDirection(),
                         juce::AttributedString::ReadingDirection::natural);

            tree.setProperty("direction", "left-to-right", nullptr);
            expectEquals(text.getTextComponent().getAttributedString().getReadingDirection(),
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
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(tree);
            auto& text = *dynamic_cast<jive::GuiItemDecorator&>(*item)
                              .toType<jive::Text>();
            expectEquals(text.getTextComponent().getAttributedString().getReadingDirection(),
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
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(tree);
            auto& text = *dynamic_cast<jive::GuiItemDecorator&>(*item)
                              .toType<jive::Text>();
            expectEquals(text.getTextComponent().getAttributedString().getLineSpacing(),
                         0.0f);

            tree.setProperty("line-spacing", 1.234f, nullptr);
            expectEquals(text.getTextComponent().getAttributedString().getLineSpacing(),
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
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(tree);
            auto& text = *dynamic_cast<jive::GuiItemDecorator&>(*item)
                              .toType<jive::Text>();
            expectEquals(text.getTextComponent().getAttributedString().getLineSpacing(),
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
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(tree);
            auto* text = dynamic_cast<jive::GuiItemDecorator&>(*item)
                             .toType<jive::Text>();
            expectEquals(text->getTextComponent().getAttributedString().getNumAttributes(),
                         0);

            tree.appendChild(juce::ValueTree{
                                 "Text",
                                 {
                                     { "text", "Nested." },
                                 },
                             },
                             nullptr);
            item = interpreter.interpret(tree);
            text = dynamic_cast<jive::GuiItemDecorator&>(*item)
                       .toType<jive::Text>();
            expectEquals<juce::String>(text->getTextComponent().getAttributedString().getText(),
                                       "Nested.");
            tree.appendChild(juce::ValueTree{
                                 "Text",
                                 {
                                     { "text", " Sentence two." },
                                 },
                             },
                             nullptr);
            item = interpreter.interpret(tree);
            text = dynamic_cast<jive::GuiItemDecorator&>(*item)
                       .toType<jive::Text>();
            expectEquals<juce::String>(text->getTextComponent().getAttributedString().getText(),
                                       "Nested. Sentence two.");
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
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(tree);
            auto& text = *dynamic_cast<jive::GuiItemDecorator&>(*item)
                              .toType<jive::Text>();
            expectEquals<juce::String>(text.getTextComponent().getAttributedString().getText(),
                                       "Setup... Conflict... Revolution!");
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
            auto& item = *dynamic_cast<jive::GuiItemDecorator*>(parentItem->getChildren()[0])
                              ->toType<jive::Text>();
            const auto font = dynamic_cast<jive::TextComponent&>(*item.getComponent())
                                  .getAttributedString()
                                  .getAttribute(0)
                                  .font;

            const auto& boxModel = jive::boxModel(item);
            expectEquals(boxModel.getWidth(),
                         std::ceil(font.getStringWidthFloat("This side up.")));
            expectEquals(boxModel.getHeight(), std::ceil(font.getHeight()));

            textTree.setProperty("text", "This one spans\nmultiple lines.", nullptr);
            expectEquals(boxModel.getWidth(),
                         std::ceil(std::max({ font.getStringWidthFloat("This one spans"),
                                              font.getStringWidthFloat("multiple lines.") })));
            expectEquals(boxModel.getHeight(),
                         std::ceil(font.getHeight()) * 2.0f);

            textTree.setProperty("width", 100.0f, nullptr);
            textTree.setProperty("text",
                                 "A very very very very very very very very very "
                                 "very very long line.",
                                 nullptr);
            expectGreaterThan(boxModel.getHeight(), font.getHeight());
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
            auto& item = *parent->getChildren()[0];
            const auto& boxModel = jive::boxModel(item);
            expectLessOrEqual(boxModel.getWidth(), 40.0f);
            expectGreaterThan(boxModel.getWidth(), 0.0f);
        }
        {
            juce::ValueTree state{
                "Component",
                {
                    { "width", 5000 },
                    { "height", 3000 },
                    { "align-items", "flex-start" },
                },
                {
                    juce::ValueTree{
                        "Component",
                    },
                },
            };
            {
                jive::Interpreter interpreter;
                auto window = interpreter.interpret(state);
                expectEquals(window->getChildren()[0]->getComponent()->getWidth(), 0);
                expectEquals(window->getChildren()[0]->getComponent()->getHeight(), 0);
            }

            state.getChild(0).appendChild(juce::ValueTree{
                                              "Text",
                                              {
                                                  { "text", "Bacon ipsum dolor amet biltong ground" },
                                              },
                                          },
                                          nullptr);

            {
                jive::Interpreter interpreter;
                auto window = interpreter.interpret(state);
                auto attributedString = dynamic_cast<jive::GuiItemDecorator&>(*window->getChildren()[0]->getChildren()[0])
                                            .toType<jive::Text>()
                                            ->getTextComponent()
                                            .getAttributedString();
                juce::TextLayout layout;
                layout.createLayout(attributedString, 1000000.0f);
                expectGreaterOrEqual(window->getChildren()[0]->getComponent()->getWidth(),
                                     static_cast<int>(std::ceil(layout.getWidth())));
                expectGreaterOrEqual(window->getChildren()[0]->getComponent()->getHeight(),
                                     static_cast<int>(std::ceil(layout.getHeight())));
            }
        }
    }
};

static TextTest textTest;
#endif
