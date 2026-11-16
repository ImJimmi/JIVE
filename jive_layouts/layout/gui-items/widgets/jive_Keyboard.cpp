#include "jive_Keyboard.h"

#if JUCE_MODULE_AVAILABLE_juce_audio_utils
namespace jive
{
    [[nodiscard]] static std::optional<int> parseNote(const juce::String& text, int octaveForMiddleC)
    {
        const auto trimmed = text.trim();

        if (trimmed.isEmpty())
            return std::nullopt;

        if (trimmed.containsOnly("-0123456789"))
            return trimmed.getIntValue();

        static constexpr std::array<int, 7> pitchClasses{ 9, 11, 0, 2, 4, 5, 7 };
        const auto letter = juce::CharacterFunctions::toUpperCase(trimmed[0]);

        if (letter < 'A' || letter > 'G')
            return std::nullopt;

        auto note = pitchClasses[static_cast<std::size_t>(letter - 'A')];
        auto rest = trimmed.substring(1);

        for (; rest.startsWithChar('#') || rest.startsWithChar('b'); rest = rest.substring(1))
            note += rest.startsWithChar('#') ? 1 : -1;

        if (rest.isEmpty() || !rest.containsOnly("-0123456789"))
            return std::nullopt;

        return 60 + note + (rest.getIntValue() - octaveForMiddleC) * 12;
    }

    Keyboard::Keyboard(std::unique_ptr<GuiItem> itemToDecorate)
        : GuiItemDecorator{ std::move(itemToDecorate) }
        , startNote{ state, ids::startNote() }
        , endNote{ state, ids::endNote() }
        , minWidth{ state, ids::minWidth() }
        , minHeight{ state, ids::minHeight() }
        , focusable{ state, ids::focusable() }
    {
        const BoxModel::ScopedCallbackLock boxModelLock{ boxModel(*this) };

        if (!startNote.exists())
            startNote = "C2";
        if (!endNote.exists())
            endNote = "C6";
        if (!minWidth.exists())
            minWidth = 360.0f;
        if (!minHeight.exists())
            minHeight = 80.0f;
        if (!focusable.exists())
            focusable = true;

        startNote.onValueChange = [this]() {
            updateNoteRange();
        };
        endNote.onValueChange = [this]() {
            updateNoteRange();
        };
        updateNoteRange();

        state.addListener(this);
    }

    Keyboard::~Keyboard()
    {
        state.removeListener(this);
    }

    void Keyboard::valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property)
    {
        if (startNote.handlePropertyChange(tree, property))
            return;

        endNote.handlePropertyChange(tree, property);
    }

    void Keyboard::updateNoteRange()
    {
        const auto octaveForMiddleC = getKeyboard().getOctaveForMiddleC();
        const auto start = juce::jlimit(0, 126, parseNote(startNote, octaveForMiddleC).value_or(36));
        const auto end = juce::jlimit(start + 1, 127, parseNote(endNote, octaveForMiddleC).value_or(84));

        getKeyboard().setNoteRange(start, end);
    }

    bool Keyboard::isContainer() const
    {
        return false;
    }

    KeyboardComponent& Keyboard::getKeyboard()
    {
        return *dynamic_cast<KeyboardComponent*>(getComponent().get());
    }

    const KeyboardComponent& Keyboard::getKeyboard() const
    {
        return *dynamic_cast<const KeyboardComponent*>(getComponent().get());
    }
} // namespace jive

    #if JIVE_UNIT_TESTS
        #include <jive_layouts/layout/interpreter/jive_Interpreter.h>

class KeyboardTest : public juce::UnitTest
{
public:
    KeyboardTest()
        : juce::UnitTest{ "jive::Keyboard", "jive" }
    {
    }

    void runTest() final
    {
        testNoteRange();
        testNoteNames();
        testKeysFillWidth();
        testDefaultSize();
        testSharedState();
    }

private:
    jive::Interpreter interpreter;

    [[nodiscard]] auto& getKeyboard(jive::GuiItem& item)
    {
        return dynamic_cast<jive::GuiItemDecorator&>(item).toType<jive::Keyboard>()->getKeyboard();
    }

