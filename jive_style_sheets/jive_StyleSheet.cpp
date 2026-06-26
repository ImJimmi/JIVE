#include "jive_StyleSheet.h"

namespace jive
{
    StyleSheet::ReferenceCountedPointer StyleSheet::create(juce::Component& component,
                                                           const juce::ValueTree& source)
    {
        if (auto* jiveLookAndFeel = dynamic_cast<LookAndFeel*>(&component.getLookAndFeel()))
            return new StyleSheet{ component, *jiveLookAndFeel, source };

        return nullptr;
    }

    StyleSheet::StyleSheet(juce::Component& comp,
                           LookAndFeel& laf,
                           const juce::ValueTree& source)
        : component{ comp }
        , lookAndFeel{ laf }
        , state{ source }
        , style{ state, "style" }
        , border{ state, "border-width" }
        , sourceDirectories{ state, "jive::source-directories" }
    {
        style.onValueChange = [this] {
            stylePropertyChanged();
        };
        stylePropertyChanged();

        const auto updateBorderWidth = [this] {
            // Remove the previously-added border-width styler from the
            // look-and-feel before forgetting its ID, otherwise stale stylers
            // accumulate and an old border-width keeps being applied.
            if (const auto existing = uuids.find("border-width");
                existing != std::end(uuids))
            {
                lookAndFeel.removeStyles(existing->second);
                uuids.erase(existing);
            }

            if (border.exists())
            {
                uuids["border-width"] = lookAndFeel
                                            .addStyles(
                                                StyleSelector::makeInlineStyleSelector(component),
                                                Styles{}
                                                    .withBorderWidth(border.calculateCurrent()));
            }

            component.lookAndFeelChanged();
            component.repaint();
        };
        border.onValueChange = updateBorderWidth;
        border.onTransitionProgressed = updateBorderWidth;
        updateBorderWidth();
    }

    StyleSheet::~StyleSheet()
    {
        if (&component.getLookAndFeel() == &lookAndFeel)
        {
            for (const auto& [name, uuid] : uuids)
                lookAndFeel.removeStyles(uuid);
        }

        for (const auto& [id, property] : observedObjects)
        {
            juce::ignoreUnused(id);

            if (auto object = property.get(); object != nullptr)
                object->removeListener(*this);
        }
    }

    void StyleSheet::propertyChanged(Object& object,
                                     const juce::Identifier&)
    {
        for (const auto& [id, property] : observedObjects)
        {
            if (property.get().get() == &object)
                property.onValueChange();
        }
    }

    void StyleSheet::clear()
    {
        // The border-width styler is managed separately (see updateBorderWidth
        // in the constructor) so it must survive clearing of the style-object
        // stylers, otherwise changing the "style" property would wipe a
        // border-width that was applied from the component's state.
        for (auto entry = std::begin(uuids); entry != std::end(uuids);)
        {
            if (entry->first == "border-width")
            {
                ++entry;
                continue;
            }

            lookAndFeel.removeStyles(entry->second);
            entry = uuids.erase(entry);
        }
    }

    void StyleSheet::stylePropertyChanged()
    {
        if (!style.exists())
        {
            clear();
            return;
        }

        if (style.get() != nullptr)
        {
            addStylesFrom(*style.get());
            return;
        }

        if (sourceDirectories.get() != nullptr)
        {
            if (auto file = sourceDirectories.get()->find(style.toString());
                file.existsAsFile())
            {
                const auto addStylesFromFile = [this, f = file]() {
                    auto fileContents = parseJSON(f.loadFileAsString());

                    if (auto* object = dynamic_cast<Object*>(fileContents.getObject()))
                        addStylesFrom(*object);
                };

                fileObserver = std::make_unique<FileObserver>(file);
                fileObserver->onFileModified = addStylesFromFile;
                addStylesFromFile();
            }
        }
    }

    void StyleSheet::addStylesFrom(Object& object)
    {
        clear();
        addInlineStyles(object);
        addSelectorBasedStyles(object);

        component.lookAndFeelChanged();
        component.repaint();
    }

    static void appendStyleProperties(Object& object,
                                      std::optional<Styles>& styles)
    {
        for (const auto& propertyName : Styles::propertyNames)
        {
            if (!object.hasProperty(propertyName))
                continue;

            styles = styles
                         .value_or(Styles{})
                         .with(propertyName, object.getProperty(propertyName));
        }
    }

    void StyleSheet::addInlineStyles(Object& object)
    {
        std::optional<Styles> styles;
        appendStyleProperties(object, styles);

        if (&object == style.get().get() && object.hasProperty("transition"))
        {
            auto transitions = fromVar<Transitions::ReferenceCountedPointer>(object["transition"]);

            if (transitions != nullptr)
            {
                styles = styles
                             .value_or(Styles{})
                             .withTransitions(transitions);
            }
        }

        if (styles.has_value())
        {
            uuids[""] = lookAndFeel.addStyles(
                StyleSelector::makeInlineStyleSelector(component),
                *styles);
        }
    }

    void StyleSheet::addSelectorBasedStyles(Object& object)
    {
        static constexpr auto getObjectIfNestedStyleObject = [](const juce::Identifier& name,
                                                                const juce::var& value) -> Object* {
            if (Styles::propertyNames.count(name.toString()) == 0)
                return dynamic_cast<Object*>(value.getObject());

            return nullptr;
        };
        std::function<void(Object&, const juce::String&)> appendFromChild;
        appendFromChild = [this, &appendFromChild](Object& child,
                                                   const juce::String& selector) {
            std::optional<Styles> styles;
            appendStyleProperties(child, styles);

            if (styles.has_value())
                uuids[selector] = lookAndFeel.addStyles(StyleSelector{ selector, &component }, *styles);

            for (const auto& [name, value] : child.getProperties())
            {
                if (auto* nested = getObjectIfNestedStyleObject(name, value))
                    appendFromChild(*nested, selector + " " + name.toString());
            }
        };

        for (const auto& [name, value] : object.getProperties())
        {
            if (auto* child = getObjectIfNestedStyleObject(name, value))
                appendFromChild(*child, name.toString());
        }
    }
} // namespace jive

#if JIVE_UNIT_TESTS
    #include <jive_core/graphics/themes/jive_SteelTheme.h>

class StyleSheetTests : public juce::UnitTest
{
public:
    StyleSheetTests()
        : juce::UnitTest{ "jive::StyleSheet", "jive" }
    {
    }

    void runTest() final
    {
        testCreation();
        testStyleProperty();
        testIndividualStyleProperties();
        testTransitions();
        testTransitionDetails();
        testSelectorBasedStyles();
        testMultiTierSelectors();
        testDynamicStyles();
        testSelectorEdgeCases();
        testInheritance();
        testMiscStyleProperties();
        testFileBasedStyles();
    }

private:
    // The default theme applies a 200ms transition to background, border, and
    // foreground, so whenever a resolved style actually changes the component
    // animates towards the new value rather than snapping to it instantly.
    // These tests are concerned with which styles end up applied (i.e. selector
    // resolution) rather than the transition itself, so this helper paints once
    // to commence any pending transition, advances time well past the
    // transition's duration, then returns the colour at the given point once
    // everything has settled on its target value.
    static juce::Colour settledColourAt(juce::Component& component, int x, int y)
    {
        component.createComponentSnapshot(component.getLocalBounds());
        jive::FakeTime::incrementTime(juce::RelativeTime::seconds(1.0));
        return component
            .createComponentSnapshot(component.getLocalBounds())
            .getPixelAt(x, y);
    }

    void testCreation()
    {
        beginTest("creation / style sheets can only be created for components using the JIVE look-and-feel");
        {
            juce::Component component;
            juce::ValueTree state;
            auto sheet = jive::StyleSheet::create(component, state);
            expect(sheet == nullptr);

            jive::LookAndFeel lookAndFeel{ component };
            sheet = jive::StyleSheet::create(component, state);
            expect(sheet != nullptr);
        }
    }

