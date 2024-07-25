#include "jive_StyleSheet.h"

namespace jive
{
    StyleSheet::ReferenceCountedPointer StyleSheet::create(juce::Component& sourceComponent,
                                                           juce::ValueTree sourceState)
    {
        return new StyleSheet{ sourceComponent, sourceState };
    }

    StyleSheet::StyleSheet(juce::Component& sourceComponent,
                           juce::ValueTree sourceState)
        : component{ &sourceComponent }
        , state{ sourceState }
        , style{ state, "style" }
        , selectors{ state }
#if !JIVE_UNIT_TESTS
        , interactionState{ *component, state }
#endif
        , borderWidth{ state, "border-width" }
    {
        jassert(component != nullptr);
        jassert(!component->getProperties().contains("style-sheet"));

        component->getProperties().set("style-sheet", this);
        component->addAndMakeVisible(backgroundCanvas, 0);
        backgroundCanvas.setBounds(component->getLocalBounds());
        backgroundCanvas.setBorderWidth(borderWidth);

        updateClosestAncestor();
        updateStyles();

        style.onValueChange = [this] {
            updateStyles();
        };
        selectors.onChange = [this] {
            applyStyles();
        };

        const auto updateBorderWidth = [this] {
            backgroundCanvas.setBorderWidth(borderWidth.calculateCurrent());
        };
        borderWidth.onValueChange = updateBorderWidth;
        borderWidth.onTransitionProgressed = updateBorderWidth;

        component->addComponentListener(this);
    }

    StyleSheet::~StyleSheet()
    {
        if (component != nullptr)
        {
            component->removeComponentListener(this);
            component->getProperties().remove("style-sheet");
        }

        if (closestAncestor != nullptr)
            closestAncestor->dependants.removeAllInstancesOf(this);
    }

    Fill StyleSheet::getBackground() const
    {
        if (auto* background = selectors.findStyle(backgroundStyles))
        {
            if (calculatedBackground != nullptr)
            {
                *calculatedBackground = background->get();
                return calculatedBackground->calculateCurrent();
            }

            return background->calculateCurrent();
        }

        return {};
    }

    Fill StyleSheet::getForeground() const
    {
        if (auto* foreground = selectors.findStyle(foregroundStyles))
        {
            if (calculatedForeground != nullptr)
            {
                *calculatedForeground = foreground->get();
                return calculatedForeground->calculateCurrent();
            }

            return foreground->calculateCurrent();
        }

        if (closestAncestor != nullptr)
            return closestAncestor->getForeground();

        return {};
    }

    Fill StyleSheet::getBorderFill() const
    {
        if (auto* borderFill = selectors.findStyle(borderFillStyles))
        {
            if (calculatedBorderFill != nullptr)
            {
                *calculatedBorderFill = borderFill->get();
                return calculatedBorderFill->calculateCurrent();
            }

            return borderFill->calculateCurrent();
        }

        return {};
    }

    BorderRadii<float> StyleSheet::getBorderRadii() const
    {
        if (auto* borderRadii = selectors.findStyle(borderRadiiStyles))
        {
            if (calculatedBorderRadii != nullptr)
            {
                *calculatedBorderRadii = borderRadii->get();
                return calculatedBorderRadii->calculateCurrent();
            }

            return borderRadii->calculateCurrent();
        }

        return {};
    }

    juce::Font StyleSheet::getFont() const
    {
        juce::Font font{
#if JUCE_MAJOR_VERSION >= 8
            juce::FontOptions{},
#endif
        };

        font.setTypefaceName(getFontFamily());

        if (juce::Font::getDefaultTypefaceForFont(font) == nullptr)
            return font;

        font.setItalic(getFontStyle() == "italic");
        font.setBold(getFontWeight() == "bold");
        font = font.withPointHeight(getFontSize());
        font.setExtraKerningFactor(getLetterSpacing() / font.getHeight());
        font.setUnderline(getTextDecoration() == "underlined");
        font.setHorizontalScale(getFontStretch());

        return font;
    }

