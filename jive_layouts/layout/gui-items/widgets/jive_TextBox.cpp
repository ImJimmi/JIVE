#include "jive_TextBox.h"

namespace jive
{
    TextBox::TextBox(std::unique_ptr<GuiItem> itemToDecorate)
        : GuiItemDecorator(std::move(itemToDecorate))
        , text{ state, ids::text() }
        , placeholder{ state, ids::placeholder() }
        , multiLine{ state, ids::multiLine() }
        , readOnly{ state, ids::readOnly() }
        , tooltip{ state, ids::tooltip() }
        , minWidth{ state, ids::minWidth() }
        , minHeight{ state, ids::minHeight() }
        , focusable{ state, ids::focusable() }
        , onChange{ state, ids::onChange() }
    {
        const BoxModel::ScopedCallbackLock boxModelLock{ boxModel(*this) };

        if (!focusable.exists())
            focusable = true;
        if (!minWidth.exists())
            minWidth = 135.0f;
        if (!minHeight.exists())
            minHeight = 24.0f;

        auto& editor = getTextEditor();

        multiLine.onValueChange = [this]() {
            getTextEditor().setMultiLine(multiLine);
            getTextEditor().setReturnKeyStartsNewLine(multiLine);
        };
        editor.setMultiLine(multiLine);
        editor.setReturnKeyStartsNewLine(multiLine);

        readOnly.onValueChange = [this]() {
            getTextEditor().setReadOnly(readOnly);
        };
        editor.setReadOnly(readOnly);

        text.onValueChange = [this]() {
            if (getTextEditor().getText() != text.get())
                getTextEditor().setText(text, false);
        };
        editor.setText(text, false);

        placeholder.onValueChange = [this]() {
            updatePlaceholder();
        };
        updatePlaceholder();

        tooltip.onValueChange = [this]() {
            getTextEditor().setTooltip(translate(tooltip));
        };
        editor.setTooltip(translate(tooltip));

        editor.addListener(this);
        state.addListener(this);
    }

    TextBox::~TextBox()
    {
        state.removeListener(this);
        getTextEditor().removeListener(this);
    }

    void TextBox::textEditorTextChanged(juce::TextEditor& editor)
    {
        text = editor.getText();
        onChange.trigger();
    }

    void TextBox::valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property)
    {
        forwardPropertyChange(tree, property, text, placeholder, multiLine, readOnly, tooltip);
    }

    void TextBox::localisedStringsChanged()
    {
        GuiItemDecorator::localisedStringsChanged();

        updatePlaceholder();
        getTextEditor().setTooltip(translate(tooltip));
    }

    void TextBox::updatePlaceholder()
    {
        auto& editor = getTextEditor();
        editor.setTextToShowWhenEmpty(translate(placeholder),
                                      editor.findColour(juce::TextEditor::textColourId).withMultipliedAlpha(0.5f));
    }

    bool TextBox::isContainer() const
    {
        return false;
    }

    juce::TextEditor& TextBox::getTextEditor()
    {
        return *dynamic_cast<juce::TextEditor*>(getComponent().get());
    }

    const juce::TextEditor& TextBox::getTextEditor() const
    {
        return *dynamic_cast<const juce::TextEditor*>(getComponent().get());
    }
} // namespace jive

#if JIVE_UNIT_TESTS
    #include <jive_layouts/layout/interpreter/jive_Interpreter.h>

class TextBoxTest : public juce::UnitTest
{
public:
    TextBoxTest()
        : juce::UnitTest{ "jive::TextBox", "jive" }
    {
    }

    void runTest() final
    {
        testGuiItem();
        testText();
        testPlaceholder();
        testMultiLine();
        testReadOnly();
        testEvents();
    }

private:
    jive::Interpreter interpreter;

    [[nodiscard]] auto& getEditor(jive::GuiItem& item)
    {
        return dynamic_cast<jive::GuiItemDecorator&>(item).toType<jive::TextBox>()->getTextEditor();
    }

    // Text editors report changes asynchronously, so the message is handled
    // here directly to keep the tests synchronous.
    static void type(juce::TextEditor& editor, const juce::String& text)
    {
        editor.setText(text, false);
        dynamic_cast<juce::Component&>(editor).handleCommandMessage(0x10003001);
    }

    void testGuiItem()
    {
        beginTest("gui-item");

        auto item = interpreter.interpret(juce::ValueTree{
            "TextBox",
            {
                { "width", 222 },
                { "height", 333 },
            },
        });
        expect(!item->isContainer());
    }

    void testText()
    {
        beginTest("text");

        juce::ValueTree tree{
            "TextBox",
            {
                { "width", 222 },
                { "height", 333 },
                { "text", "Hello" },
            },
        };
        auto item = interpreter.interpret(tree);
        auto& editor = getEditor(*item);
        expectEquals(editor.getText(), juce::String{ "Hello" });

        tree.setProperty("text", "World", nullptr);
        expectEquals(editor.getText(), juce::String{ "World" });

        type(editor, "Typed");
        expectEquals(tree["text"].toString(), juce::String{ "Typed" });
    }

    void testPlaceholder()
    {
        beginTest("placeholder");

        juce::ValueTree tree{
            "TextBox",
            {
                { "width", 222 },
                { "height", 333 },
            },
        };
        auto item = interpreter.interpret(tree);
        auto& editor = getEditor(*item);
        expect(editor.getTextToShowWhenEmpty().isEmpty());

        tree.setProperty("placeholder", "Search", nullptr);
        expectEquals(editor.getTextToShowWhenEmpty(), juce::String{ "Search" });
    }

    void testMultiLine()
    {
        beginTest("multi-line");

        juce::ValueTree tree{
            "TextBox",
            {
                { "width", 222 },
                { "height", 333 },
            },
        };
        auto item = interpreter.interpret(tree);
        auto& editor = getEditor(*item);
        expect(!editor.isMultiLine());

        tree.setProperty("multi-line", true, nullptr);
        expect(editor.isMultiLine());
        expect(editor.getReturnKeyStartsNewLine());
    }

    void testReadOnly()
    {
        beginTest("read-only");

        juce::ValueTree tree{
            "TextBox",
            {
                { "width", 222 },
                { "height", 333 },
                { "read-only", true },
            },
        };
        auto item = interpreter.interpret(tree);
        auto& editor = getEditor(*item);
        expect(editor.isReadOnly());

        tree.setProperty("read-only", false, nullptr);
        expect(!editor.isReadOnly());
    }

    void testEvents()
    {
        beginTest("events");

        juce::ValueTree tree{
            "TextBox",
            {
                { "width", 222 },
                { "height", 333 },
            },
        };
        auto item = interpreter.interpret(tree);
        auto& editor = getEditor(*item);
        jive::Event onChange{ tree, "on-change" };
        expectEquals(onChange.getAssumedTriggerCount(), 0);

        type(editor, "abc");
        expectEquals(onChange.getAssumedTriggerCount(), 1);

        tree.setProperty("text", "def", nullptr);
        expectEquals(onChange.getAssumedTriggerCount(), 1);
    }
};

static TextBoxTest textBoxTest;
#endif