    void testStyleProperty()
    {
        beginTest("style property / with no style property specified, the component should use the default styles");
        {
            juce::TextButton button;
            button.setSize(100, 25);
            juce::ValueTree state{ "Button" };
            jive::LookAndFeel lookAndFeel{ button };
            auto sheet = jive::StyleSheet::create(button, state);

            const auto snapshot = button.createComponentSnapshot(button.getLocalBounds());
            expectEquals(snapshot.getPixelAt(50, 12),
                         jive::parseColour(jive::themes::steel::raisinBlack));
        }

        {
            juce::TextButton button;
            button.setSize(100, 25);
            juce::ValueTree state{
                "Button",
                {
                    {
                        "style",
                        new jive::Object{
                            { "background", "blue" },
                        },
                    },
                },
            };
            jive::LookAndFeel lookAndFeel{ button };
            auto sheet = jive::StyleSheet::create(button, state);

            beginTest("style property / with a style property specified, the component should use relevant styles");
            {
                const auto snapshot = button.createComponentSnapshot(button.getLocalBounds());
                expectEquals(snapshot.getPixelAt(50, 12), juce::Colours::blue);
            }

            beginTest("style property / the component should go back to the defaults when the style property is removed");
            {
                state.removeProperty("style", nullptr);
                expectEquals(settledColourAt(button, 50, 12),
                             jive::parseColour(jive::themes::steel::raisinBlack));
            }
        }
    }

    void testIndividualStyleProperties()
    {
        juce::TextButton button;
        button.setSize(100, 25);
        juce::ValueTree state{
            "Button",
            {
                {
                    "style",
                    new jive::Object{},
                },
            },
        };
        auto& style = dynamic_cast<jive::Object&>(*state["style"].getDynamicObject());
        jive::LookAndFeel lookAndFeel{ button };
        auto sheet = jive::StyleSheet::create(button, state);

        beginTest("individual properties / style object with no properties");
        {
            const auto snapshot = button.createComponentSnapshot(button.getLocalBounds());
            expectEquals(snapshot.getPixelAt(50, 12),
                         jive::parseColour(jive::themes::steel::raisinBlack));
        }

        beginTest("individual properties / new style property specified");
        {
            style.setProperty("background", jive::toVar(juce::Colours::pink));

            expectEquals(settledColourAt(button, 50, 12),
                         juce::Colours::pink);
        }

        beginTest("individual properties / property changed");
        {
            style.setProperty("background", jive::toVar(juce::Colours::orange));

            expectEquals(settledColourAt(button, 50, 12),
                         juce::Colours::orange);
        }

        beginTest("individual properties / property removed");
        {
            style.removeProperty("background");

            expectEquals(settledColourAt(button, 50, 12),
                         jive::parseColour(jive::themes::steel::raisinBlack));
        }
    }

    void testTransitions()
    {
        juce::TextButton button;
        button.setSize(100, 25);
        juce::ValueTree state{
            "Button",
            {
                {
                    "style",
                    new jive::Object{
                        { "background", "red" },
                        { "transition", "background 1s linear" },
                    },
                },
            },
        };
        auto& style = dynamic_cast<jive::Object&>(*state["style"].getDynamicObject());
        jive::LookAndFeel lookAndFeel{ button };
        auto sheet = jive::StyleSheet::create(button, state);

        beginTest("transitions / background");
        {
            auto snapshot = button.createComponentSnapshot(button.getLocalBounds());
            expectEquals(snapshot.getPixelAt(50, 12), juce::Colours::red);

            style.setProperty("background", "blue");
            snapshot = button.createComponentSnapshot(button.getLocalBounds());
            expectEquals(snapshot.getPixelAt(50, 12), juce::Colours::red);

            jive::FakeTime::incrementTime(juce::RelativeTime::seconds(0.5));
            snapshot = button.createComponentSnapshot(button.getLocalBounds());
            expectEquals(snapshot.getPixelAt(50, 12), juce::Colours::red.interpolatedWith(juce::Colours::blue, 0.5f));

            jive::FakeTime::incrementTime(juce::RelativeTime::seconds(0.5));
            snapshot = button.createComponentSnapshot(button.getLocalBounds());
            expectEquals(snapshot.getPixelAt(50, 12), juce::Colours::blue);
        }
    }

    // Resolves the most-applicable styles for the given component and returns
    // the current (possibly mid-transition) colour of the named fill style.
    static juce::Colour resolvedColour(jive::LookAndFeel& lookAndFeel,
                                       const juce::Component& component,
                                       const juce::String& styleName)
    {
        return lookAndFeel
            .findMostApplicableStyles(component)
            .find<jive::Fill>(styleName)
            .value()
            .toJuceFillType()
            .colour;
    }

