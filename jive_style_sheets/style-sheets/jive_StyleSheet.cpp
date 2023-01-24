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
        , borderWidth{ state, "border-width" }
    {
        jassert(component != nullptr);

        applyStyles();

        component->addAndMakeVisible(backgroundCanvas, 0);
        backgroundCanvas.setBounds(component->getLocalBounds());

        component->addComponentListener(this);
        component->addMouseListener(this, true);
        stateRoot.addListener(this);

        borderWidth.onValueChange = [this]() {
            backgroundCanvas.setBorderWidth(borderWidth.get());
        };

        if (auto object = style.get();
            object != nullptr)
        {
            object->addListener(*this);
        }
    }

    StyleSheet::~StyleSheet()
    {
        if (component != nullptr)
        {
            component->removeComponentListener(this);
            component->removeMouseListener(this);
        }

        if (auto object = style.get();
            object != nullptr)
        {
            object->removeListener(*this);
        }
    }

    //==================================================================================================================
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
        applyStyles();
    }

    void StyleSheet::mouseExit(const juce::MouseEvent&)
    {
        applyStyles();
    }

    void StyleSheet::mouseDown(const juce::MouseEvent&)
    {
        applyStyles();
    }

    void StyleSheet::mouseUp(const juce::MouseEvent&)
    {
        applyStyles();
    }

    void StyleSheet::valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier& id)
    {
        if (id == juce::Identifier{ "style" })
        {
            if (style.exists())
                style.get()->addListener(*this);

            applyStyles();
        }
    }

    void StyleSheet::propertyChanged(Object& object, const juce::Identifier&)
    {
        jassert(&object == style.get().get());
        applyStyles();
    }

    //==================================================================================================================
    juce::StringArray StyleSheet::findApplicableSelectorsInOrderOfSpecificity() const
    {
        juce::StringArray selectors;

        if (!component->isEnabled())
            selectors.add("disabled");

        if (component->hasKeyboardFocus(false))
            selectors.add("focus");

        if (const auto mousePosition = component->getMouseXYRelative();
            const_cast<BackgroundCanvas&>(backgroundCanvas).hitTest(mousePosition.x, mousePosition.y))
        {
            if (component->isMouseButtonDown(true))
                selectors.add("active");

            if (component->isMouseOverOrDragging(true))
                selectors.add("hover");
        }

        return selectors;
    }

    enum class StyleSearchStrategy
    {
        objectAndChildren,
        childrenOnly,
    };

    template <StyleSearchStrategy strategy>
    juce::var findStyleProperty(const Object& object,
                                const juce::StringArray& selectorsInOrderOfSpecificity,
                                const juce::Identifier& propertyName)
    {
        for (const auto& selector : selectorsInOrderOfSpecificity)
        {
            if (object.hasProperty(selector))
            {
                const auto& nested = dynamic_cast<Object&>(*object
                                                                .getProperty(selector)
                                                                .getDynamicObject());
                const auto value = findStyleProperty<StyleSearchStrategy::objectAndChildren>(nested,
                                                                                             selectorsInOrderOfSpecificity,
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
                                const juce::StringArray& selectorsInOrderOfSpecificity,
                                const juce::Identifier& propertyName)
    {
        jassert(state.isValid());

        if (auto object = Property<Object::ReferenceCountedPointer>{ state, "style" }.get())
        {
            return findStyleProperty<strategy>(*object.get(),
                                               selectorsInOrderOfSpecificity,
                                               propertyName);
        }

        return juce::var{};
    }

    juce::var StyleSheet::findStyleProperty(const juce::Identifier& propertyName) const
    {
        juce::StringArray applicableSelectors;
        applicableSelectors.add(state.getType().toString());
        applicableSelectors.addArray(findApplicableSelectorsInOrderOfSpecificity());

        if (auto value = ::jive::findStyleProperty<StyleSearchStrategy::objectAndChildren>(state,
                                                                                           applicableSelectors,
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
                                                                                          applicableSelectors,
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
        juce::StringArray applicableSelectors;

        for (auto currentState = state;
             currentState.isValid();
             currentState = currentState.getParent())
        {
            applicableSelectors.add(currentState.getType().toString());
        }

        applicableSelectors.addArray(findApplicableSelectorsInOrderOfSpecificity());

        for (auto stateToSearch = state;
             stateToSearch.isValid();
             stateToSearch = stateToSearch.getParent())
        {
            if (auto value = ::jive::findStyleProperty<StyleSearchStrategy::objectAndChildren>(stateToSearch,
                                                                                               applicableSelectors,
                                                                                               propertyName);
                value != juce::var{})
            {
                return value;
            }
        }

        return {};
    }

    //==================================================================================================================
    void StyleSheet::applyStyles()
    {
        backgroundCanvas.setFill(getBackground());
        backgroundCanvas.setBorderFill(getBorderFill());
        backgroundCanvas.setBorderWidth(borderWidth.get());
        backgroundCanvas.setBorderRadii(getBorderRadii());
        backgroundCanvas.repaint();

        if (auto* text = dynamic_cast<TextComponent*>(component.getComponent()))
            text->setTextFill(getForeground());
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
};

static StyleSheetTest styleSheetTest;
#endif