    void testNoteRange()
    {
        beginTest("note range");

        juce::ValueTree tree{
            "Keyboard",
            {
                { "width", 400 },
                { "height", 100 },
            },
        };
        auto item = interpreter.interpret(tree);
        auto& keyboard = getKeyboard(*item);
        expectEquals(keyboard.getRangeStart(), 36);
        expectEquals(keyboard.getRangeEnd(), 84);

        tree.setProperty("start-note", 24, nullptr);
        tree.setProperty("end-note", 72, nullptr);
        expectEquals(keyboard.getRangeStart(), 24);
        expectEquals(keyboard.getRangeEnd(), 72);

        tree.setProperty("start-note", -10, nullptr);
        tree.setProperty("end-note", 500, nullptr);
        expectEquals(keyboard.getRangeStart(), 0);
        expectEquals(keyboard.getRangeEnd(), 127);

        tree.setProperty("start-note", 60, nullptr);
        tree.setProperty("end-note", 12, nullptr);
        expectEquals(keyboard.getRangeStart(), 60);
        expectEquals(keyboard.getRangeEnd(), 61);
    }

    void testNoteNames()
    {
        beginTest("note names");

        juce::ValueTree tree{
            "Keyboard",
            {
                { "width", 400 },
                { "height", 100 },
                { "start-note", "C1" },
                { "end-note", "c5" },
            },
        };
        auto item = interpreter.interpret(tree);
        auto& keyboard = getKeyboard(*item);
        expectEquals(keyboard.getRangeStart(), 24);
        expectEquals(keyboard.getRangeEnd(), 72);

        tree.setProperty("start-note", "F#2", nullptr);
        tree.setProperty("end-note", "Bb4", nullptr);
        expectEquals(keyboard.getRangeStart(), 42);
        expectEquals(keyboard.getRangeEnd(), 70);

        tree.setProperty("start-note", "A-1", nullptr);
        expectEquals(keyboard.getRangeStart(), 9);

        tree.setProperty("start-note", "H2", nullptr);
        tree.setProperty("end-note", "C", nullptr);
        expectEquals(keyboard.getRangeStart(), 36);
        expectEquals(keyboard.getRangeEnd(), 84);
    }

    void testKeysFillWidth()
    {
        beginTest("keys fill the width");

        juce::ValueTree tree{
            "Keyboard",
            {
                { "width", 290 },
                { "height", 100 },
                { "start-note", 36 },
                { "end-note", 84 },
            },
        };
        auto item = interpreter.interpret(tree);
        auto& keyboard = getKeyboard(*item);
        expectWithinAbsoluteError(keyboard.getKeyWidth(), 10.0f, 0.001f);

        tree.setProperty("end-note", 60, nullptr);
        expectWithinAbsoluteError(keyboard.getKeyWidth(), 290.0f / 15.0f, 0.001f);
    }

    void testDefaultSize()
    {
        beginTest("default size");

        juce::ValueTree parentState{
            "Component",
            {
                { "width", 999 },
                { "height", 999 },
                { "align-items", "flex-start" },
            },
            {
                juce::ValueTree{ "Keyboard" },
            },
        };
        auto parent = interpreter.interpret(parentState);
        auto& keyboard = *dynamic_cast<jive::GuiItemDecorator&>(*parent->getChildren()[0])
                              .toType<jive::Keyboard>();
        expect(!keyboard.isContainer());
        const auto& boxModel = jive::boxModel(keyboard);
        expectEquals(boxModel.getWidth(), 360.0f);
        expectEquals(boxModel.getHeight(), 80.0f);
    }

    void testSharedState()
    {
        beginTest("shared state");

        juce::ValueTree tree{
            "Keyboard",
            {
                { "width", 400 },
                { "height", 100 },
            },
        };
        auto ownItem = interpreter.interpret(tree);
        auto& ownKeyboard = getKeyboard(*ownItem);

        juce::MidiKeyboardState state;
        jive::Interpreter sharingInterpreter;
        sharingInterpreter.setMidiKeyboardState(state);
        auto sharedItem = sharingInterpreter.interpret(tree.createCopy());
        auto& sharedKeyboard = getKeyboard(*sharedItem);
        expect(&sharedKeyboard.getState() == &state);
        expect(&ownKeyboard.getState() != &state);
    }
};

static KeyboardTest keyboardTest;
    #endif
#endif