    void StyleSheet::componentParentHierarchyChanged(juce::Component& comp)
    {
        jassertquiet(&comp == component.getComponent());
        updateClosestAncestor();
        applyStyles();
    }

    void StyleSheet::componentMovedOrResized(juce::Component& comp, bool, bool resized)
    {
        jassertquiet(&comp == component.getComponent());

        if (!resized)
            return;

        backgroundCanvas.setBounds(component->getLocalBounds());
    }

    juce::String StyleSheet::getFontFamily() const
    {
        if (auto* family = selectors.findStyle(fontFamilyStyles))
            return family->toString();

        if (closestAncestor != nullptr)
            return closestAncestor->getFontFamily();

#if JUCE_MAJOR_VERSION >= 8
        return juce::Font::getSystemUIFontName();
#else
        return juce::Font::getDefaultSansSerifFontName();
#endif
    }

    float StyleSheet::getFontSize() const
    {
        if (auto* size = selectors.findStyle(fontSizeStyles))
        {
            if (calculatedFontSize != nullptr)
            {
                *calculatedFontSize = size->get();
                return calculatedFontSize->calculateCurrent();
            }

            return size->calculateCurrent();
        }

        if (closestAncestor != nullptr)
            return closestAncestor->getFontSize();

        return 14.0f;
    }

    float StyleSheet::getFontStretch() const
    {
        if (auto* stretch = selectors.findStyle(fontStretchStyles))
        {
            if (calculatedFontStretch != nullptr)
            {
                *calculatedFontStretch = stretch->get();
                return calculatedFontStretch->calculateCurrent();
            }

            return stretch->calculateCurrent();
        }

        if (closestAncestor != nullptr)
            return closestAncestor->getFontStretch();

        return 1.0f;
    }

    juce::String StyleSheet::getFontStyle() const
    {
        if (auto* fontStyle = selectors.findStyle(fontStyleStyles))
            return fontStyle->toString();

        if (closestAncestor != nullptr)
            return closestAncestor->getFontStyle();

        return "normal";
    }

    juce::String StyleSheet::getFontWeight() const
    {
        if (auto* weight = selectors.findStyle(fontWeightStyles))
            return weight->toString();

        if (closestAncestor != nullptr)
            return closestAncestor->getFontWeight();

        return "normal";
    }

    float StyleSheet::getLetterSpacing() const
    {
        if (auto* spacing = selectors.findStyle(letterSpacingStyles))
        {
            if (calculatedLetterSpacing != nullptr)
            {
                *calculatedLetterSpacing = spacing->get();
                return calculatedLetterSpacing->calculateCurrent();
            }

            return spacing->calculateCurrent();
        }

        if (closestAncestor != nullptr)
            return closestAncestor->getLetterSpacing();

        return 0.0f;
    }

    juce::String StyleSheet::getTextDecoration() const
    {
        if (auto* decoration = selectors.findStyle(textDecorationStyles))
            return decoration->toString();

        if (closestAncestor != nullptr)
            return closestAncestor->getTextDecoration();

        return "normal";
    }

