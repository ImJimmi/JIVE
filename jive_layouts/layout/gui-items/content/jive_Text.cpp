#include "jive_Text.h"

#include <jive_layouts/layout/gui-items/jive_CommonGuiItem.h>
#include <jive_layouts/layout/gui-items/jive_ContainerItem.h>

namespace jive
{
    Text::Text(std::unique_ptr<GuiItem> itemToDecorate)
        : GuiItemDecorator{ std::move(itemToDecorate) }
        , text{ state, "text" }
        , lineSpacing{ state, "line-spacing" }
        , wordWrap{ state, "word-wrap" }
        , idealWidth{ state, "ideal-width" }
        , idealHeight{ state, "ideal-height" }
    {
        const BoxModel::ScopedCallbackLock boxModelLock{ boxModel(*this) };

        if (!wordWrap.exists())
            wordWrap = juce::AttributedString::WordWrap::byWord;

        text.onValueChange = [this]() {
            updateTextComponent();
        };
        wordWrap.onValueChange = [this]() {
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

#if JIVE_IS_PLUGIN_PROJECT
    void Text::attachToParameter(juce::RangedAudioParameter* newParameter, juce::UndoManager* undoManager)
    {
        parameter = newParameter;

        if (parameter != nullptr)
        {
            const auto onChange = [this](float) {
                text = parameter->getCurrentValueAsText();
            };
            parameterAttachment = std::make_unique<juce::ParameterAttachment>(*parameter, onChange, undoManager);
            text = parameter->getCurrentValueAsText();
        }
        else
        {
            parameterAttachment = nullptr;
        }
    }
#endif

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

    template <typename T>
    [[nodiscard]] static auto nextWholeNumberAbove(T value)
    {
        static_assert(std::is_floating_point<T>());
        const auto valueRoundedUp = std::ceil(value);

        if (juce::approximatelyEqual(value, valueRoundedUp))
            return valueRoundedUp + static_cast<T>(1);

        return valueRoundedUp;
    }

    void Text::updateTextComponent()
    {
        getTextComponent().setText(text);
        getTextComponent().setLineSpacing(lineSpacing);
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

        idealWidth = nextWholeNumberAbove(buildTextLayout(static_cast<float>(std::numeric_limits<juce::uint16>::max()))
                                              .getWidth());

        if (auto* parentItem = getParent())
        {
            if (!parentItem->isContainer())
                getTextComponent().setAccessible(false);

            if (auto* containerParent = dynamic_cast<GuiItemDecorator&>(*parentItem).getTopLevelDecorator().toType<ContainerItem>())
                containerParent->updateIdealSizeUnrestrained();
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
    #include <jive_core/graphics/jive_FontUtilities.h>
    #include <jive_layouts/layout/jive_Interpreter.h>

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
        testWordWrap();
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
                         std::ceil(jive::calculateStringWidth("This side up.", font)));
            expectEquals(boxModel.getHeight(), std::ceil(font.getHeight()));

            textTree.setProperty("text", "This one spans\nmultiple lines.", nullptr);
            expectEquals(boxModel.getWidth(),
                         std::ceil(std::max({
                             jive::calculateStringWidth("This one spans", font),
                             jive::calculateStringWidth("multiple lines.", font),
                         })));
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
