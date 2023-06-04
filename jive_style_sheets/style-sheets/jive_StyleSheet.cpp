#include <jive_style_sheets/jive_style_sheets.h>

namespace jive
{
    namespace fontProperties
    {
        static const juce::Identifier fontFamily{ "font-family" };
        static const juce::Identifier fontStyle{ "font-style" };
        static const juce::Identifier fontWeight{ "font-weight" };
        static const juce::Identifier fontSize{ "font-size" };
        static const juce::Identifier letterSpacing{ "letter-spacing" };
        static const juce::Identifier textDecoration{ "text-decoration" };
        static const juce::Identifier fontStretch{ "font-stretch" };

        static const juce::Array all{
            fontFamily,
            fontStyle,
            fontWeight,
            fontSize,
            letterSpacing,
            textDecoration,
            fontStretch,
        };
    } // namespace fontProperties

    juce::StringArray getAncestorTypes(const juce::ValueTree& child)
    {
        juce::StringArray types;

        for (auto parent = child.getParent();
             parent.isValid();
             parent = parent.getParent())
        {
            types.add(parent.getType().toString());
        }

        return types;
    }

    struct StyleSheet::Selectors
    {
    public:
        explicit Selectors(const juce::ValueTree& sourceState)
            : state{ sourceState }
            , enabled{ state, enabledID }
            , mouse{ state, mouseID }
            , keyboard{ state, keyboardID }
        {
            const auto informListeners = [this]() {
                if (onChange != nullptr)
                    onChange();
            };
            enabled.onValueChange = informListeners;
            mouse.onValueChange = informListeners;
            keyboard.onValueChange = informListeners;
        }

        juce::StringArray getSelectorsInOrderOfSpecificity() const
        {
            juce::StringArray result{ state.getType() };
            result.addArray(getAncestorTypes(state));

            if (!enabled.get())
                result.add("disabled");
            if (keyboard == ComponentInteractionState::Keyboard::focus)
                result.add("focus");
            if (mouse == ComponentInteractionState::Mouse::active)
                result.add("active");
            if (mouse != ComponentInteractionState::Mouse::dissociate)
                result.add("hover");

            return result;
        }

        std::function<void()> onChange = nullptr;

    private:
        static const inline juce::Identifier enabledID{ "enabled" };
        static const inline juce::Identifier mouseID{ "mouse" };
        static const inline juce::Identifier keyboardID{ "keyboard" };

        juce::ValueTree state;
        Property<bool> enabled;
        Property<ComponentInteractionState::Mouse> mouse;
        Property<ComponentInteractionState::Keyboard> keyboard;
    };

    StyleSheet::StyleSheet(juce::Component& sourceComponent,
                           juce::ValueTree sourceState)
        : component{ &sourceComponent }
        , state{ sourceState }
        , stateRoot{ state.getRoot() }
        , interactionState{ sourceComponent, state }
        , style{ state, "style" }
        , borderWidth{ state, "border-width" }
        , selectors{ std::make_unique<Selectors>(state) }
    {
        jassert(component != nullptr);
        jassert(!component->getProperties().contains("style-sheet"));

        component->getProperties().set("style-sheet", juce::var{ this });
        component->addAndMakeVisible(backgroundCanvas, 0);
        backgroundCanvas.setBounds(component->getLocalBounds());

        applyStyles();

        component->addComponentListener(this);
        stateRoot.addListener(this);

        borderWidth.onValueChange = [this]() {
            backgroundCanvas.setBorderWidth(borderWidth.get());
        };

        if (auto object = style.get();
            object != nullptr)
        {
            object->addListener(*this);
        }

        selectors->onChange = [this]() {
            applyStyles();
        };
    }

    StyleSheet::~StyleSheet()
    {
        if (component != nullptr)
        {
            component->removeComponentListener(this);
            component->getProperties().remove("style-sheet");
        }

        if (auto object = style.get();
            object != nullptr)
        {
            object->removeListener(*this);
        }
    }

    Fill StyleSheet::getBackground() const
    {
        return juce::VariantConverter<Fill>::fromVar(findStyleProperty("background"));
    }

    Fill StyleSheet::getForeground() const
    {
        return juce::VariantConverter<Fill>::fromVar(findHierarchicalStyleProperty("foreground"));
    }

    Fill StyleSheet::getBorderFill() const
    {
        return juce::VariantConverter<Fill>::fromVar(findStyleProperty("border"));
    }

    BorderRadii<float> StyleSheet::getBorderRadii() const
    {
        return juce::VariantConverter<BorderRadii<float>>::fromVar(findStyleProperty("border-radius"));
    }