    void testTransitionDetails()
    {
        beginTest("transition details / multiple properties transition independently");
        {
            juce::TextButton button;
            button.setSize(100, 25);
            juce::ValueTree state{
                "Button",
                {
                    {
                        "style",
                        new jive::Object{
                            { "background", "red" },
                            { "foreground", "black" },
                            { "transition", "background 1s linear, foreground 1s linear" },
                        },
                    },
                },
            };
            auto& style = dynamic_cast<jive::Object&>(*state["style"].getDynamicObject());
            jive::LookAndFeel lookAndFeel{ button };
            auto sheet = jive::StyleSheet::create(button, state);

            expectEquals(resolvedColour(lookAndFeel, button, "background"), juce::Colours::red);
            expectEquals(resolvedColour(lookAndFeel, button, "foreground"), juce::Colours::black);

            style.setProperty("background", "blue");
            style.setProperty("foreground", "white");

            // Commence both transitions at the same moment.
            expectEquals(resolvedColour(lookAndFeel, button, "background"), juce::Colours::red);
            expectEquals(resolvedColour(lookAndFeel, button, "foreground"), juce::Colours::black);

            jive::FakeTime::incrementTime(juce::RelativeTime::seconds(0.5));
            expectEquals(resolvedColour(lookAndFeel, button, "background"),
                         juce::Colours::red.interpolatedWith(juce::Colours::blue, 0.5f));
            expectEquals(resolvedColour(lookAndFeel, button, "foreground"),
                         juce::Colours::black.interpolatedWith(juce::Colours::white, 0.5f));

            jive::FakeTime::incrementTime(juce::RelativeTime::seconds(0.5));
            expectEquals(resolvedColour(lookAndFeel, button, "background"), juce::Colours::blue);
            expectEquals(resolvedColour(lookAndFeel, button, "foreground"), juce::Colours::white);
        }

        beginTest("transition details / only properties with a transition animate");
        {
            juce::TextButton button;
            button.setSize(100, 25);
            juce::ValueTree state{
                "Button",
                {
                    {
                        "style",
                        new jive::Object{
                            { "background", "red" },
                            { "foreground", "black" },
                            // A node-level transition replaces the theme default, so
                            // here only "background" should animate.
                            { "transition", "background 1s linear" },
                        },
                    },
                },
            };
            auto& style = dynamic_cast<jive::Object&>(*state["style"].getDynamicObject());
            jive::LookAndFeel lookAndFeel{ button };
            auto sheet = jive::StyleSheet::create(button, state);

            expectEquals(resolvedColour(lookAndFeel, button, "background"), juce::Colours::red);
            expectEquals(resolvedColour(lookAndFeel, button, "foreground"), juce::Colours::black);

            style.setProperty("background", "blue");
            style.setProperty("foreground", "white");

            // Background should be mid-transition (still red), but foreground has
            // no transition so it should change instantly.
            expectEquals(resolvedColour(lookAndFeel, button, "background"), juce::Colours::red);
            expectEquals(resolvedColour(lookAndFeel, button, "foreground"), juce::Colours::white);
        }

        beginTest("transition details / interrupting a transition continues from the current value");
        {
            juce::TextButton button;
            button.setSize(100, 25);
            juce::ValueTree state{
                "Button",
                {
                    {
                        "style",
                        new jive::Object{
                            { "background", "red" },
                            { "transition", "background 1s linear" },
                        },
                    },
                },
            };
            auto& style = dynamic_cast<jive::Object&>(*state["style"].getDynamicObject());
            jive::LookAndFeel lookAndFeel{ button };
            auto sheet = jive::StyleSheet::create(button, state);

            expectEquals(resolvedColour(lookAndFeel, button, "background"), juce::Colours::red);

            // Start transitioning red -> blue.
            style.setProperty("background", "blue");
            expectEquals(resolvedColour(lookAndFeel, button, "background"), juce::Colours::red);

            // Halfway through, the colour is halfway between red and blue.
            jive::FakeTime::incrementTime(juce::RelativeTime::seconds(0.5));
            const auto halfway = juce::Colours::red.interpolatedWith(juce::Colours::blue, 0.5f);
            expectEquals(resolvedColour(lookAndFeel, button, "background"), halfway);

            // Now interrupt by transitioning towards green. The transition should
            // continue smoothly from the *current* (halfway) colour rather than
            // jumping back to the old target (blue).
            style.setProperty("background", "green");
            expectEquals(resolvedColour(lookAndFeel, button, "background"), halfway);

            jive::FakeTime::incrementTime(juce::RelativeTime::seconds(1.0));
            expectEquals(resolvedColour(lookAndFeel, button, "background"), juce::Colours::green);
        }

        beginTest("transition details / a state change transitions and then reverts");
        {
            juce::TextButton button;
            button.setSize(100, 25);
            juce::ValueTree state{
                "Button",
                {
                    {
                        "style",
                        new jive::Object{
                            { "background", "red" },
                            {
                                ":hover",
                                new jive::Object{
                                    { "background", "blue" },
                                },
                            },
                            { "transition", "background 1s linear" },
                        },
                    },
                },
            };
            jive::LookAndFeel lookAndFeel{ button };
            auto sheet = jive::StyleSheet::create(button, state);

            expectEquals(resolvedColour(lookAndFeel, button, "background"), juce::Colours::red);

            // Hovering should transition towards the hover colour.
            button.getProperties().set("jive::hover", true);
            button.lookAndFeelChanged();
            expectEquals(resolvedColour(lookAndFeel, button, "background"), juce::Colours::red);
            jive::FakeTime::incrementTime(juce::RelativeTime::seconds(1.0));
            expectEquals(resolvedColour(lookAndFeel, button, "background"), juce::Colours::blue);

            // Un-hovering should transition back towards the default colour.
            button.getProperties().remove("jive::hover");
            button.lookAndFeelChanged();
            expectEquals(resolvedColour(lookAndFeel, button, "background"), juce::Colours::blue);
            jive::FakeTime::incrementTime(juce::RelativeTime::seconds(0.5));
            expectEquals(resolvedColour(lookAndFeel, button, "background"),
                         juce::Colours::blue.interpolatedWith(juce::Colours::red, 0.5f));
            jive::FakeTime::incrementTime(juce::RelativeTime::seconds(0.5));
            expectEquals(resolvedColour(lookAndFeel, button, "background"), juce::Colours::red);
        }
    }

    void testDynamicStyles()
    {
        beginTest("dynamic styles / adding a class makes a class selector apply");
        {
            juce::Component parent;
            parent.setSize(100, 100);

            juce::TextButton button;
            button.setBounds(10, 10, 80, 25);
            parent.addAndMakeVisible(button);

            juce::ValueTree state{
                "Component",
                {
                    {
                        "style",
                        new jive::Object{
                            {
                                ".special",
                                new jive::Object{
                                    { "background", "magenta" },
                                },
                            },
                        },
                    },
                },
            };
            jive::LookAndFeel lookAndFeel{ parent };
            auto sheet = jive::StyleSheet::create(parent, state);

            // Without the class, the button uses its default background.
            expect(settledColourAt(button, 40, 12) != juce::Colours::magenta);

            // Adding the class should make the selector apply.
            button.getProperties().set("class", "special");
            button.lookAndFeelChanged();
            expectEquals(settledColourAt(button, 40, 12), juce::Colours::magenta);

            // Removing the class should revert to the default background.
            button.getProperties().remove("class");
            button.lookAndFeelChanged();
            expect(settledColourAt(button, 40, 12) != juce::Colours::magenta);
        }

        beginTest("dynamic styles / changing the component ID changes which selector applies");
        {
            juce::Component parent;
            parent.setSize(100, 100);

            juce::TextButton button;
            button.setComponentID("first");
            button.setBounds(10, 10, 80, 25);
            parent.addAndMakeVisible(button);

            juce::ValueTree state{
                "Component",
                {
                    {
                        "style",
                        new jive::Object{
                            {
                                "#first",
                                new jive::Object{
                                    { "background", "teal" },
                                },
                            },
                            {
                                "#second",
                                new jive::Object{
                                    { "background", "maroon" },
                                },
                            },
                        },
                    },
                },
            };
            jive::LookAndFeel lookAndFeel{ parent };
            auto sheet = jive::StyleSheet::create(parent, state);

            expectEquals(settledColourAt(button, 40, 12), juce::Colours::teal);

            button.setComponentID("second");
            button.lookAndFeelChanged();
            expectEquals(settledColourAt(button, 40, 12), juce::Colours::maroon);
        }

        beginTest("dynamic styles / reparenting a component re-resolves descendant selectors");
        {
            juce::Component root;
            root.setSize(200, 200);

            juce::Component containerA;
            containerA.setComponentID("container-a");
            containerA.setBounds(0, 0, 100, 200);
            root.addAndMakeVisible(containerA);

            juce::Component containerB;
            containerB.setComponentID("container-b");
            containerB.setBounds(100, 0, 100, 200);
            root.addAndMakeVisible(containerB);

            juce::TextButton button;
            button.setBounds(0, 0, 80, 25);
            containerA.addAndMakeVisible(button);

            juce::ValueTree state{
                "Component",
                {
                    {
                        "style",
                        new jive::Object{
                            {
                                "#container-a",
                                new jive::Object{
                                    {
                                        "TextButton",
                                        new jive::Object{
                                            { "background", "orange" },
                                        },
                                    },
                                },
                            },
                            {
                                "#container-b",
                                new jive::Object{
                                    {
                                        "TextButton",
                                        new jive::Object{
                                            { "background", "purple" },
                                        },
                                    },
                                },
                            },
                        },
                    },
                },
            };
            jive::LookAndFeel lookAndFeel{ root };
            auto sheet = jive::StyleSheet::create(root, state);

            expectEquals(settledColourAt(button, 40, 12), juce::Colours::orange);

            // Move the button into container B - it should pick up B's styles.
            containerB.addAndMakeVisible(button);
            button.lookAndFeelChanged();
            expectEquals(settledColourAt(button, 40, 12), juce::Colours::purple);
        }
    }

