#include <jive_style_sheets/jive_style_sheets.h>

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    StyleSheet::StyleSheet(std::shared_ptr<juce::Component> sourceComponent,
                           juce::ValueTree sourceState)
        : component{ sourceComponent }
        , state{ sourceState }
        , style{ state, "style" }
    {
        style.onValueChange = [this]() {
            setStyle(collateStyle());
        };
        setStyle(collateStyle());

        component->addAndMakeVisible(background, 0);
        background.setBounds(sourceComponent->getLocalBounds());

        component->addComponentListener(this);
    }

    StyleSheet::~StyleSheet()
    {
        component->removeComponentListener(this);
    }

    //==================================================================================================================
    void StyleSheet::componentMovedOrResized(juce::Component& componentThatWasMovedOrResized,
                                             bool /*wasMoved*/,
                                             bool /*wasResized*/)
    {
        jassertquiet(&componentThatWasMovedOrResized == component.get());
        background.setBounds(component->getLocalBounds());
    }

    //==================================================================================================================
    juce::FillType getBackgroundFill(juce::var style,
                                     juce::ValueTree state)
    {
        using Converter = juce::VariantConverter<juce::Colour>;

        if (style.hasProperty("background"))
            return Converter::fromVar(style["background"]);

        if (style.hasProperty(state.getType()))
            return getBackgroundFill(style[state.getType()], state);

        return juce::Colour{};
    }

    juce::FillType StyleSheet::getBackgroundFill() const
    {
        return ::jive::getBackgroundFill(style.get(), state);
    }

    juce::var findPropertyOfStyleSheet(juce::var style,
                                       const juce::Identifier& property,
                                       const juce::Identifier& type)
    {
        if (style.hasProperty(property))
            return style[property];

        if (style.hasProperty(type))
            return findPropertyOfStyleSheet(style[type], property, type);

        return {};
    }

    juce::var findPropertyOfStyleSheet(juce::ValueTree state,
                                       const juce::Identifier& property)
    {
        if (state.hasProperty("style"))
        {
            const auto style = state["style"];
            const auto value = findPropertyOfStyleSheet(style, property, state.getType());

            if (value != juce::var{})
                return value;
        }

        const auto parent = state.getParent();

        if (parent.isValid())
            return findPropertyOfStyleSheet(parent, property);

        return {};
    }

    StyleSheet::Style StyleSheet::collateStyle() const
    {
        Style collatedStyle;

        collatedStyle.background = juce::VariantConverter<Fill>::fromVar(findPropertyOfStyleSheet(state, "background"));

        return collatedStyle;
    }

    void StyleSheet::setStyle(Style collatedStyle)
    {
        background.setFill(collatedStyle.background);
    }
} // namespace jive

//======================================================================================================================
#if JIVE_UNIT_TESTS
bool compare(const juce::Image& image, juce::Colour colour)
{
    for (auto y = 0; y < image.getHeight(); y++)
    {
        for (auto x = 0; x < image.getWidth(); x++)
        {
            if (image.getPixelAt(x, y) != colour)
                return false;
        }
    }

    return true;
}

bool withinAbsoluteError(juce::Colour lhs, juce::Colour rhs, juce::uint8 tolerance)
{
    return std::abs(lhs.getAlpha() - rhs.getAlpha()) <= tolerance
        && std::abs(lhs.getRed() - rhs.getRed()) <= tolerance
        && std::abs(lhs.getGreen() - rhs.getGreen()) <= tolerance
        && std::abs(lhs.getBlue() - rhs.getBlue()) <= tolerance;
}

class StyleSheetTest : public juce::UnitTest
{
public:
    StyleSheetTest()
        : juce::UnitTest{ "jive::StyleSheet", "jive" }
    {
    }

    void runTest() final
    {
        testBackgroundColour();
        testLinearBackgroundGradient();
        testTypeStyling();
        testHereditaryStyling();
    }

private:
    void testBackgroundColour()
    {
        beginTest("background colour");

        auto component = std::make_shared<juce::Component>();
        juce::ValueTree state{ "Component" };
        jive::StyleSheet styleSheet{ component, state };
        expectEquals(component->getNumChildComponents(), 1);
        expect(jive::find<jive::BackgroundCanvas>(*component) != nullptr);

        component->setBounds(0, 0, 10, 10);
        state.setProperty("style",
                          juce::JSON::parse(R"(
                              {
                              },
                          )"),
                          nullptr);
        auto snapshot = component->createComponentSnapshot(component->getLocalBounds());
        expect(compare(snapshot, juce::Colour{ 0x00000000 }));

        component->setBounds(0, 0, 10, 10);
        state.setProperty("style",
                          juce::JSON::parse(R"(
                              {
                                  "background": "0xFF00FF00",
                              },
                          )"),
                          nullptr);
        snapshot = component->createComponentSnapshot(component->getLocalBounds());
        expect(compare(snapshot, juce::Colour{ 0xFF00FF00 }));

        state.setProperty("style",
                          juce::JSON::parse(R"(
                              {
                                  "background": "#BADD06",
                              },
                          )"),
                          nullptr);
        snapshot = component->createComponentSnapshot(component->getLocalBounds());
        expect(compare(snapshot, juce::Colour{ 0xFFBADD06 }));

        state.setProperty("style",
                          juce::JSON::parse(R"(
                              {
                                  "background": 'rgb(201, 76, 76)',
                              },
                          )"),
                          nullptr);
        snapshot = component->createComponentSnapshot(component->getLocalBounds());
        expect(compare(snapshot, juce::Colour{ 0xFFC94C4C }));

        state.setProperty("style",
                          juce::JSON::parse(R"(
                              {
                                  "background": 'rgba(201, 76, 76, 1.0)',
                              },
                          )"),
                          nullptr);
        snapshot = component->createComponentSnapshot(component->getLocalBounds());
        expect(compare(snapshot, juce::Colour{ 0xFFC94C4C }));

        state.setProperty("style",
                          juce::JSON::parse(R"(
                              {
                                  "background": 'hsl(89, 43%, 51%)',
                              },
                          )"),
                          nullptr);
        snapshot = component->createComponentSnapshot(component->getLocalBounds());
        expect(compare(snapshot, juce::Colour{ 0xFF84B84C }));

        state.setProperty("style",
                          juce::JSON::parse(R"(
                              {
                                  "background": 'hsla(89, 43%, 51%, 1.0)',
                              },
                          )"),
                          nullptr);
        snapshot = component->createComponentSnapshot(component->getLocalBounds());
        expect(compare(snapshot, juce::Colour{ 0xFF84B84C }));
    }