    juce::Font StyleSheet::getFont() const
    {
        juce::Font font;

        if (const auto fontFamily = findHierarchicalStyleProperty(fontProperties::fontFamily).toString();
            fontFamily.isNotEmpty())
        {
            font.setTypefaceName(fontFamily);
        }

        if (const auto fontStyle = findHierarchicalStyleProperty(fontProperties::fontStyle).toString();
            fontStyle.isNotEmpty())
        {
            font.setItalic(fontStyle.compareIgnoreCase("italic") == 0);
        }

        if (const auto weight = findHierarchicalStyleProperty(fontProperties::fontWeight).toString();
            weight.isNotEmpty())
        {
            font.setBold(weight.compareIgnoreCase("bold") == 0);
        }

        if (const auto size = findHierarchicalStyleProperty(fontProperties::fontSize);
            size != juce::var{})
        {
            font = font.withPointHeight(static_cast<float>(size));
        }

        if (const auto spacing = findHierarchicalStyleProperty(fontProperties::letterSpacing);
            spacing != juce::var{})
        {
            const auto extraKerning = static_cast<float>(spacing) / font.getHeight();
            font.setExtraKerningFactor(extraKerning);
        }

        if (const auto decoration = findHierarchicalStyleProperty(fontProperties::textDecoration).toString();
            decoration.isNotEmpty())
        {
            font.setUnderline(decoration.compareIgnoreCase("underlined") == 0);
        }

        if (const auto stretch = findHierarchicalStyleProperty(fontProperties::fontStretch);
            stretch != juce::var{})
        {
            font.setHorizontalScale(static_cast<float>(stretch));
        }

        return font;
    }

    void StyleSheet::componentMovedOrResized(juce::Component& componentThatWasMovedOrResized,
                                             bool /*wasMoved*/,
                                             bool /*wasResized*/)
    {
        jassertquiet(&componentThatWasMovedOrResized == component);
        backgroundCanvas.setBounds(component->getLocalBounds());
    }

    void StyleSheet::componentParentHierarchyChanged(juce::Component& childComponent)
    {
        jassertquiet(&childComponent == component);
        applyStyles();
    }

    void StyleSheet::valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier& id)
    {
        if (id == juce::Identifier{ "style" })
        {
            if (auto object = style.get();
                object != nullptr)
            {
                object->addListener(*this);
            }

            applyStyles();
        }
    }

    void StyleSheet::propertyChanged(Object& object, const juce::Identifier&)
    {
        jassertquiet(&object == style.get().get());
        applyStyles();
    }

    enum class StyleSearchStrategy
    {
        objectAndChildren,
        childrenOnly,
    };

    template <StyleSearchStrategy strategy>
    juce::var findStyleProperty(const Object& object,
                                const StyleSheet::Selectors& selectors,
                                const juce::Identifier& propertyName)
    {
        for (const auto& selector : selectors.getSelectorsInOrderOfSpecificity())
        {
            if (object.hasProperty(selector))
            {
                const auto& nested = dynamic_cast<Object&>(*object
                                                                .getProperty(selector)
                                                                .getDynamicObject());
                const auto value = findStyleProperty<StyleSearchStrategy::objectAndChildren>(nested,
                                                                                             selectors,
                                                                                             propertyName);

                if (value != juce::var{})
                    return value;
            }
        }

        if constexpr (strategy == StyleSearchStrategy::objectAndChildren)
            return object.getProperty(propertyName);
        else
            return juce::var{};
    }

    template <StyleSearchStrategy strategy>
    juce::var findStyleProperty(const juce::ValueTree& state,
                                const StyleSheet::Selectors& selectors,
                                const juce::Identifier& propertyName)
    {
        jassert(state.isValid());

        if (auto object = Property<Object::ReferenceCountedPointer>{ state, "style" }.get())
        {
            return findStyleProperty<strategy>(*object.get(),
                                               selectors,
                                               propertyName);
        }

        return juce::var{};
    }

    juce::var StyleSheet::findStyleProperty(const juce::Identifier& propertyName) const
    {
        if (auto value = ::jive::findStyleProperty<StyleSearchStrategy::objectAndChildren>(state,
                                                                                           *selectors,
                                                                                           propertyName);
            value != juce::var{})
        {
            return value;
        }

        for (auto stateToSearch = state.getParent();
             stateToSearch.isValid();
             stateToSearch = stateToSearch.getParent())
        {
            if (auto value = ::jive::findStyleProperty<StyleSearchStrategy::childrenOnly>(stateToSearch,
                                                                                          *selectors,
                                                                                          propertyName);
                value != juce::var{})
            {
                return value;
            }
        }

        return {};
    }

    juce::var StyleSheet::findHierarchicalStyleProperty(const juce::Identifier& propertyName) const
    {
        for (juce::ReferenceCountedObjectPtr<StyleSheet> styleSheetToSearch{ const_cast<StyleSheet*>(this) };
             styleSheetToSearch != nullptr;
             styleSheetToSearch = styleSheetToSearch->findClosestAncestorStyleSheet())
        {
            if (auto value = ::jive::findStyleProperty<StyleSearchStrategy::objectAndChildren>(styleSheetToSearch->state,
                                                                                               *styleSheetToSearch->selectors,
                                                                                               propertyName);
                value != juce::var{})
            {
                return value;
            }
        }

        return {};
    }

    juce::ReferenceCountedObjectPtr<StyleSheet> StyleSheet::findClosestAncestorStyleSheet()
    {
        for (auto* parent = component->getParentComponent();
             parent != nullptr;
             parent = parent->getParentComponent())
        {
            if (auto& properties = parent->getProperties();
                properties.contains("style-sheet"))
            {
                return dynamic_cast<StyleSheet*>(properties["style-sheet"].getObject());
            }
        }

        return nullptr;
    }

    juce::Array<StyleSheet::ReferenceCountedPointer> StyleSheet::collectChildSheets()
    {
        juce::Array<ReferenceCountedPointer> result;

        for (auto* const child : component->getChildren())
        {
            auto& properties = child->getProperties();

            if (properties.contains("style-sheet"))
                result.add(dynamic_cast<StyleSheet*>(properties["style-sheet"].getObject()));
        }

        return result;
    }

    void StyleSheet::applyStyles()
    {
        backgroundCanvas.setFill(getBackground());
        backgroundCanvas.setBorderFill(getBorderFill());
        backgroundCanvas.setBorderWidth(borderWidth.get());
        backgroundCanvas.setBorderRadii(getBorderRadii());

        const auto foregroundColour = *getForeground().getColour();
        const auto font = getFont();

        if (auto* text = dynamic_cast<TextComponent*>(component.getComponent()))
        {
            text->setTextColour(foregroundColour);
            text->setFont(font);
        }
        if (state.getType().toString().compareIgnoreCase("svg") == 0)
        {
            state.setProperty("fill",
                              "#" + foregroundColour.toDisplayString(false),
                              nullptr);
        }

        for (auto child : collectChildSheets())
            child->applyStyles();
    }
} // namespace jive

