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
            background.setFill(juce::VariantConverter<juce::Colour>::fromVar(style.get()["background"]));
        };

        component->addAndMakeVisible(background);
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
                                  "background": "0xFF00FF00",
                              },
                          )"),
                          nullptr);
        auto snapshot = component->createComponentSnapshot(component->getLocalBounds());
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
};

static StyleSheetTest styleSheetTest;
#endif
