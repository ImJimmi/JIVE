#include <jive_style_sheets/jive_style_sheets.h>

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    StyleSheet::StyleSheet(juce::Component& sourceComponent,
                           juce::ValueTree sourceState)
        : component{ &sourceComponent }
        , state{ sourceState }
        , stateRoot{ state.getRoot() }
        , style{ state, "style" }
    {
        jassert(component != nullptr);

        applyStylesToCanvas();

        component->addAndMakeVisible(backgroundCanvas, 0);
        backgroundCanvas.setBounds(component->getLocalBounds());

        component->addComponentListener(this);
        component->addMouseListener(this, true);
        stateRoot.addListener(this);
    }

    StyleSheet::~StyleSheet()
    {
        if (component != nullptr)
            component->removeComponentListener(this);
    }

    //==================================================================================================================
    Fill StyleSheet::getBackground() const
    {
        return juce::VariantConverter<Fill>::fromVar(findStyleProperty("background"));
    }

    //==================================================================================================================
    void StyleSheet::componentMovedOrResized(juce::Component& componentThatWasMovedOrResized,
                                             bool /*wasMoved*/,
                                             bool /*wasResized*/)
    {
        jassertquiet(&componentThatWasMovedOrResized == component);
        backgroundCanvas.setBounds(component->getLocalBounds());
    }

    void StyleSheet::mouseEnter(const juce::MouseEvent&)
    {
        applyStylesToCanvas();
    }

    void StyleSheet::mouseExit(const juce::MouseEvent&)
    {
        applyStylesToCanvas();
    }

    void StyleSheet::mouseDown(const juce::MouseEvent&)
    {
        applyStylesToCanvas();
    }

    void StyleSheet::mouseUp(const juce::MouseEvent&)
    {
        applyStylesToCanvas();
    }

    void StyleSheet::valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier& id)
    {
        if (id == juce::Identifier{ "style" })
            applyStylesToCanvas();
    }

    //==================================================================================================================
    juce::StringArray findApplicableSelectorsInOrderOfSpecificity(const juce::Component& component)
    {
        juce::StringArray selectors;

        if (!component.isEnabled())
            selectors.add("disabled");

        if (component.hasKeyboardFocus(false))
            selectors.add("focus");

        if (component.isMouseButtonDown(true))
            selectors.add("active");

        if (component.isMouseOverOrDragging(true))
            selectors.add("hover");

        return selectors;
    }

    juce::var findStyleProperty(const Object& object,
                                const juce::Identifier& componentType,
                                const juce::Identifier& propertyName,
                                const juce::Component& component)
    {
        auto selectors = juce::StringArray{ componentType.toString() };
        selectors.addArray(findApplicableSelectorsInOrderOfSpecificity(component));

        for (const auto& selector : selectors)
        {
            if (object.hasProperty(selector))
            {
                const auto& nested = dynamic_cast<Object&>(*object.getProperty(selector).getDynamicObject());
                return findStyleProperty(nested, componentType, propertyName, component);
            }
        }

        return object.getProperty(propertyName);
    }

    juce::var findStyleProperty(const juce::ValueTree& state,
                                const juce::Identifier& componentType,
                                const juce::Identifier& propertyName,
                                const juce::Component& component)
    {
        jassert(state.isValid());

        if (auto object = Property<Object::ReferenceCountedPointer>{ state, "style" }.get())
        {
            if (auto value = findStyleProperty(*object, componentType, propertyName, component);
                !value.isUndefined())
            {
                return value;
            }
        }

        if (auto parent = state.getParent();
            parent.isValid())
        {
            return findStyleProperty(parent, componentType, propertyName, component);
        }

        return {};
    }

    juce::var StyleSheet::findStyleProperty(const juce::Identifier& propertyName) const
    {
        return ::jive::findStyleProperty(state, state.getType(), propertyName, *component);
    }

    //==================================================================================================================
    void StyleSheet::applyStylesToCanvas()
    {
        backgroundCanvas.setFill(getBackground());
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

        juce::Component component;
        juce::ValueTree state{ "Component" };
        jive::StyleSheet styleSheet{ component, state };
        expectEquals(component.getNumChildComponents(), 1);
        expect(jive::find<jive::BackgroundCanvas>(component) != nullptr);

        component.setBounds(0, 0, 10, 10);
        state.setProperty("style",
                          R"(
                              {
                              },
                          )",
                          nullptr);
        auto snapshot = component.createComponentSnapshot(component.getLocalBounds());
        expect(compare(snapshot, juce::Colour{ 0x00000000 }));

        component.setBounds(0, 0, 10, 10);
        state.setProperty("style",
                          R"(
                              {
                                  "background": "0xFF00FF00",
                              },
                          )",
                          nullptr);
        snapshot = component.createComponentSnapshot(component.getLocalBounds());
        expect(compare(snapshot, juce::Colour{ 0xFF00FF00 }));

        state.setProperty("style",
                          R"(
                              {
                                  "background": "#BADD06",
                              },
                          )",
                          nullptr);
        snapshot = component.createComponentSnapshot(component.getLocalBounds());
        expect(compare(snapshot, juce::Colour{ 0xFFBADD06 }));

        state.setProperty("style",
                          R"(
                              {
                                  "background": 'rgb(201, 76, 76)',
                              },
                          )",
                          nullptr);
        snapshot = component.createComponentSnapshot(component.getLocalBounds());
        expect(compare(snapshot, juce::Colour{ 0xFFC94C4C }));

        state.setProperty("style",
                          R"(
                              {
                                  "background": 'rgba(201, 76, 76, 1.0)',
                              },
                          )",
                          nullptr);
        snapshot = component.createComponentSnapshot(component.getLocalBounds());
        expect(compare(snapshot, juce::Colour{ 0xFFC94C4C }));

        state.setProperty("style",
                          R"(
                              {
                                  "background": 'hsl(89, 43%, 51%)',
                              },
                          )",
                          nullptr);
        snapshot = component.createComponentSnapshot(component.getLocalBounds());
        expect(compare(snapshot, juce::Colour{ 0xFF84B84C }));

        state.setProperty("style",
                          R"(
                              {
                                  "background": 'hsla(89, 43%, 51%, 1.0)',
                              },
                          )",
                          nullptr);
        snapshot = component.createComponentSnapshot(component.getLocalBounds());
        expect(compare(snapshot, juce::Colour{ 0xFF84B84C }));
    }

    void testLinearBackgroundGradient()
    {
        beginTest("linear background gradient");

        juce::Component component;
        juce::ValueTree state{ "Component" };
        jive::StyleSheet styleSheet{ component, state };
        component.setBounds(0, 0, 10, 10);
        state.setProperty("style",
                          R"(
                              {
                                  "background": {
                                      "gradient": "linear",
                                      "stops": {
                                          "0": "#111111",
                                          "1": "#999999",
                                      }
                                  },
                              },
                          )",
                          nullptr);
        auto snapshot = component.createComponentSnapshot(component.getLocalBounds());
        expectEquals(snapshot.getPixelAt(0, 0), juce::Colour{ 0xFF161616 });
        expectEquals(snapshot.getPixelAt(9, 0), juce::Colour{ 0xFF161616 });
        expectEquals(snapshot.getPixelAt(0, 9), juce::Colour{ 0xFF949494 });
        expectEquals(snapshot.getPixelAt(9, 9), juce::Colour{ 0xFF949494 });

        state.setProperty("style",
                          R"(
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
                          )",
                          nullptr);
        snapshot = component.createComponentSnapshot(component.getLocalBounds());
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

        juce::Component component;
        juce::ValueTree state{ "CustomType" };
        jive::StyleSheet styleSheet{ component, state };

        component.setBounds(0, 0, 10, 10);
        state.setProperty("style",
                          R"(
                              {
                                  "MyWidget": {
                                      "background": "#7F7F7F",
                                  }
                              },
                          )",
                          nullptr);
        auto snapshot = component.createComponentSnapshot(component.getLocalBounds());
        expect(compare(snapshot, juce::Colour{ 0x00000000 }));

        state.setProperty("style",
                          R"(
                              {
                                  "CustomType": {
                                      "background": "#7F7F7F",
                                  }
                              },
                          )",
                          nullptr);
        snapshot = component.createComponentSnapshot(component.getLocalBounds());
        expect(compare(snapshot, juce::Colour{ 0xFF7F7F7F }));
    }

    void testHereditaryStyling()
    {
        beginTest("hereditary styling");

        juce::ValueTree state{
            "Root",
            {
                { "style", R"({ "background": "#111111" })" },
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

        juce::Component component;
        component.setBounds(0, 0, 10, 10);

        jive::StyleSheet styleSheet{ component, state.getChild(0).getChild(0) };
        auto snapshot = component.createComponentSnapshot(component.getLocalBounds());
        expect(compare(snapshot, juce::Colour{ 0xFF111111 }));

        state.getChild(0).setProperty("style", R"({ "background": "#222222" })", nullptr);
        snapshot = component.createComponentSnapshot(component.getLocalBounds());
        expect(compare(snapshot, juce::Colour{ 0xFF222222 }));

        state.setProperty("style", R"({ "background": "#333333" })", nullptr);
        snapshot = component.createComponentSnapshot(component.getLocalBounds());
        expect(compare(snapshot, juce::Colour{ 0xFF222222 }));

        state.getChild(0).getChild(0).setProperty("style", R"({ "background": "#444444" })", nullptr);
        snapshot = component.createComponentSnapshot(component.getLocalBounds());
        expect(compare(snapshot, juce::Colour{ 0xFF444444 }));
    }
};

static StyleSheetTest styleSheetTest;
#endif