#if JIVE_UNIT_TESTS
    #include <jive_layouts/jive_layouts.h>

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
    #if JUCE_MAC
        // These tests were flakey on Windows so for now, only run on macOS
        testBackgroundColour();
        testLinearBackgroundGradient();
        testTypeStyling();
    #endif

        testFont();
    }

private:
    void testBackgroundColour()
    {
        beginTest("background colour");

        juce::Component component;
        juce::ValueTree state{ "Component" };
        jive::StyleSheet::ReferenceCountedPointer styleSheet = new jive::StyleSheet{ component, state };
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
        jive::StyleSheet::ReferenceCountedPointer styleSheet = new jive::StyleSheet{ component, state };
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
        jive::StyleSheet::ReferenceCountedPointer styleSheet = new jive::StyleSheet{ component, state };

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

    void testFont()
    {
        beginTest("font");

        juce::ValueTree state{
            "Component",
            {
                { "width", 100 },
                { "height", 100 },
            },
            {
                juce::ValueTree{
                    "Text",
                    {
                        {
                            "style",
                            R"({})",
                        },
                    },
                },
            },
        };
        jive::Interpreter interpreter;
        auto item = interpreter.interpret(state);
        auto& text = dynamic_cast<jive::TextComponent&>(*item->getChildren()[0]->getComponent());
        expectEquals(text.getFont(), juce::Font{});

        jive::StyleSheet::ReferenceCountedPointer styleSheet = new jive::StyleSheet{ text, state.getChild(0) };
        jive::Property<jive::Object::ReferenceCountedPointer> style{ state.getChild(0), "style" };
        juce::Font expected;
        expectEquals(text.getFont(), expected);

        style.get()->setProperty("font-family", "Helvetica");
        expected.setTypefaceName("Helvetica");
        expectEquals(text.getFont(), expected);

        style.get()->setProperty("font-style", "italic");
        expected.setItalic(true);
        expectEquals(text.getFont(), expected);

        style.get()->setProperty("font-weight", "bold");
        expected.setBold(true);
        expectEquals(text.getFont(), expected);

        style.get()->setProperty("font-size", 123.45f);
        expected = expected.withPointHeight(123.45f);
        expectEquals(text.getFont(), expected);

        style.get()->setProperty("letter-spacing", 5.3f);
        expected.setExtraKerningFactor(5.3f / expected.getHeight());
        expectEquals(text.getFont(), expected);

        style.get()->setProperty("text-decoration", "underlined");
        expected.setUnderline(true);
        expectEquals(text.getFont(), expected);

        style.get()->setProperty("font-stretch", 0.381f);
        expected.setHorizontalScale(0.381f);
        expectEquals(text.getFont(), expected);
    }
};

static StyleSheetTest styleSheetTest;
#endif