    void testLinearBackgroundGradient()
    {
        beginTest("linear background gradient");

        auto component = std::make_shared<juce::Component>();
        juce::ValueTree state{ "Component" };
        jive::StyleSheet styleSheet{ component, state };
        component->setBounds(0, 0, 10, 10);
        state.setProperty("style",
                          juce::JSON::parse(R"(
                              {
                                  "background": {
                                      "gradient": "linear",
                                      "stops": {
                                          "0": "#111111",
                                          "1": "#999999",
                                      }
                                  },
                              },
                          )"),
                          nullptr);
        auto snapshot = component->createComponentSnapshot(component->getLocalBounds());
        expectEquals(snapshot.getPixelAt(0, 0), juce::Colour{ 0xFF161616 });
        expectEquals(snapshot.getPixelAt(9, 0), juce::Colour{ 0xFF161616 });
        expectEquals(snapshot.getPixelAt(0, 9), juce::Colour{ 0xFF949494 });
        expectEquals(snapshot.getPixelAt(9, 9), juce::Colour{ 0xFF949494 });

        state.setProperty("style",
                          juce::JSON::parse(R"(
                              {
                                  "background": {
                                      "gradient": "linear",
                                      "orientation": "horizontal",
                                      "stops": {
                                          "0": "#111111",
                                          "1": "#999999",
                                      }
                                  },
                              },
                          )"),
                          nullptr);
        snapshot = component->createComponentSnapshot(component->getLocalBounds());
        juce::PNGImageFormat format;
        auto stream = juce::File::getSpecialLocation(juce::File::userDesktopDirectory).getChildFile("snapshot.png").createOutputStream();
        format.writeImageToStream(snapshot, *stream);
        expect(withinAbsoluteError(snapshot.getPixelAt(0, 0), juce::Colour{ 0xFF161616 }, 1));
        expect(withinAbsoluteError(snapshot.getPixelAt(0, 9), juce::Colour{ 0xFF161616 }, 1));
        expect(withinAbsoluteError(snapshot.getPixelAt(9, 0), juce::Colour{ 0xFF949494 }, 1));
        expect(withinAbsoluteError(snapshot.getPixelAt(9, 9), juce::Colour{ 0xFF949494 }, 1));
    }

    void testTypeStyling()
    {
        beginTest("type styling");

        auto component = std::make_shared<juce::Component>();
        juce::ValueTree state{ "CustomType" };
        jive::StyleSheet styleSheet{ component, state };

        component->setBounds(0, 0, 10, 10);
        state.setProperty("style",
                          juce::JSON::parse(R"(
                              {
                                  "MyWidget": {
                                      "background": "#7F7F7F",
                                  }
                              },
                          )"),
                          nullptr);
        auto snapshot = component->createComponentSnapshot(component->getLocalBounds());
        expect(compare(snapshot, juce::Colour{ 0x00000000 }));

        state.setProperty("style",
                          juce::JSON::parse(R"(
                              {
                                  "CustomType": {
                                      "background": "#7F7F7F",
                                  }
                              },
                          )"),
                          nullptr);
        snapshot = component->createComponentSnapshot(component->getLocalBounds());
        expect(compare(snapshot, juce::Colour{ 0xFF7F7F7F }));
    }

    void testHereditaryStyling()
    {
        beginTest("hereditary styling");

        juce::ValueTree state{
            "Root",
            {
                { "style", juce::JSON::parse(R"({ "background": "#111111" })") },
            },
            {
                juce::ValueTree{
                    "Parent",
                    {},
                    {
                        juce::ValueTree{ "Child" },
                    },
                },
            },
        };

        auto component = std::make_shared<juce::Component>();
        component->setBounds(0, 0, 10, 10);

        jive::StyleSheet styleSheet{ component, state.getChild(0).getChild(0) };
        auto snapshot = component->createComponentSnapshot(component->getLocalBounds());
        expect(compare(snapshot, juce::Colour{ 0xFF111111 }));

        state.getChild(0).setProperty("style", juce::JSON::parse(R"({ "background": "#222222" })"), nullptr);
        snapshot = component->createComponentSnapshot(component->getLocalBounds());
        expect(compare(snapshot, juce::Colour{ 0xFF222222 }));

        state.setProperty("style", juce::JSON::parse(R"({ "background": "#333333" })"), nullptr);
        snapshot = component->createComponentSnapshot(component->getLocalBounds());
        expect(compare(snapshot, juce::Colour{ 0xFF222222 }));

        state.getChild(0).getChild(0).setProperty("style", juce::JSON::parse(R"({ "background": "#444444" })"), nullptr);
        snapshot = component->createComponentSnapshot(component->getLocalBounds());
        expect(compare(snapshot, juce::Colour{ 0xFF444444 }));
    }
};

static StyleSheetTest styleSheetTest;
#endif