    void testFileBasedStyles()
    {
        beginTest("file-based styles / styles are loaded from a referenced file");
        {
            auto directory = juce::File::getSpecialLocation(juce::File::tempDirectory)
                                 .getChildFile("jive-style-sheet-tests");
            directory.createDirectory();
            auto file = directory.getChildFile("button.json");
            file.replaceWithText(R"({ "background": "red" })");

            juce::TextButton button;
            button.setSize(100, 25);
            juce::ValueTree state{
                "Button",
                {
                    { "jive::source-directories", new jive::SourceDirectories{ directory } },
                    { "style", "button.json" },
                },
            };
            jive::LookAndFeel lookAndFeel{ button };
            auto sheet = jive::StyleSheet::create(button, state);

            const auto styles = lookAndFeel.findMostApplicableStyles(button);
            const auto background = styles.find<jive::Fill>("background");
            expect(background.has_value());
            expect(background.value() == juce::Colours::red);

            file.deleteFile();
            directory.deleteRecursively();
        }

        beginTest("file-based styles / switching the referenced file applies the new file's styles");
        {
            auto directory = juce::File::getSpecialLocation(juce::File::tempDirectory)
                                 .getChildFile("jive-style-sheet-tests");
            directory.createDirectory();
            auto redFile = directory.getChildFile("red.json");
            redFile.replaceWithText(R"({ "background": "red" })");
            auto greenFile = directory.getChildFile("green.json");
            greenFile.replaceWithText(R"({ "background": "green" })");

            juce::TextButton button;
            button.setSize(100, 25);
            juce::ValueTree state{
                "Button",
                {
                    { "jive::source-directories", new jive::SourceDirectories{ directory } },
                    { "style", "red.json" },
                },
            };
            jive::LookAndFeel lookAndFeel{ button };
            auto sheet = jive::StyleSheet::create(button, state);

            expect(lookAndFeel
                       .findMostApplicableStyles(button)
                       .find<jive::Fill>("background")
                       .value()
                   == juce::Colours::red);

            state.setProperty("style", "green.json", nullptr);

            // Switching files changes the background, which transitions via the
            // theme's default transition, so commence then settle it.
            juce::ignoreUnused(lookAndFeel.findMostApplicableStyles(button));
            jive::FakeTime::incrementTime(juce::RelativeTime::seconds(1.0));

            expect(lookAndFeel
                       .findMostApplicableStyles(button)
                       .find<jive::Fill>("background")
                       .value()
                   == juce::Colours::green);

            redFile.deleteFile();
            greenFile.deleteFile();
            directory.deleteRecursively();
        }

        beginTest("file-based styles / modifying the referenced file hot-reloads its styles");
        {
            auto directory = juce::File::getSpecialLocation(juce::File::tempDirectory)
                                 .getChildFile("jive-style-sheet-tests");
            directory.createDirectory();
            auto file = directory.getChildFile("hot.json");
            file.replaceWithText(R"({ "background": "red" })");

            juce::TextButton button;
            button.setSize(100, 25);
            juce::ValueTree state{
                "Button",
                {
                    { "jive::source-directories", new jive::SourceDirectories{ directory } },
                    { "style", "hot.json" },
                },
            };
            jive::LookAndFeel lookAndFeel{ button };
            auto sheet = jive::StyleSheet::create(button, state);

            expect(lookAndFeel
                       .findMostApplicableStyles(button)
                       .find<jive::Fill>("background")
                       .value()
                   == juce::Colours::red);

            // Edit the file's contents and make sure its modification time
            // changes so the observer notices, then trigger the observer.
            file.replaceWithText(R"({ "background": "blue" })");
            file.setLastModificationTime(juce::Time::getCurrentTime() + juce::RelativeTime::seconds(5.0));
            jive::FileObserver::triggerAllTimerCallbacks();

            // The hot-reloaded background change transitions, so settle it.
            juce::ignoreUnused(lookAndFeel.findMostApplicableStyles(button));
            jive::FakeTime::incrementTime(juce::RelativeTime::seconds(1.0));

            expect(lookAndFeel
                       .findMostApplicableStyles(button)
                       .find<jive::Fill>("background")
                       .value()
                   == juce::Colours::blue);

            file.deleteFile();
            directory.deleteRecursively();
        }
    }

    void testMiscStyleProperties()
    {
        beginTest("misc style properties / gradient background resolves as a gradient");
        {
            juce::TextButton button;
            button.setSize(100, 25);
            juce::ValueTree state{
                "Button",
                {
                    {
                        "style",
                        new jive::Object{
                            { "background", "linear-gradient(0deg, #FF0000, #0000FF)" },
                        },
                    },
                },
            };
            jive::LookAndFeel lookAndFeel{ button };
            auto sheet = jive::StyleSheet::create(button, state);

            const auto styles = lookAndFeel.findMostApplicableStyles(button);
            const auto background = styles.find<jive::Fill>("background");
            expect(background.has_value());
            expect(background->isGradient());
        }

        beginTest("misc style properties / border-radius resolves to the specified radii");
        {
            juce::TextButton button;
            button.setSize(100, 25);
            juce::ValueTree state{
                "Button",
                {
                    {
                        "style",
                        new jive::Object{
                            { "border-radius", "10 20 30 40" },
                        },
                    },
                },
            };
            jive::LookAndFeel lookAndFeel{ button };
            auto sheet = jive::StyleSheet::create(button, state);

            const auto styles = lookAndFeel.findMostApplicableStyles(button);
            const auto radii = styles.find<jive::BorderRadii<float>>("border-radius");
            expect(radii.has_value());
            expectEquals(radii->topLeft, 10.0f);
            expectEquals(radii->topRight, 20.0f);
            expectEquals(radii->bottomRight, 30.0f);
            expectEquals(radii->bottomLeft, 40.0f);
        }

        beginTest("misc style properties / border-width set on the component state is applied");
        {
            juce::TextButton button;
            button.setSize(100, 25);
            juce::ValueTree state{
                "Button",
                {
                    { "border-width", 5.0 },
                },
            };
            jive::LookAndFeel lookAndFeel{ button };
            auto sheet = jive::StyleSheet::create(button, state);

            const auto styles = lookAndFeel.findMostApplicableStyles(button);
            const auto borderWidth = styles.find<juce::BorderSize<float>>("border-width");
            expect(borderWidth.has_value());
            expectEquals(borderWidth->getTop(), 5.0f);
            expectEquals(borderWidth->getLeft(), 5.0f);
        }

        beginTest("misc style properties / updating border-width on the state updates the applied style");
        {
            juce::TextButton button;
            button.setSize(100, 25);
            juce::ValueTree state{
                "Button",
                {
                    { "border-width", 5.0 },
                },
            };
            jive::LookAndFeel lookAndFeel{ button };
            auto sheet = jive::StyleSheet::create(button, state);

            state.setProperty("border-width", 12.0, nullptr);

            const auto styles = lookAndFeel.findMostApplicableStyles(button);
            const auto borderWidth = styles.find<juce::BorderSize<float>>("border-width");
            expect(borderWidth.has_value());
            expectEquals(borderWidth->getTop(), 12.0f);
        }

        beginTest("misc style properties / removing border-width from the state reverts to the theme default");
        {
            juce::TextButton button;
            button.setSize(100, 25);
            juce::ValueTree state{
                "Button",
                {
                    { "border-width", 5.0 },
                },
            };
            jive::LookAndFeel lookAndFeel{ button };
            auto sheet = jive::StyleSheet::create(button, state);

            state.removeProperty("border-width", nullptr);

            const auto styles = lookAndFeel.findMostApplicableStyles(button);
            const auto borderWidth = styles.find<juce::BorderSize<float>>("border-width");
            expect(borderWidth.has_value());
            expectEquals(borderWidth->getTop(), jive::themes::steel::borderWidth.getTop());
        }

        beginTest("misc style properties / changing the style object preserves a state border-width");
        {
            juce::TextButton button;
            button.setSize(100, 25);
            juce::ValueTree state{
                "Button",
                {
                    { "border-width", 5.0 },
                    {
                        "style",
                        new jive::Object{
                            { "background", "red" },
                        },
                    },
                },
            };
            auto& style = dynamic_cast<jive::Object&>(*state["style"].getDynamicObject());
            jive::LookAndFeel lookAndFeel{ button };
            auto sheet = jive::StyleSheet::create(button, state);

            expectEquals(lookAndFeel
                             .findMostApplicableStyles(button)
                             .find<juce::BorderSize<float>>("border-width")
                             ->getTop(),
                         5.0f);

            // Changing an unrelated style property shouldn't clobber the
            // border-width that was applied from the component's state.
            style.setProperty("background", "blue");

            expectEquals(lookAndFeel
                             .findMostApplicableStyles(button)
                             .find<juce::BorderSize<float>>("border-width")
                             ->getTop(),
                         5.0f);
        }
    }

