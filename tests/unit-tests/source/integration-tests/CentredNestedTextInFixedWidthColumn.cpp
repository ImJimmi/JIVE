#include <jive_layouts/jive_layouts.h>

struct CentredNestedTextInFixedWidthColumn : public juce::UnitTest
{
    CentredNestedTextInFixedWidthColumn()
        : juce::UnitTest{ "Centred nested text in fixed-width column", "jive" }
    {
    }

    void runTest() final
    {
        jive::Interpreter interpreter;
        auto root = interpreter.interpret(jive::parseXML(R"(
            <Window width="400" height="400" padding="10" style="{ '.caption': { 'font-size': 13 }, '.readout': { 'font-size': 12 } }">
                <Component id="control" align-items="center" gap="4" width="60">
                    <Component width="38" height="38" />
                    <Text id="direct" class="caption" word-wrap="none">Resonance</Text>
                    <Component id="group" align-items="center" gap="2">
                        <Text id="nested" class="caption" word-wrap="none">Resonance</Text>
                        <Text class="readout" word-wrap="none">250 ms</Text>
                    </Component>
                </Component>
            </Window>
        )"));

        const auto width = [&root](const juce::Identifier& id) {
            return jive::findItemWithID(*root, id)->getComponent()->getWidth();
        };

        beginTest("nested text is as wide as the same text placed directly");
        expectGreaterThan(width("direct"), 38);
        expectEquals(width("nested"), width("direct"));
        expectEquals(width("group"), width("direct"));
    }
};

static CentredNestedTextInFixedWidthColumn centredNestedTextInFixedWidthColumn;
