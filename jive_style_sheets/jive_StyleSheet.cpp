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
            uuids.erase("border-width");

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
        for (const auto& [name, uuid] : uuids)
            lookAndFeel.removeStyles(uuid);

        uuids.clear();
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
                    {
                        clear();
                        addStylesFrom(*object);
                    }
                };

                fileObserver = std::make_unique<FileObserver>(file);
                fileObserver->onFileModified = addStylesFromFile;
                addStylesFromFile();
            }
        }
    }

    [[nodiscard]] static juce::String calculateStylesID(Object& object)
    {
        if (object.getParent() == nullptr)
            return "Component";

        juce::StringArray parts;
        parts.add(object.findNameInParent());

        for (auto* parent = object.getParent();
             parent != nullptr;
             parent = parent->getParent())
        {
            if (parent->getParent() != nullptr)
                parts.insert(0, parent->findNameInParent());
        }

        return parts.joinIntoString("/");
    }

    void StyleSheet::addStylesFrom(Object& object)
    {
        const auto id = calculateStylesID(object);

        if (uuids.count(id))
        {
            lookAndFeel.removeStyles(uuids[id]);
            uuids.erase(id);
        }

        addInlineStyles(object, id);
        addSelectorBasedStyles(object, id);

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

    void StyleSheet::addInlineStyles(Object& object,
                                     const juce::String& id)
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

                vBlank = std::make_unique<juce::VBlankAttachment>(&component, [this, transitions]() {
                    component.repaint();
                });
            }
        }

        if (styles.has_value())
        {
            uuids[id] = lookAndFeel.addStyles(
                StyleSelector::makeInlineStyleSelector(component),
                *styles);
        }
    }

    void StyleSheet::addSelectorBasedStyles(Object& object,
                                            const juce::String& id)
    {
        static constexpr auto getObjectIfNestedStyleObject = [](const juce::Identifier& name,
                                                                const juce::var& value) -> Object* {
            if (Styles::propertyNames.count(name.toString()) == 0)
                return dynamic_cast<Object*>(value.getObject());

            return nullptr;
        };
        std::function<void(Object&, const juce::String&)> appendFromChild;
        appendFromChild = [this, &id, &appendFromChild](Object& child,
                                                        const juce::String& selector) {
            std::optional<Styles> styles;
            appendStyleProperties(child, styles);

            if (styles.has_value())
                uuids[id] = lookAndFeel.addStyles(StyleSelector{ selector, &component }, *styles);

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

    // void StyleSheet::addStylesFrom(Object& object,
    //                                LookAndFeel::ComponentPredicate predicate,
    //                                InteractionState interationState,
    //                                LookAndFeel::Precedence precedence)
    // {
    //     const auto id = calculateStylesID(object);
    //     const auto addStyles = [this, id, obj = Object::WeakReference{ &object }, predicate, interationState, precedence]() {
    //         std::optional<Styles> styles;

    //         if (uuids.count(id))
    //         {
    //             styles = *lookAndFeel.findStyles(uuids[id]);
    //             lookAndFeel.removeStyles(uuids[id]);
    //             uuids.erase(id);
    //         }

    //         if (obj == nullptr)
    //             return;

    //         findAndAddStyleProperties(*obj,
    //                                   id,
    //                                   styles,
    //                                   predicate,
    //                                   interationState,
    //                                   precedence);
    //         findAndAddStatefulStyles(*obj,
    //                                  predicate,
    //                                  interationState,
    //                                  precedence);
    //         findAndAddOtherStyles(*obj);

    //         component.lookAndFeelChanged();
    //         component.repaint();
    //     };

    //     if (observedObjects.count(id) == 0 || observedObjects.at(id).get().get() != &object)
    //     {
    //         StyleProperty* newProperty = nullptr;
    //         observedObjects.erase(id);

    //         jassert(observedObjects.count(id) == 0);

    //         if (auto* parent = object.getParent())
    //         {
    //             const auto [entry, wasAdded] = observedObjects.emplace(id, StyleProperty{ parent, object.findNameInParent() });
    //             newProperty = &entry->second;
    //         }
    //         else if (&object == style.get().get())
    //         {
    //             const auto [entry, wasAdded] = observedObjects.emplace(id, StyleProperty{ state, "style" });
    //             newProperty = &entry->second;
    //         }

    //         if (newProperty != nullptr)
    //         {
    //             newProperty->get()->addListener(*this);
    //             newProperty->onValueChange = addStyles;
    //         }
    //     }

    //     addStyles();
    // }

    // void StyleSheet::findAndAddStyleProperties(Object& object,
    //                                            const juce::String& id,
    //                                            std::optional<Styles>& styles,
    //                                            LookAndFeel::ComponentPredicate predicate,
    //                                            InteractionState interationState,
    //                                            LookAndFeel::Precedence precedence)
    // {
    //     auto newStylesAdded = false;

    //     for (const auto& propertyName : Styles::propertyNames)
    //     {
    //         if (!object.hasProperty(propertyName))
    //             continue;

    //         styles = styles
    //                      .value_or(Styles{})
    //                      .with(propertyName, object.getProperty(propertyName));
    //         newStylesAdded = true;
    //     }

    //     if (&object == style.get().get() && object.hasProperty("transition"))
    //     {
    //         auto transitions = fromVar<Transitions::ReferenceCountedPointer>(object["transition"]);

    //         if (transitions != nullptr)
    //         {
    //             styles = styles
    //                          .value_or(Styles{})
    //                          .withTransitions(transitions);
    //             newStylesAdded = true;

    //             vBlank = std::make_unique<juce::VBlankAttachment>(&component, [this, transitions]() {
    //                 component.repaint();
    //             });
    //         }
    //     }

    //     if (styles.has_value() && newStylesAdded)
    //     {
    //         uuids[id] = lookAndFeel.addStyles(
    //             predicate,
    //             *styles,
    //             interationState,
    //             precedence);
    //     }
    // }

    // void StyleSheet::findAndAddStatefulStyles(Object& object,
    //                                           LookAndFeel::ComponentPredicate predicate,
    //                                           InteractionState interationState,
    //                                           LookAndFeel::Precedence precedence)
    // {
    //     for (const auto& stateName : InteractionState::propertyNames)
    //     {
    //         if (!object.hasProperty(stateName))
    //             continue;

    //         if (auto* child = dynamic_cast<Object*>(object[stateName].getDynamicObject()))
    //         {
    //             auto newState = interationState;
    //             newState.set(stateName);
    //             addStylesFrom(*child, predicate, newState, precedence);
    //         }
    //     }
    // }

    // void StyleSheet::findAndAddOtherStyles(Object& object)
    // {
    //     for (const auto& [name, value] : object.getProperties())
    //     {
    //         if (Styles::propertyNames.count(name.toString()) > 0 || InteractionState::propertyNames.count(name.toString()) > 0)
    //             continue;

    //         auto child = dynamic_cast<Object*>(value.getDynamicObject());

    //         if (child == nullptr)
    //             continue;

    //         if (name.toString().startsWith("#"))
    //         {
    //             addStylesFrom(
    //                 *child,
    //                 [safeComponent = juce::Component::SafePointer<juce::Component>{ &component },
    //                  compID = name.toString().fromFirstOccurrenceOf("#", false, false)](const juce::Component& comp) {
    //                     return comp.getComponentID() == compID
    //                         && safeComponent != nullptr
    //                         && (safeComponent.getComponent() == &comp
    //                             || safeComponent->isParentOf(&comp));
    //                 },
    //                 InteractionState{},
    //                 painterPrecedence::componentID);
    //         }
    //         else if (name.toString().startsWith("."))
    //         {
    //             addStylesFrom(
    //                 *child,
    //                 [safeComponent = juce::Component::SafePointer<juce::Component>{ &component },
    //                  className = name.toString().fromFirstOccurrenceOf(".", false, false)](const juce::Component& comp) {
    //                     const auto classes = fromVar<juce::StringArray>(comp.getProperties().getWithDefault("class", ""));

    //                     return classes.contains(className)
    //                         && safeComponent != nullptr
    //                         && (safeComponent.getComponent() == &comp
    //                             || safeComponent->isParentOf(&comp));
    //                 },
    //                 InteractionState{},
    //                 painterPrecedence::componentClass);
    //         }
    //         else
    //         {
    //             addStylesFrom(
    //                 *child,
    //                 [safeComponent = juce::Component::SafePointer<juce::Component>{ &component },
    //                  typeName = name.toString()](const juce::Component& comp) {
    //                     return ComponentTypePredicates::check(comp, typeName)
    //                         && safeComponent != nullptr
    //                         && (safeComponent.getComponent() == &comp
    //                             || safeComponent->isParentOf(&comp));
    //                 },
    //                 InteractionState{},
    //                 painterPrecedence::componentType);
    //         }
    //     }
    // }
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
        testSelectorBasedStyles();
        testMultiTierSelectors();
    }

private:
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
                const auto snapshot = button.createComponentSnapshot(button.getLocalBounds());
                expectEquals(snapshot.getPixelAt(50, 12),
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

            const auto snapshot = button.createComponentSnapshot(button.getLocalBounds());
            expectEquals(snapshot.getPixelAt(50, 12),
                         juce::Colours::pink);
        }

        beginTest("individual properties / property changed");
        {
            style.setProperty("background", jive::toVar(juce::Colours::orange));

            const auto snapshot = button.createComponentSnapshot(button.getLocalBounds());
            expectEquals(snapshot.getPixelAt(50, 12),
                         juce::Colours::orange);
        }

        beginTest("individual properties / property removed");
        {
            style.removeProperty("background");

            const auto snapshot = button.createComponentSnapshot(button.getLocalBounds());
            expectEquals(snapshot.getPixelAt(50, 12),
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
            auto snapshot = button.createComponentSnapshot(button.getLocalBounds());
            expectEquals(snapshot.getPixelAt(50, 12), juce::Colours::grey);

            // Hover state
            button.getProperties().set("jive::hover", true);
            button.lookAndFeelChanged();
            snapshot = button.createComponentSnapshot(button.getLocalBounds());
            expectEquals(snapshot.getPixelAt(50, 12), juce::Colours::lightgrey);

            // Active state
            button.getProperties().remove("jive::hover");
            button.getProperties().set("jive::active", true);
            button.lookAndFeelChanged();
            snapshot = button.createComponentSnapshot(button.getLocalBounds());
            expectEquals(snapshot.getPixelAt(50, 12), juce::Colours::darkgrey);
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
            auto snapshot = button.createComponentSnapshot(button.getLocalBounds());
            expectEquals(snapshot.getPixelAt(40, 12), juce::Colours::red);

            // Hover state
            button.getProperties().set("jive::hover", true);
            button.lookAndFeelChanged();
            snapshot = button.createComponentSnapshot(button.getLocalBounds());
            expectEquals(snapshot.getPixelAt(40, 12), juce::Colours::blue);
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
            auto snapshot = button.createComponentSnapshot(button.getLocalBounds());
            expectEquals(snapshot.getPixelAt(40, 12), juce::Colours::green);

            // Active state
            button.getProperties().set("jive::active", true);
            button.lookAndFeelChanged();
            snapshot = button.createComponentSnapshot(button.getLocalBounds());
            expectEquals(snapshot.getPixelAt(40, 12), juce::Colours::darkgreen);
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
            auto snapshot = button.createComponentSnapshot(button.getLocalBounds());
            expectEquals(snapshot.getPixelAt(40, 12), juce::Colours::orange);

            // Focus state
            button.getProperties().set("jive::focus", true);
            button.lookAndFeelChanged();
            snapshot = button.createComponentSnapshot(button.getLocalBounds());
            expectEquals(snapshot.getPixelAt(40, 12), juce::Colours::darkorange);
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
            auto snapshot = button.createComponentSnapshot(button.getLocalBounds());
            expectEquals(snapshot.getPixelAt(40, 12), juce::Colours::cyan);

            // Disabled state
            button.setEnabled(false);
            button.lookAndFeelChanged();
            snapshot = button.createComponentSnapshot(button.getLocalBounds());
            expectEquals(snapshot.getPixelAt(40, 12), juce::Colours::grey);
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
            auto snapshot = toggle.createComponentSnapshot(toggle.getLocalBounds());
            expectEquals(snapshot.getPixelAt(12, 12), juce::Colours::salmon);

            // Checked state
            toggle.setToggleState(true, juce::sendNotificationSync);
            toggle.lookAndFeelChanged();
            snapshot = toggle.createComponentSnapshot(toggle.getLocalBounds());
            expectEquals(snapshot.getPixelAt(12, 12), juce::Colours::springgreen);
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
            auto snapshot1 = button1.createComponentSnapshot(button1.getLocalBounds());
            expectEquals(snapshot1.getPixelAt(40, 12), juce::Colours::tomato);

            auto snapshot2 = button2.createComponentSnapshot(button2.getLocalBounds());
            expectEquals(snapshot2.getPixelAt(40, 12), juce::Colours::steelblue);

            // Hover button1 only
            button1.getProperties().set("jive::hover", true);
            button1.lookAndFeelChanged();
            snapshot1 = button1.createComponentSnapshot(button1.getLocalBounds());
            expectEquals(snapshot1.getPixelAt(40, 12), juce::Colours::coral);

            // button2 should still have its default color
            snapshot2 = button2.createComponentSnapshot(button2.getLocalBounds());
            expectEquals(snapshot2.getPixelAt(40, 12), juce::Colours::steelblue);

            // Now hover button2
            button2.getProperties().set("jive::hover", true);
            button2.lookAndFeelChanged();
            snapshot2 = button2.createComponentSnapshot(button2.getLocalBounds());
            expectEquals(snapshot2.getPixelAt(40, 12), juce::Colours::lightsteelblue);
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
            auto snapshot = childButton.createComponentSnapshot(childButton.getLocalBounds());
            expectEquals(snapshot.getPixelAt(40, 12), juce::Colours::indigo);

            // Hover state
            childButton.getProperties().set("jive::hover", true);
            childButton.lookAndFeelChanged();
            snapshot = childButton.createComponentSnapshot(childButton.getLocalBounds());
            expectEquals(snapshot.getPixelAt(40, 12), juce::Colours::blueviolet);
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
            auto snapshot1 = btn1.createComponentSnapshot(btn1.getLocalBounds());
            expectEquals(snapshot1.getPixelAt(40, 12), juce::Colours::royalblue);

            auto snapshot2 = btn2.createComponentSnapshot(btn2.getLocalBounds());
            expectEquals(snapshot2.getPixelAt(40, 12), juce::Colours::crimson);

            // Active states
            btn1.getProperties().set("jive::active", true);
            btn1.lookAndFeelChanged();
            snapshot1 = btn1.createComponentSnapshot(btn1.getLocalBounds());
            expectEquals(snapshot1.getPixelAt(40, 12), juce::Colours::navy);

            btn2.getProperties().set("jive::active", true);
            btn2.lookAndFeelChanged();
            snapshot2 = btn2.createComponentSnapshot(btn2.getLocalBounds());
            expectEquals(snapshot2.getPixelAt(40, 12), juce::Colours::darkred);
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
            auto snapshot = deepButton.createComponentSnapshot(deepButton.getLocalBounds());
            expectEquals(snapshot.getPixelAt(40, 12), juce::Colours::gold);

            // Hover state
            deepButton.getProperties().set("jive::hover", true);
            deepButton.lookAndFeelChanged();
            snapshot = deepButton.createComponentSnapshot(deepButton.getLocalBounds());
            expectEquals(snapshot.getPixelAt(40, 12), juce::Colours::goldenrod);
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
            auto snapshot = button.createComponentSnapshot(button.getLocalBounds());
            expectEquals(snapshot.getPixelAt(40, 12), juce::Colours::purple);

            // Hover only
            button.getProperties().set("jive::hover", true);
            button.lookAndFeelChanged();
            snapshot = button.createComponentSnapshot(button.getLocalBounds());
            expectEquals(snapshot.getPixelAt(40, 12), juce::Colours::mediumpurple);

            // Hover and active combined
            button.getProperties().set("jive::active", true);
            button.lookAndFeelChanged();
            snapshot = button.createComponentSnapshot(button.getLocalBounds());
            expectEquals(snapshot.getPixelAt(40, 12), juce::Colours::rebeccapurple);
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
            auto snapshot = button.createComponentSnapshot(button.getLocalBounds());
            expectEquals(snapshot.getPixelAt(40, 12), juce::Colours::olive);

            // Hover state
            button.getProperties().set("jive::hover", true);
            button.lookAndFeelChanged();
            snapshot = button.createComponentSnapshot(button.getLocalBounds());
            expectEquals(snapshot.getPixelAt(40, 12), juce::Colours::olivedrab);

            // Active state (remove hover first to test active independently)
            button.getProperties().remove("jive::hover");
            button.getProperties().set("jive::active", true);
            button.lookAndFeelChanged();
            snapshot = button.createComponentSnapshot(button.getLocalBounds());
            expectEquals(snapshot.getPixelAt(40, 12), juce::Colours::darkolivegreen);
        }
    }
};

static StyleSheetTests styleSheetTests;
#endif