    void testInheritance()
    {
        beginTest("inheritance / inheritable properties cascade to descendants but others do not");
        {
            juce::Component parent;
            parent.setSize(200, 200);

            juce::TextButton child;
            child.setBounds(0, 0, 100, 50);
            parent.addAndMakeVisible(child);

            juce::ValueTree state{
                "Component",
                {
                    {
                        "style",
                        new jive::Object{
                            { "background", "red" },
                            { "font-scale", 2.0 },
                        },
                    },
                },
            };
            jive::LookAndFeel lookAndFeel{ parent };
            auto sheet = jive::StyleSheet::create(parent, state);

            const auto childStyles = lookAndFeel.findMostApplicableStyles(child);

            // "font-scale" is an inheritable property, so the child should pick
            // it up from its ancestor.
            expect(childStyles.find<float>("font-scale").has_value());
            expectEquals(childStyles.find<float>("font-scale").value(), 2.0f);

            // "background" is not inheritable, so the child should not become red.
            const auto childBackground = childStyles.find<jive::Fill>("background");
            expect(!childBackground.has_value()
                   || !(childBackground.value() == juce::Colours::red));
        }

        beginTest("inheritance / a child style sheet combines its own styles with inherited parent styles");
        {
            juce::Component parent;
            parent.setSize(200, 200);

            juce::TextButton child;
            child.setBounds(0, 0, 100, 50);
            parent.addAndMakeVisible(child);

            juce::ValueTree parentState{
                "Component",
                {
                    {
                        "style",
                        new jive::Object{
                            { "foreground", "lime" },
                            { "font-scale", 3.0 },
                        },
                    },
                },
            };
            juce::ValueTree childState{
                "Button",
                {
                    {
                        "style",
                        new jive::Object{
                            { "background", "navy" },
                        },
                    },
                },
            };

            jive::LookAndFeel lookAndFeel{ parent };
            auto parentSheet = jive::StyleSheet::create(parent, parentState);
            auto childSheet = jive::StyleSheet::create(child, childState);

            const auto childStyles = lookAndFeel.findMostApplicableStyles(child);

            // The child's own background applies...
            expect(childStyles.find<jive::Fill>("background").value() == juce::Colours::navy);
            // ...as do the inheritable styles cascading down from the parent.
            expect(childStyles.find<jive::Fill>("foreground").value() == juce::Colours::lime);
            expect(childStyles.find<float>("font-scale").has_value());
            expectEquals(childStyles.find<float>("font-scale").value(), 3.0f);
        }

        beginTest("inheritance / a descendant's own value overrides an inherited one");
        {
            juce::Component parent;
            parent.setSize(200, 200);

            juce::TextButton child;
            child.getProperties().set("class", "child");
            child.setBounds(0, 0, 100, 50);
            parent.addAndMakeVisible(child);

            juce::ValueTree state{
                "Component",
                {
                    {
                        "style",
                        new jive::Object{
                            { "font-scale", 2.0 },
                            {
                                ".child",
                                new jive::Object{
                                    { "font-scale", 4.0 },
                                },
                            },
                        },
                    },
                },
            };
            jive::LookAndFeel lookAndFeel{ parent };
            auto sheet = jive::StyleSheet::create(parent, state);

            const auto childStyles = lookAndFeel.findMostApplicableStyles(child);
            expect(childStyles.find<float>("font-scale").has_value());
            expectEquals(childStyles.find<float>("font-scale").value(), 4.0f);
        }
    }

    void testSelectorEdgeCases()
    {
        beginTest("selector edge cases / compound class selector requires all classes");
        {
            juce::Component parent;
            parent.setSize(100, 100);

            juce::TextButton both;
            both.getProperties().set("class", "primary big");
            both.setBounds(10, 10, 80, 25);
            parent.addAndMakeVisible(both);

            juce::TextButton onlyOne;
            onlyOne.getProperties().set("class", "primary");
            onlyOne.setBounds(10, 40, 80, 25);
            parent.addAndMakeVisible(onlyOne);

            juce::ValueTree state{
                "Component",
                {
                    {
                        "style",
                        new jive::Object{
                            {
                                ".primary.big",
                                new jive::Object{
                                    { "background", "teal" },
                                },
                            },
                        },
                    },
                },
            };
            jive::LookAndFeel lookAndFeel{ parent };
            auto sheet = jive::StyleSheet::create(parent, state);

            expectEquals(settledColourAt(both, 40, 12), juce::Colours::teal);
            expect(settledColourAt(onlyOne, 40, 12) != juce::Colours::teal);
        }

        beginTest("selector edge cases / comma-separated selectors including a descendant + type");
        {
            juce::Component panel;
            panel.setComponentID("panel");
            panel.setSize(100, 100);

            juce::TextButton button;
            button.setBounds(10, 10, 80, 25);
            panel.addAndMakeVisible(button);

            juce::ValueTree state{
                "Component",
                {
                    {
                        "style",
                        new jive::Object{
                            {
                                "#panel TextButton, .nonexistent",
                                new jive::Object{
                                    { "background", "olive" },
                                },
                            },
                        },
                    },
                },
            };
            jive::LookAndFeel lookAndFeel{ panel };
            auto sheet = jive::StyleSheet::create(panel, state);

            expectEquals(settledColourAt(button, 40, 12), juce::Colours::olive);
        }

        beginTest("selector edge cases / more specific descendant selector wins over a plain type selector");
        {
            juce::Component container;
            container.setComponentID("container");
            container.setSize(100, 100);

            juce::TextButton button;
            button.setBounds(10, 10, 80, 25);
            container.addAndMakeVisible(button);

            juce::ValueTree state{
                "Component",
                {
                    {
                        "style",
                        new jive::Object{
                            {
                                "TextButton",
                                new jive::Object{
                                    { "background", "blue" },
                                },
                            },
                            {
                                "#container TextButton",
                                new jive::Object{
                                    { "background", "red" },
                                },
                            },
                        },
                    },
                },
            };
            jive::LookAndFeel lookAndFeel{ container };
            auto sheet = jive::StyleSheet::create(container, state);

            // "#container TextButton" carries an ID, so it should win over the
            // plain "TextButton" type selector.
            expectEquals(settledColourAt(button, 40, 12), juce::Colours::red);
        }
    }