    [[nodiscard]] static StyleSheet* findClosestAncestorStyleSheet(const juce::Component& rootComponent)
    {
        for (auto* parent = rootComponent.getParentComponent();
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

    void StyleSheet::updateClosestAncestor()
    {
        if (closestAncestor != nullptr)
            closestAncestor->dependants.removeAllInstancesOf(this);

        if (component == nullptr)
        {
            closestAncestor = nullptr;
            return;
        }

        closestAncestor = findClosestAncestorStyleSheet(*component);

        if (closestAncestor != nullptr)
            closestAncestor->dependants.add(this);
    }

    void StyleSheet::updateStyles(jive::Object& source, StyleIdentifier styleID)
    {
        const auto appendStyle = [&](const auto& styleProperty, auto& styles) {
            using PropertyType = std::remove_reference_t<decltype(styles[styleID])>;

            if (source.hasProperty(styleProperty) && styles.find(styleID) == std::end(styles))
            {
                styles.insert(std::make_pair(styleID, PropertyType{ &source, styleProperty }));
                styles.at(styleID).onValueChange = [this] {
                    applyStyles();
                };
                styles.at(styleID).onTransitionProgressed = [this] {
                    applyStyles();
                };
            }
        };
        appendStyle("background", backgroundStyles);
        appendStyle("foreground", foregroundStyles);
        appendStyle("border", borderFillStyles);
        appendStyle("border-radius", borderRadiiStyles);
        appendStyle("font-family", fontFamilyStyles);
        appendStyle("font-size", fontSizeStyles);
        appendStyle("font-stretch", fontStretchStyles);
        appendStyle("font-style", fontStyleStyles);
        appendStyle("font-weight", fontWeightStyles);
        appendStyle("letter-spacing", letterSpacingStyles);
        appendStyle("text-decoration", textDecorationStyles);

        const auto appendNestedStyles = [&](const auto& property, auto&& updateNewID) {
            if (auto* child = dynamic_cast<jive::Object*>(source[property].getObject()))
            {
                auto newID = styleID;
                updateNewID(newID);

                updateStyles(*child, newID);
            }
        };

        if (selectors.id.exists())
        {
            appendNestedStyles(selectors.id.toString(),
                               [&](auto& newID) {
                                   newID.id = selectors.id.get();
                               });
        }

        for (const auto& className : selectors.classes.get())
        {
            appendNestedStyles(className,
                               [&](auto& newID) {
                                   newID.className = className;
                               });
        }

        appendNestedStyles(selectors.state.getType().toString(),
                           [&](auto& newID) {
                               newID.type = selectors.state.getType().toString();
                           });
        appendNestedStyles("disabled",
                           [&](auto& newID) {
                               newID.enabled = false;
                           });
        appendNestedStyles("focus",
                           [&](auto& newID) {
                               newID.keyboard = ComponentInteractionState::Keyboard::focus;
                           });
        appendNestedStyles("active",
                           [&](auto& newID) {
                               newID.mouse = ComponentInteractionState::Mouse::active;
                           });
        appendNestedStyles("hover",
                           [&](auto& newID) {
                               newID.mouse = ComponentInteractionState::Mouse::hover;
                           });
        appendNestedStyles("checked",
                           [&](auto& newID) {
                               newID.toggled = true;
                           });
    }

    void StyleSheet::updateStyles()
    {
        backgroundStyles.clear();
        foregroundStyles.clear();
        borderFillStyles.clear();
        borderRadiiStyles.clear();
        fontFamilyStyles.clear();
        fontSizeStyles.clear();
        fontStretchStyles.clear();
        fontStyleStyles.clear();
        fontWeightStyles.clear();
        letterSpacingStyles.clear();
        textDecorationStyles.clear();

        if (auto styleState = style.get(); styleState != nullptr)
        {
            updateStyles(*styleState, StyleIdentifier{});

            const auto onTransitionProgressed = [this] {
                applyStyles();
            };

            calculatedBackground = std::make_unique<Property<Fill>>(styleState, "calculated-background");
            calculatedBackground->setTransitionSourceProperty("background");
            calculatedBackground->onTransitionProgressed = onTransitionProgressed;

            calculatedForeground = std::make_unique<Property<Fill>>(styleState, "calculated-foreground");
            calculatedForeground->setTransitionSourceProperty("foreground");
            calculatedForeground->onTransitionProgressed = onTransitionProgressed;

            calculatedBorderFill = std::make_unique<Property<Fill>>(styleState, "calculated-border");
            calculatedBorderFill->setTransitionSourceProperty("border");
            calculatedBorderFill->onTransitionProgressed = onTransitionProgressed;

            calculatedBorderRadii = std::make_unique<Property<BorderRadii<float>>>(styleState, "calculated-border-radius");
            calculatedBorderRadii->setTransitionSourceProperty("border-radius");
            calculatedBorderRadii->onTransitionProgressed = onTransitionProgressed;

            calculatedFontSize = std::make_unique<Property<float>>(styleState, "calculated-font-size");
            calculatedFontSize->setTransitionSourceProperty("font-size");
            calculatedFontSize->onTransitionProgressed = onTransitionProgressed;

            calculatedFontStretch = std::make_unique<Property<float>>(styleState, "calculated-font-stretch");
            calculatedFontStretch->setTransitionSourceProperty("font-stretch");
            calculatedFontStretch->onTransitionProgressed = onTransitionProgressed;

            calculatedLetterSpacing = std::make_unique<Property<float>>(styleState, "calculated-letter-spacing");
            calculatedLetterSpacing->setTransitionSourceProperty("letter-spacing");
            calculatedLetterSpacing->onTransitionProgressed = onTransitionProgressed;
        }

        applyStyles();
    }

    void StyleSheet::applyStyles()
    {
        backgroundCanvas.setFill(getBackground());
        backgroundCanvas.setBorderFill(getBorderFill());
        backgroundCanvas.setBorderRadii(getBorderRadii());

        const auto foreground = getForeground();

        if (auto* text = dynamic_cast<TextComponent*>(component.getComponent()))
        {
            // TextComponent uses `juce::AttributedString` which doesn't
            // currently support anything other than solid colours!
            jassert(foreground.getColour().has_value());
            text->setTextColour(foreground.getColour().value_or(juce::Colours::hotpink));
            text->setFont(getFont());
        }
        if (state.getType().toString().compareIgnoreCase("svg") == 0)
        {
            state.setProperty("fill",
                              "#" + foreground.getColour()->toDisplayString(false),
                              nullptr);
        }

        for (auto* dependant : dependants)
            dependant->applyStyles();
    }
} // namespace jive

#if JIVE_UNIT_TESTS
    #include <jive_layouts/jive_layouts.h>

class StyleSheetTest : public juce::UnitTest
{
public:
    StyleSheetTest()
        : juce::UnitTest{ "jive::StyleSheet", "jive" }
    {
    }

    void runTest() final
    {
        testFindingStylesInLocalStyleSheet();
        testFindingStylesInParentStyleSheets();
        testChangingStylesDuringRuntime();
    }

private:
    [[nodiscard]] jive::BackgroundCanvas* findCanvas(const juce::Component& component)
    {
        for (auto* child : component.getChildren())
        {
            if (auto* canvas = dynamic_cast<jive::BackgroundCanvas*>(child))
                return canvas;
        }

        return nullptr;
    }

    void testFindingStylesInLocalStyleSheet()
    {
        beginTest("finding styles locally / no style");
        {
            juce::Component component;
            juce::ValueTree state{ "Component" };
            const auto styleSheet = jive::StyleSheet::create(component, state);
            expectEquals(styleSheet->getBackground(),
                         jive::Fill{});
            expectEquals(findCanvas(component)->getFill(), jive::Fill{});
        }

        beginTest("finding styles locally / basic");
        {
            juce::Component component;
            juce::ValueTree state{
                "Component",
                {
                    { "style", new jive::Object{ { "background", "#123456" } } },
                },
            };
            const auto styleSheet = jive::StyleSheet::create(component, state);
            expectEquals(styleSheet->getBackground(),
                         jive::Fill{ juce::Colour{ 0xFF123456 } });
            expectEquals(findCanvas(component)->getFill(),
                         jive::Fill{ juce::Colour{ 0xFF123456 } });
        }

        beginTest("finding styles locally / selectors");
        {
            juce::Component component;
            juce::ValueTree state{
                "Component",
                {
                    { "mouse", "hover" },
                    { "style", new jive::Object{ { "background", "#ABCDEF" } } },
                },
            };
            const auto styleSheet = jive::StyleSheet::create(component, state);
            expectEquals(styleSheet->getBackground(),
                         jive::Fill{ juce::Colour{ 0xFFABCDEF } });
            expectEquals(findCanvas(component)->getFill(),
                         jive::Fill{ juce::Colour{ 0xFFABCDEF } });

            state.setProperty("style",
                              new jive::Object{
                                  { "background", "#ABCDEF" },
                                  {
                                      "hover",
                                      new jive::Object{
                                          { "background", "#FEDCBA" },
                                      },
                                  },
                              },
                              nullptr);
            expectEquals(styleSheet->getBackground(),
                         jive::Fill{ juce::Colour{ 0xFFFEDCBA } });
            expectEquals(findCanvas(component)->getFill(),
                         jive::Fill{ juce::Colour{ 0xFFFEDCBA } });

            state.setProperty("style",
                              new jive::Object{
                                  { "background", "#ABCDEF" },
                                  {
                                      "hover",
                                      new jive::Object{
                                          { "background", "#FEDCBA" },
                                      },
                                  },
                                  {
                                      "disabled",
                                      new jive::Object{
                                          { "background", "#DEADED" },
                                      },
                                  },
                              },
                              nullptr);
            state.setProperty("enabled", false, nullptr);
            expectEquals(styleSheet->getBackground(),
                         jive::Fill{ juce::Colour{ 0xFFDEADED } });
            expectEquals(findCanvas(component)->getFill(),
                         jive::Fill{ juce::Colour{ 0xFFDEADED } });

            state.setProperty("style",
                              new jive::Object{
                                  { "background", "#ABCDEF" },
                                  {
                                      "hover",
                                      new jive::Object{
                                          { "background", "#FEDCBA" },
                                          {
                                              "disabled",
                                              new jive::Object{
                                                  { "background", "#666666" },
                                              },
                                          },
                                      },
                                  },
                                  {
                                      "disabled",
                                      new jive::Object{
                                          { "background", "#DEADED" },
                                          {
                                              "hover",
                                              new jive::Object{
                                                  { "background", "#333333" },
                                              },
                                          },
                                      },
                                  },
                              },
                              nullptr);
            expect(styleSheet->getBackground() == jive::Fill{ juce::Colour{ 0xFF333333 } }
                   || styleSheet->getBackground() == jive::Fill{ juce::Colour{ 0xFF666666 } });
            expect(findCanvas(component)->getFill() == jive::Fill{ juce::Colour{ 0xFF333333 } }
                   || findCanvas(component)->getFill() == jive::Fill{ juce::Colour{ 0xFF666666 } });
        }
    }

    void testFindingStylesInParentStyleSheets()
    {
        beginTest("finding styles among parents / no style");
        {
            jive::TextComponent component;
            juce::ValueTree state{ "Component" };
            const auto styleSheet = jive::StyleSheet::create(component, state);
            expectEquals(styleSheet->getForeground(),
                         jive::Fill{});
            expectEquals(component.getTextColour(), juce::Colours::transparentBlack);
        }

        beginTest("finding styles among parents / basic");
        {
            juce::Component parent;
            jive::TextComponent component;
            parent.addChildComponent(component);
            juce::ValueTree parentState{
                "Component",
                {
                    { "style", new jive::Object{ { "foreground", "#777444" } } },
                },
                {
                    juce::ValueTree{ "Component" },
                },
            };
            const auto parentStyleSheet = jive::StyleSheet::create(parent, parentState);
            const auto styleSheet = jive::StyleSheet::create(component, parentState.getChild(0));

            expectEquals(parentStyleSheet->getForeground(),
                         jive::Fill{ juce::Colour{ 0xFF777444 } });
            expectEquals(styleSheet->getForeground(),
                         jive::Fill{ juce::Colour{ 0xFF777444 } });
            expectEquals(component.getTextColour(), juce::Colour{ 0xFF777444 });
        }
    }

    void testChangingStylesDuringRuntime()
    {
        beginTest("changing styles / changing single property");
        {
            jive::TextComponent component;
            juce::ValueTree state{
                "Component",
                {
                    {
                        "style",
                        new jive::Object{
                            { "font-family", "Helvetica" },
                        },
                    },
                },
            };
            jive::Property<juce::String> fontFamily{
                dynamic_cast<jive::Object*>(state["style"].getObject()),
                "font-family",
            };
            const auto styleSheet = jive::StyleSheet::create(component, state);
            expectEquals<juce::String>(component.getFont().getTypefaceName(), "Helvetica");

            fontFamily = "Comic Sans MS";
            expectEquals<juce::String>(component.getFont().getTypefaceName(), "Comic Sans MS");
        }

        beginTest("changing styles / changing properties in selectors");
        {
            juce::Component component;
            juce::ValueTree state{
                "Component",
                {
                    { "mouse", "hover" },
                    {
                        "style",
                        new jive::Object{
                            {
                                "hover",
                                new jive::Object{
                                    { "background", "#FADDED" },
                                },
                            },
                        },
                    },
                },
            };
            jive::Property<juce::String> background{
                dynamic_cast<jive::Object*>(state["style"]["hover"].getObject()),
                "background",
            };
            const auto styleSheet = jive::StyleSheet::create(component, state);
            auto& canvas = dynamic_cast<jive::BackgroundCanvas&>(*component.getChildComponent(0));
            expectEquals(canvas.getFill(), jive::Fill{ juce::Colour{ 0xFFFADDED } });

            background = "#BED4ED";
            expectEquals(canvas.getFill(), jive::Fill{ juce::Colour{ 0xFFBED4ED } });
        }

        beginTest("changing styles / changing inherited properties");
        {
            juce::Component parentComponent;
            jive::TextComponent component;
            parentComponent.addChildComponent(component);

            juce::ValueTree parentState{
                "Component",
                {
                    {
                        "style",
                        new jive::Object{
                            { "foreground", "#DDDFFF" },
                            {
                                "hover",
                                new jive::Object{
                                    { "foreground", "#999111" },
                                },
                            },
                        },
                    },
                },
                {
                    juce::ValueTree{
                        "Text",
                    },
                },
            };
            jive::Property<juce::String> foreground{
                dynamic_cast<jive::Object*>(parentState["style"].getObject()),
                "foreground",
            };
            const auto parentStyleSheet = jive::StyleSheet::create(parentComponent, parentState);
            const auto styleSheet = jive::StyleSheet::create(component, parentState.getChild(0));
            expectEquals(component.getTextColour(), juce::Colour{ 0xFFDDDFFF });

            foreground = "#808808";
            expectEquals(component.getTextColour(), juce::Colour{ 0xFF808808 });

            parentState.setProperty("mouse", "hover", nullptr);
            expectEquals(component.getTextColour(), juce::Colour{ 0xFF999111 });
        }

        beginTest("changing styles / changing inherited style sheets");
        {
            juce::Component parentComponent;
            jive::TextComponent component;
            parentComponent.addChildComponent(component);

            juce::ValueTree parentState{
                "Component",
                {
                    {
                        "style",
                        new jive::Object{
                            { "foreground", "#012345" },
                        },
                    },
                },
                {
                    juce::ValueTree{
                        "Text",
                    },
                },
            };
            const auto parentStyleSheet = jive::StyleSheet::create(parentComponent, parentState);
            const auto styleSheet = jive::StyleSheet::create(component, parentState.getChild(0));
            expectEquals(component.getTextColour(), juce::Colour{ 0xFF012345 });

            parentState.setProperty("style",
                                    new jive::Object{
                                        { "foreground", "#ABCDEF" },
                                        {
                                            "focus",
                                            new jive::Object{
                                                { "foreground", "#775647" },
                                            },
                                        },
                                    },
                                    nullptr);
            expectEquals(component.getTextColour(), juce::Colour{ 0xFFABCDEF });

            parentState.setProperty("keyboard", "focus", nullptr);
            expectEquals(component.getTextColour(), juce::Colour{ 0xFF775647 });
        }
    }
};

static StyleSheetTest styleSheetTest;
#endif
