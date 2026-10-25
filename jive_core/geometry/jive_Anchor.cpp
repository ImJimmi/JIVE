#include "jive_Anchor.h"

namespace jive
{
    [[nodiscard]] static std::optional<Anchor> findNamedAnchor(const juce::String& name)
    {
        static const Length start{ 0.0f, Length::Unit::percentage };
        static const Length middle{ 50.0f, Length::Unit::percentage };
        static const Length end{ 100.0f, Length::Unit::percentage };
        static const std::array<std::pair<juce::String, Anchor>, 9> namedAnchors{ {
            { "top-left", { start, start } },
            { "centred-top", { middle, start } },
            { "top-right", { end, start } },
            { "centred-left", { start, middle } },
            { "centred", { middle, middle } },
            { "centred-right", { end, middle } },
            { "bottom-left", { start, end } },
            { "centred-bottom", { middle, end } },
            { "bottom-right", { end, end } },
        } };

        const auto namedAnchor = std::find_if(std::begin(namedAnchors),
                                              std::end(namedAnchors),
                                              [&name](const auto& pair) {
                                                  return pair.first == name;
                                              });

        if (namedAnchor == std::end(namedAnchors))
            return std::nullopt;

        return namedAnchor->second;
    }

    bool Anchor::operator==(const Anchor& other) const
    {
        return x == other.x && y == other.y;
    }

    juce::String Anchor::toString() const
    {
        return x.toString() + " " + y.toString();
    }

    Anchor Anchor::fromString(const juce::String& anchorString)
    {
        if (const auto namedAnchor = findNamedAnchor(anchorString.trim()))
            return *namedAnchor;

        const auto tokens = juce::StringArray::fromTokens(anchorString, " ", "");

        if (tokens.size() == 1)
            return Anchor{ tokens[0], tokens[0] };
        if (tokens.size() == 2)
            return Anchor{ tokens[0], tokens[1] };

        return {};
    }
} // namespace jive

namespace juce
{
    var VariantConverter<jive::Anchor>::toVar(const jive::Anchor& anchor)
    {
        return anchor.toString();
    }

    jive::Anchor VariantConverter<jive::Anchor>::fromVar(const var& value)
    {
        return jive::Anchor::fromString(value.toString());
    }
} // namespace juce

#if JIVE_UNIT_TESTS
class AnchorUnitTest : public juce::UnitTest
{
public:
    AnchorUnitTest()
        : juce::UnitTest{ "jive::Anchor", "jive" }
    {
    }

    void runTest() final
    {
        testNamedPoints();
        testLengths();
    }

private:
    void testNamedPoints()
    {
        beginTest("named points");

        expect(jive::Anchor::fromString("top-left") == jive::Anchor{ "0%", "0%" });
        expect(jive::Anchor::fromString("centred-top") == jive::Anchor{ "50%", "0%" });
        expect(jive::Anchor::fromString("top-right") == jive::Anchor{ "100%", "0%" });
        expect(jive::Anchor::fromString("centred-left") == jive::Anchor{ "0%", "50%" });
        expect(jive::Anchor::fromString("centred") == jive::Anchor{ "50%", "50%" });
        expect(jive::Anchor::fromString("centred-right") == jive::Anchor{ "100%", "50%" });
        expect(jive::Anchor::fromString("bottom-left") == jive::Anchor{ "0%", "100%" });
        expect(jive::Anchor::fromString("centred-bottom") == jive::Anchor{ "50%", "100%" });
        expect(jive::Anchor::fromString("bottom-right") == jive::Anchor{ "100%", "100%" });
        expect(jive::Anchor{} == jive::Anchor::fromString("top-left"));
    }

    void testLengths()
    {
        beginTest("lengths");

        expect(jive::Anchor::fromString("10px 50%") == jive::Anchor{ "10px", "50%" });
        expect(jive::Anchor::fromString("25% 3") == jive::Anchor{ "25%", "3px" });
        expect(jive::Anchor::fromString("75%") == jive::Anchor{ "75%", "75%" });
        expectEquals(jive::Anchor::fromString("10px 50%").toString(), juce::String{ "10px 50%" });
    }
};

static AnchorUnitTest anchorUnitTest;
#endif