    void testSelectorBasedStyles()
    {
        beginTest("selector-based styles / ID selector at top level");
        {
            juce::Component parent;
            parent.setSize(100, 100);

            juce::TextButton button;
            button.setComponentID("my-button");
            button.setBounds(10, 10, 80, 25);
            parent.addAndMakeVisible(button);

            juce::ValueTree state{
                "Component",
                {
                    {
                        "style",
                        new jive::Object{
                            {
                                "#my-button",
                                new jive::Object{
                                    { "background", "green" },
                                },
                            },
                        },
                    },
                },
            };

            jive::LookAndFeel lookAndFeel{ parent };
            auto sheet = jive::StyleSheet::create(parent, state);

            const auto snapshot = button.createComponentSnapshot(button.getLocalBounds());
            expectEquals(snapshot.getPixelAt(40, 12), juce::Colours::green);
        }

        beginTest("selector-based styles / class selector at top level");
        {
            juce::Component parent;
            parent.setSize(100, 100);

            juce::TextButton button1;
            button1.getProperties().set("class", "primary");
            button1.setBounds(10, 10, 80, 25);
            parent.addAndMakeVisible(button1);

            juce::TextButton button2;
            button2.getProperties().set("class", "secondary");
            button2.setBounds(10, 40, 80, 25);
            parent.addAndMakeVisible(button2);

            juce::ValueTree state{
                "Component",
                {
                    {
                        "style",
                        new jive::Object{
                            {
                                ".primary",
                                new jive::Object{
                                    { "background", "blue" },
                                },
                            },
                            {
                                ".secondary",
                                new jive::Object{
                                    { "background", "red" },
                                },
                            },
                        },
                    },
                },
            };

            jive::LookAndFeel lookAndFeel{ parent };
            auto sheet = jive::StyleSheet::create(parent, state);

            const auto snapshot1 = button1.createComponentSnapshot(button1.getLocalBounds());
            expectEquals(snapshot1.getPixelAt(40, 12), juce::Colours::blue);

            const auto snapshot2 = button2.createComponentSnapshot(button2.getLocalBounds());
            expectEquals(snapshot2.getPixelAt(40, 12), juce::Colours::red);
        }

        beginTest("selector-based styles / type selector at top level");
        {
            juce::Component parent;
            parent.setSize(100, 100);

            juce::TextButton button;
            parent.addAndMakeVisible(button);
            button.setBounds(10, 10, 80, 25);

            juce::ValueTree state{
                "Component",
                {
                    {
                        "style",
                        new jive::Object{
                            {
                                "TextButton",
                                new jive::Object{
                                    { "background", "purple" },
                                },
                            },
                        },
                    },
                },
            };

            jive::LookAndFeel lookAndFeel{ parent };
            auto sheet = jive::StyleSheet::create(parent, state);

            const auto snapshot = parent.createComponentSnapshot(parent.getLocalBounds());
            expectEquals(snapshot.getPixelAt(40, 12), juce::Colours::purple);
        }

        beginTest("selector-based styles / nested selectors building hierarchy");
        {
            juce::Component parent;
            parent.setComponentID("container");
            parent.setSize(100, 100);

            juce::Component childContainer;
            childContainer.setComponentID("inner");
            childContainer.setBounds(0, 0, 100, 100);
            parent.addAndMakeVisible(childContainer);

            juce::TextButton button;
            button.getProperties().set("class", "btn");
            button.setBounds(10, 10, 80, 25);
            childContainer.addAndMakeVisible(button);

            juce::ValueTree state{
                "Component",
                {
                    {
                        "style",
                        new jive::Object{
                            {
                                "#container",
                                new jive::Object{
                                    {
                                        "#inner",
                                        new jive::Object{
                                            {
                                                ".btn",
                                                new jive::Object{
                                                    { "background", "orange" },
                                                },
                                            },
                                        },
                                    },
                                },
                            },
                        },
                    },
                },
            };

            jive::LookAndFeel lookAndFeel{ parent };
            auto sheet = jive::StyleSheet::create(parent, state);

            // The full selector should be "#container #inner .btn"
            const auto snapshot = button.createComponentSnapshot(button.getLocalBounds());
            expectEquals(snapshot.getPixelAt(40, 12), juce::Colours::orange);
        }

        beginTest("selector-based styles / direct child selector");
        {
            juce::Component parent;
            parent.setComponentID("parent");
            parent.setSize(100, 100);

            juce::TextButton directChild;
            directChild.getProperties().set("class", "btn");
            directChild.setBounds(10, 10, 80, 25);
            parent.addAndMakeVisible(directChild);

            juce::Component wrapper;
            wrapper.setBounds(10, 40, 80, 50);
            parent.addAndMakeVisible(wrapper);

            juce::TextButton indirectChild;
            indirectChild.getProperties().set("class", "btn");
            indirectChild.setBounds(0, 0, 80, 25);
            wrapper.addAndMakeVisible(indirectChild);

            juce::ValueTree state{
                "Component",
                {
                    {
                        "style",
                        new jive::Object{
                            {
                                "#parent > .btn",
                                new jive::Object{
                                    { "background", "cyan" },
                                },
                            },
                        },
                    },
                },
            };

            jive::LookAndFeel lookAndFeel{ parent };
            auto sheet = jive::StyleSheet::create(parent, state);

            // Direct child should have the style
            const auto directSnapshot = directChild.createComponentSnapshot(directChild.getLocalBounds());
            expectEquals(directSnapshot.getPixelAt(40, 12), juce::Colours::cyan);

            // Indirect child should NOT have the style (use default)
            const auto indirectSnapshot = indirectChild.createComponentSnapshot(indirectChild.getLocalBounds());
            expectEquals(indirectSnapshot.getPixelAt(40, 12),
                         jive::parseColour(jive::themes::steel::raisinBlack));
        }

        beginTest("selector-based styles / state selectors");
        {
            juce::TextButton button;
            button.setSize(100, 25);

            juce::ValueTree state{
                "Button",
                {
                    {
                        "style",
                        new jive::Object{
                            { "background", "gray" },
                            {
                                ":hover",
                                new jive::Object{
                                    { "background", "lightgray" },
                                },
                            },
                            {
                                ":active",
                                new jive::Object{
                                    { "background", "darkgray" },
                                },
                            },
                        },
                    },
                },
            };

            jive::LookAndFeel lookAndFeel{ button };
            auto sheet = jive::StyleSheet::create(button, state);

            // Default state
            expectEquals(settledColourAt(button, 50, 12), juce::Colours::grey);

            // Hover state
            button.getProperties().set("jive::hover", true);
            button.lookAndFeelChanged();
            expectEquals(settledColourAt(button, 50, 12), juce::Colours::lightgrey);

            // Active state
            button.getProperties().remove("jive::hover");
            button.getProperties().set("jive::active", true);
            button.lookAndFeelChanged();
            expectEquals(settledColourAt(button, 50, 12), juce::Colours::darkgrey);
        }

        beginTest("selector-based styles / multiple selectors (comma-separated)");
        {
            juce::Component parent;
            parent.setSize(200, 100);

            juce::TextButton button1;
            button1.setComponentID("btn-primary");
            button1.setBounds(10, 10, 80, 25);
            parent.addAndMakeVisible(button1);

            juce::TextButton button2;
            button2.getProperties().set("class", "highlighted");
            button2.setBounds(100, 10, 80, 25);
            parent.addAndMakeVisible(button2);

            juce::TextButton button3;
            button3.setBounds(10, 50, 80, 25);
            parent.addAndMakeVisible(button3);

            juce::ValueTree state{
                "Component",
                {
                    {
                        "style",
                        new jive::Object{
                            {
                                "#btn-primary, .highlighted",
                                new jive::Object{
                                    { "background", "yellow" },
                                },
                            },
                        },
                    },
                },
            };

            jive::LookAndFeel lookAndFeel{ parent };
            auto sheet = jive::StyleSheet::create(parent, state);

            // Both button1 and button2 should match
            const auto snapshot1 = button1.createComponentSnapshot(button1.getLocalBounds());
            expectEquals(snapshot1.getPixelAt(40, 12), juce::Colours::yellow);

            const auto snapshot2 = button2.createComponentSnapshot(button2.getLocalBounds());
            expectEquals(snapshot2.getPixelAt(40, 12), juce::Colours::yellow);

            // button3 should use default styles
            const auto snapshot3 = button3.createComponentSnapshot(button3.getLocalBounds());
            expectEquals(snapshot3.getPixelAt(40, 12),
                         jive::parseColour(jive::themes::steel::raisinBlack));
        }

        beginTest("selector-based styles / precedence - more specific selector wins");
        {
            juce::TextButton button;
            button.setComponentID("special-btn");
            button.getProperties().set("class", "btn");
            button.setSize(100, 25);

            juce::ValueTree state{
                "Button",
                {
                    {
                        "style",
                        new jive::Object{
                            {
                                ".btn",
                                new jive::Object{
                                    { "background", "blue" },
                                },
                            },
                            {
                                "#special-btn",
                                new jive::Object{
                                    { "background", "gold" },
                                },
                            },
                        },
                    },
                },
            };

            jive::LookAndFeel lookAndFeel{ button };
            auto sheet = jive::StyleSheet::create(button, state);

            // ID selector has higher precedence than class selector
            const auto snapshot = button.createComponentSnapshot(button.getLocalBounds());
            expectEquals(snapshot.getPixelAt(50, 12), juce::Colours::gold);
        }
    }

