#include <jive_core/jive_core.h>

namespace jive
{
    static juce::String accumulateInlineText(juce::XmlElement& xml)
    {
        juce::String result;

        for (auto* child : xml.getChildIterator())
        {
            if (child->isTextElement())
                result += child->getText();
        }

        return result;
    }

    static void replaceInlineTextWithProperty(juce::XmlElement& xml)
    {
        const auto text = xml.getStringAttribute("text", "")
                        + accumulateInlineText(xml);

        if (text.isNotEmpty())
            xml.setAttribute("text", text);

        xml.deleteAllTextElements();
    }

    static void replaceInlineTextWithTextElement(juce::XmlElement& xml)
    {
        if (const auto text = accumulateInlineText(xml);
            text.isNotEmpty())
        {
            auto textElement = std::make_unique<juce::XmlElement>("Text");
            textElement->setAttribute("text", text);
            xml.addChildElement(textElement.release());
        }

        xml.deleteAllTextElements();
    }

    static void replaceInlineTextRecursive(juce::XmlElement& xml)
    {
        if (xml.getTagName() == "Text")
            replaceInlineTextWithProperty(xml);
        else
            replaceInlineTextWithTextElement(xml);

        for (auto* child : xml.getChildIterator())
            replaceInlineTextRecursive(*child);
    }

    [[nodiscard]] juce::ValueTree parseXML(const juce::XmlElement& sourceXML)
    {
        auto xml = sourceXML;
        replaceInlineTextRecursive(xml);

        return juce::ValueTree::fromXml(xml);
    }

    [[nodiscard]] juce::ValueTree parseXML(const juce::String& xmlString)
    {
        if (auto xml = juce::parseXML(xmlString))
            return parseXML(*xml);

        return {};
    }

    [[nodiscard]] juce::ValueTree parseXML(const void* xmlStringData, int xmlStringDataSize)
    {
        return jive::parseXML(juce::String::createStringFromData(xmlStringData, xmlStringDataSize));
    }
} // namespace jive

#if JIVE_UNIT_TESTS
class XmlParserUnitTest : public juce::UnitTest
{
public:
    XmlParserUnitTest()
        : juce::UnitTest{ "jive::parseXML()", "jive" }
    {
    }

    void runTest() final
    {
        testParsingStringData();
        testParsingString();
        testParsingXmlElement();
        testTextElementWithInlineText();
        testNonTextElementWithInlineText();
    }

private:
    void testParsingStringData()
    {
        beginTest("parsing string data");

        static const juce::String source = R"(
            <Foo x="12"/>
        )";
        const auto result = jive::parseXML(source.toRawUTF8(), source.length());
        expectEquals(result.getType().toString(), juce::String{ "Foo" });
        expect(result.hasProperty("x"));
        expectEquals(static_cast<int>(result["x"]), 12);
    }

    void testParsingString()
    {
        beginTest("parsing string");

        static const juce::String source = R"(
            <Bar y="21"/>
        )";
        const auto result = jive::parseXML(source);
        expectEquals(result.getType().toString(), juce::String{ "Bar" });
        expect(result.hasProperty("y"));
        expectEquals(static_cast<int>(result["y"]), 21);
    }

    void testParsingXmlElement()
    {
        beginTest("parsing xml");

        juce::XmlElement source{ "Maz" };
        source.setAttribute("z", "30");

        const auto result = jive::parseXML(source);
        expectEquals(result.getType().toString(), juce::String{ "Maz" });
        expect(result.hasProperty("z"));
        expectEquals(static_cast<int>(result["z"]), 30);
    }

    void testTextElementWithInlineText()
    {
        beginTest("text element with inline text");

        {
            static constexpr auto source = R"(
                <Text>Hello, World!</Text>
            )";
            const auto result = jive::parseXML(source);
            expect(result.hasProperty("text"));
            expectEquals(result["text"].toString(), juce::String{ "Hello, World!" });
        }
        {
            static constexpr auto source = R"(
                <Foo>
                    <Bar>
                        <Text>Goodbye, Dlrow!</Text>
                    </Bar>
                </Foo>
            )";
            const auto result = jive::parseXML(source);
            const auto text = result.getChild(0).getChild(0);
            expect(text.hasProperty("text"));
            expectEquals(text["text"].toString(), juce::String{ "Goodbye, Dlrow!" });
        }
    }

    void testNonTextElementWithInlineText()
    {
        beginTest("non-text element with inline text");

        {
            static constexpr auto source = R"(
                <Button>Click me!</Button>
            )";
            const auto result = jive::parseXML(source);
            expect(result.getChildWithName("Text").isValid());
            expect(result.getChildWithName("Text").hasProperty("text"));
            expectEquals(result.getChildWithName("Text")["text"].toString(),
                         juce::String{ "Click me!" });
        }
    }
};

static XmlParserUnitTest xmlParserUnitTest;
#endif