    void testMultiTierSelectors()
    {
        beginTest("multi-tier selectors / styles on individual ValueTree nodes apply to that node's component");
        {
            juce::Component root;
            root.setSize(200, 100);

            juce::TextButton button;
            button.setBounds(10, 10, 80, 25);
            root.addAndMakeVisible(button);

            // Style is specified on the button's own ValueTree node
            juce::ValueTree buttonState{
                "TextButton",
                {
                    {
                        "style",
                        new jive::Object{
                            { "background", "red" },
                        },
                    },
                },
            };

            jive::LookAndFeel lookAndFeel{ root };
            auto buttonSheet = jive::StyleSheet::create(button, buttonState);

            const auto snapshot = button.createComponentSnapshot(button.getLocalBounds());
            expectEquals(snapshot.getPixelAt(40, 12), juce::Colours::red);
        }

        beginTest("multi-tier selectors / stateful :hover style on individual node");
        {
            juce::Component root;
            root.setSize(200, 100);

            juce::TextButton button;
            button.setBounds(10, 10, 80, 25);
            root.addAndMakeVisible(button);

            juce::ValueTree buttonState{
                "TextButton",
                {
                    {
                        "style",
                        new jive::Object{
                            { "background", "red" },
                            {
                                ":hover",
                                new jive::Object{
                                    { "background", "blue" },
                                },
                            },
                        },
                    },
                },
            };

            jive::LookAndFeel lookAndFeel{ root };
            auto buttonSheet = jive::StyleSheet::create(button, buttonState);

            // Default state
            expectEquals(settledColourAt(button, 40, 12), juce::Colours::red);

            // Hover state
            button.getProperties().set("jive::hover", true);
            button.lookAndFeelChanged();
            expectEquals(settledColourAt(button, 40, 12), juce::Colours::blue);
        }

        beginTest("multi-tier selectors / stateful :active style on individual node");
        {
            juce::Component root;
            root.setSize(200, 100);

            juce::TextButton button;
            button.setBounds(10, 10, 80, 25);
            root.addAndMakeVisible(button);

            juce::ValueTree buttonState{
                "TextButton",
                {
                    {
                        "style",
                        new jive::Object{
                            { "background", "green" },
                            {
                                ":active",
                                new jive::Object{
                                    { "background", "darkgreen" },
                                },
                            },
                        },
                    },
                },
            };

            jive::LookAndFeel lookAndFeel{ root };
            auto buttonSheet = jive::StyleSheet::create(button, buttonState);

            // Default state
            expectEquals(settledColourAt(button, 40, 12), juce::Colours::green);

            // Active state
            button.getProperties().set("jive::active", true);
            button.lookAndFeelChanged();
            expectEquals(settledColourAt(button, 40, 12), juce::Colours::darkgreen);
        }

        beginTest("multi-tier selectors / stateful :focus style on individual node");
        {
            juce::Component root;
            root.setSize(200, 100);

            juce::TextButton button;
            button.setBounds(10, 10, 80, 25);
            root.addAndMakeVisible(button);

            juce::ValueTree buttonState{
                "TextButton",
                {
                    {
                        "style",
                        new jive::Object{
                            { "background", "orange" },
                            {
                                ":focus",
                                new jive::Object{
                                    { "background", "darkorange" },
                                },
                            },
                        },
                    },
                },
            };

            jive::LookAndFeel lookAndFeel{ root };
            auto buttonSheet = jive::StyleSheet::create(button, buttonState);

            // Default state
            expectEquals(settledColourAt(button, 40, 12), juce::Colours::orange);

            // Focus state
            button.getProperties().set("jive::focus", true);
            button.lookAndFeelChanged();
            expectEquals(settledColourAt(button, 40, 12), juce::Colours::darkorange);
        }

        beginTest("multi-tier selectors / stateful :disabled style on individual node");
        {
            juce::Component root;
            root.setSize(200, 100);

            juce::TextButton button;
            button.setBounds(10, 10, 80, 25);
            root.addAndMakeVisible(button);

            juce::ValueTree buttonState{
                "TextButton",
                {
                    {
                        "style",
                        new jive::Object{
                            { "background", "cyan" },
                            {
                                ":disabled",
                                new jive::Object{
                                    { "background", "gray" },
                                },
                            },
                        },
                    },
                },
            };

            jive::LookAndFeel lookAndFeel{ root };
            auto buttonSheet = jive::StyleSheet::create(button, buttonState);

            // Enabled state
            expectEquals(settledColourAt(button, 40, 12), juce::Colours::cyan);

            // Disabled state
            button.setEnabled(false);
            button.lookAndFeelChanged();
            expectEquals(settledColourAt(button, 40, 12), juce::Colours::grey);
        }

        beginTest("multi-tier selectors / stateful :checked style on individual toggle button node");
        {
            juce::Component root;
            root.setSize(200, 100);

            juce::ToggleButton toggle;
            toggle.setBounds(10, 10, 80, 25);
            root.addAndMakeVisible(toggle);

            juce::ValueTree toggleState{
                "ToggleButton",
                {
                    {
                        "style",
                        new jive::Object{
                            { "background", "salmon" },
                            {
                                ":checked",
                                new jive::Object{
                                    { "background", "springgreen" },
                                    { "accent", "rgba(0, 0, 0, 0)" },
                                },
                            },
                        },
                    },
                },
            };

            jive::LookAndFeel lookAndFeel{ root };
            auto toggleSheet = jive::StyleSheet::create(toggle, toggleState);

            // Unchecked state
            expectEquals(settledColourAt(toggle, 12, 12), juce::Colours::salmon);

            // Checked state
            toggle.setToggleState(true, juce::sendNotificationSync);
            toggle.lookAndFeelChanged();
            expectEquals(settledColourAt(toggle, 12, 12), juce::Colours::springgreen);
        }

        beginTest("multi-tier selectors / multiple nodes with independent styles");
        {
            juce::Component root;
            root.setSize(200, 100);

            juce::TextButton button1;
            button1.setBounds(10, 10, 80, 25);
            root.addAndMakeVisible(button1);

            juce::TextButton button2;
            button2.setBounds(100, 10, 80, 25);
            root.addAndMakeVisible(button2);

            // Each button has its own ValueTree node with independent styles
            juce::ValueTree button1State{
                "TextButton",
                {
                    {
                        "style",
                        new jive::Object{
                            { "background", "tomato" },
                            {
                                ":hover",
                                new jive::Object{
                                    { "background", "coral" },
                                },
                            },
                        },
                    },
                },
            };

            juce::ValueTree button2State{
                "TextButton",
                {
                    {
                        "style",
                        new jive::Object{
                            { "background", "steelblue" },
                            {
                                ":hover",
                                new jive::Object{
                                    { "background", "lightsteelblue" },
                                },
                            },
                        },
                    },
                },
            };

            jive::LookAndFeel lookAndFeel{ root };
            auto button1Sheet = jive::StyleSheet::create(button1, button1State);
            auto button2Sheet = jive::StyleSheet::create(button2, button2State);

            // Default state - each button has its own color
            expectEquals(settledColourAt(button1, 40, 12), juce::Colours::tomato);
            expectEquals(settledColourAt(button2, 40, 12), juce::Colours::steelblue);

            // Hover button1 only
            button1.getProperties().set("jive::hover", true);
            button1.lookAndFeelChanged();
            expectEquals(settledColourAt(button1, 40, 12), juce::Colours::coral);

            // button2 should still have its default color
            expectEquals(settledColourAt(button2, 40, 12), juce::Colours::steelblue);

            // Now hover button2
            button2.getProperties().set("jive::hover", true);
            button2.lookAndFeelChanged();
            expectEquals(settledColourAt(button2, 40, 12), juce::Colours::lightsteelblue);
        }

        beginTest("multi-tier selectors / node-level styles with selectors for children");
        {
            juce::Component container;
            container.setSize(200, 100);

            juce::TextButton childButton;
            childButton.getProperties().set("class", "action-btn");
            childButton.setBounds(10, 10, 80, 25);
            container.addAndMakeVisible(childButton);

            // Container's ValueTree node has styles that target its children
            juce::ValueTree containerState{
                "Component",
                {
                    {
                        "style",
                        new jive::Object{
                            {
                                ".action-btn",
                                new jive::Object{
                                    { "background", "indigo" },
                                },
                            },
                            {
                                ".action-btn:hover",
                                new jive::Object{
                                    { "background", "blueviolet" },
                                },
                            },
                        },
                    },
                },
            };

            jive::LookAndFeel lookAndFeel{ container };
            auto containerSheet = jive::StyleSheet::create(container, containerState);

            // Default state
            expectEquals(settledColourAt(childButton, 40, 12), juce::Colours::indigo);

            // Hover state
            childButton.getProperties().set("jive::hover", true);
            childButton.lookAndFeelChanged();
            expectEquals(settledColourAt(childButton, 40, 12), juce::Colours::blueviolet);
        }

        beginTest("multi-tier selectors / combining node-level inline styles with child selectors");
        {
            juce::Component panel;
            panel.setSize(200, 100);

            juce::TextButton btn1;
            btn1.setComponentID("primary");
            btn1.setBounds(10, 10, 80, 25);
            panel.addAndMakeVisible(btn1);

            juce::TextButton btn2;
            btn2.setComponentID("secondary");
            btn2.setBounds(10, 40, 80, 25);
            panel.addAndMakeVisible(btn2);

            // Panel has its own inline style plus child selectors
            juce::ValueTree panelState{
                "Component",
                {
                    {
                        "style",
                        new jive::Object{
                            { "background", "whitesmoke" }, // Panel's own background
                            {
                                "#primary",
                                new jive::Object{
                                    { "background", "royalblue" },
                                },
                            },
                            {
                                "#primary:active",
                                new jive::Object{
                                    { "background", "navy" },
                                },
                            },
                            {
                                "#secondary",
                                new jive::Object{
                                    { "background", "crimson" },
                                },
                            },
                            {
                                "#secondary:active",
                                new jive::Object{
                                    { "background", "darkred" },
                                },
                            },
                        },
                    },
                },
            };

            jive::LookAndFeel lookAndFeel{ panel };
            auto panelSheet = jive::StyleSheet::create(panel, panelState);

            // Default states
            expectEquals(settledColourAt(btn1, 40, 12), juce::Colours::royalblue);
            expectEquals(settledColourAt(btn2, 40, 12), juce::Colours::crimson);

            // Active states
            btn1.getProperties().set("jive::active", true);
            btn1.lookAndFeelChanged();
            expectEquals(settledColourAt(btn1, 40, 12), juce::Colours::navy);

            btn2.getProperties().set("jive::active", true);
            btn2.lookAndFeelChanged();
            expectEquals(settledColourAt(btn2, 40, 12), juce::Colours::darkred);
        }

        beginTest("multi-tier selectors / deeply nested component hierarchy with node-level styles");
        {
            juce::Component root;
            root.setSize(200, 200);

            juce::Component level1;
            level1.setBounds(0, 0, 200, 200);
            root.addAndMakeVisible(level1);

            juce::Component level2;
            level2.setBounds(0, 0, 200, 200);
            level1.addAndMakeVisible(level2);

            juce::TextButton deepButton;
            deepButton.getProperties().set("class", "deep");
            deepButton.setBounds(10, 10, 80, 25);
            level2.addAndMakeVisible(deepButton);

            // Only level2 has styles - they should apply to deepButton
            juce::ValueTree level2State{
                "Component",
                {
                    {
                        "style",
                        new jive::Object{
                            {
                                ".deep",
                                new jive::Object{
                                    { "background", "gold" },
                                },
                            },
                            {
                                ".deep:hover",
                                new jive::Object{
                                    { "background", "goldenrod" },
                                },
                            },
                        },
                    },
                },
            };

            jive::LookAndFeel lookAndFeel{ root };
            auto level2Sheet = jive::StyleSheet::create(level2, level2State);

            // Default state
            expectEquals(settledColourAt(deepButton, 40, 12), juce::Colours::gold);

            // Hover state
            deepButton.getProperties().set("jive::hover", true);
            deepButton.lookAndFeelChanged();
            expectEquals(settledColourAt(deepButton, 40, 12), juce::Colours::goldenrod);
        }

        beginTest("multi-tier selectors / combined state selectors :hover:active on node");
        {
            juce::Component root;
            root.setSize(200, 100);

            juce::TextButton button;
            button.setBounds(10, 10, 80, 25);
            root.addAndMakeVisible(button);

            juce::ValueTree buttonState{
                "TextButton",
                {
                    {
                        "style",
                        new jive::Object{
                            { "background", "purple" },
                            {
                                ":hover",
                                new jive::Object{
                                    { "background", "mediumpurple" },
                                },
                            },
                            {
                                ":hover:active",
                                new jive::Object{
                                    { "background", "rebeccapurple" },
                                },
                            },
                        },
                    },
                },
            };

            jive::LookAndFeel lookAndFeel{ root };
            auto buttonSheet = jive::StyleSheet::create(button, buttonState);

            // Default state
            expectEquals(settledColourAt(button, 40, 12), juce::Colours::purple);

            // Hover only
            button.getProperties().set("jive::hover", true);
            button.lookAndFeelChanged();
            expectEquals(settledColourAt(button, 40, 12), juce::Colours::mediumpurple);

            // Hover and active combined
            button.getProperties().set("jive::active", true);
            button.lookAndFeelChanged();
            expectEquals(settledColourAt(button, 40, 12), juce::Colours::rebeccapurple);
        }

        beginTest("multi-tier selectors / type selector with state on node");
        {
            juce::Component container;
            container.setSize(200, 100);

            juce::TextButton button;
            button.setBounds(10, 10, 80, 25);
            container.addAndMakeVisible(button);

            juce::ValueTree containerState{
                "Component",
                {
                    {
                        "style",
                        new jive::Object{
                            {
                                "TextButton",
                                new jive::Object{
                                    { "background", "olive" },
                                },
                            },
                            {
                                "TextButton:hover",
                                new jive::Object{
                                    { "background", "olivedrab" },
                                },
                            },
                            {
                                "TextButton:active",
                                new jive::Object{
                                    { "background", "darkolivegreen" },
                                },
                            },
                        },
                    },
                },
            };

            jive::LookAndFeel lookAndFeel{ container };
            auto containerSheet = jive::StyleSheet::create(container, containerState);

            // Default state
            expectEquals(settledColourAt(button, 40, 12), juce::Colours::olive);

            // Hover state
            button.getProperties().set("jive::hover", true);
            button.lookAndFeelChanged();
            expectEquals(settledColourAt(button, 40, 12), juce::Colours::olivedrab);

            // Active state (remove hover first to test active independently)
            button.getProperties().remove("jive::hover");
            button.getProperties().set("jive::active", true);
            button.lookAndFeelChanged();
            expectEquals(settledColourAt(button, 40, 12), juce::Colours::darkolivegreen);
        }
    }
};

static StyleSheetTests styleSheetTests;
#endif
