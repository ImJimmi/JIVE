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
                                                [safeComponent = juce::Component::SafePointer<juce::Component>{ &component }](const juce::Component& c) {
                                                    return safeComponent != nullptr && safeComponent.getComponent() == &c;
                                                },
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
        for (const auto& [name, uuid] : uuids)
            lookAndFeel.removeStyles(uuid);

        for (const auto& [id, property] : observedObjects)
        {
            juce::ignoreUnused(id);
            property.get()->removeListener(*this);
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

    void StyleSheet::stylePropertyChanged()
    {
        if (!style.exists() || style.get() == nullptr)
        {
            for (const auto& [name, uuid] : uuids)
                lookAndFeel.removeStyles(uuid);

            uuids.clear();

            return;
        }

        addStylesFrom(*style.get());
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

    void StyleSheet::addStylesFrom(Object& object, InteractionState interationState)
    {
        addStylesFrom(
            object,
            [safeComponent = juce::Component::SafePointer<juce::Component>{ &component }](const juce::Component& comp) {
                return safeComponent != nullptr && safeComponent.getComponent() == &comp;
            },
            interationState,
            painterPrecedence::componentPredicate);
    }

    void StyleSheet::addStylesFrom(Object& object,
                                   LookAndFeel::ComponentPredicate predicate,
                                   InteractionState interationState,
                                   LookAndFeel::Precedence precedence)
    {
        const auto id = calculateStylesID(object);
        const auto addStyles = [this, id, &object, predicate, interationState, precedence]() {
            std::optional<Styles> styles;

            if (uuids.count(id))
            {
                styles = *lookAndFeel.findStyles(uuids[id]);
                lookAndFeel.removeStyles(uuids[id]);
                uuids.erase(id);
            }

            findAndAddStyleProperties(object,
                                      id,
                                      styles,
                                      predicate,
                                      interationState,
                                      precedence);
            findAndAddStatefulStyles(object,
                                     predicate,
                                     interationState,
                                     precedence);
            findAndAddOtherStyles(object);

            component.lookAndFeelChanged();
            component.repaint();
        };

        if (observedObjects.count(id) == 0 || observedObjects.at(id).get().get() != &object)
        {
            StyleProperty* newProperty;
            observedObjects.erase(id);

            jassert(observedObjects.count(id) == 0);

            if (auto* parent = object.getParent())
            {
                const auto [entry, wasAdded] = observedObjects.emplace(id, StyleProperty{ parent, object.findNameInParent() });
                newProperty = &entry->second;
            }
            else if (&object == style.get().get())
            {
                const auto [entry, wasAdded] = observedObjects.emplace(id, StyleProperty{ state, "style" });
                newProperty = &entry->second;
            }
            else
            {
                jassertfalse;
            }

            newProperty->get()->addListener(*this);
            newProperty->onValueChange = addStyles;
        }

        addStyles();
    }

    void StyleSheet::findAndAddStyleProperties(Object& object,
                                               const juce::String& id,
                                               std::optional<Styles>& styles,
                                               LookAndFeel::ComponentPredicate predicate,
                                               InteractionState interationState,
                                               LookAndFeel::Precedence precedence)
    {
        auto newStylesAdded = false;

        for (const auto& propertyName : Styles::propertyNames)
        {
            if (!object.hasProperty(propertyName))
                continue;

            styles = styles
                         .value_or(Styles{})
                         .with(propertyName, object.getProperty(propertyName));
            newStylesAdded = true;
        }

        if (&object == style.get().get() && object.hasProperty("transition"))
        {
            auto transitions = fromVar<Transitions::ReferenceCountedPointer>(object["transition"]);

            if (transitions != nullptr)
            {
                styles = styles
                             .value_or(Styles{})
                             .withTransitions(transitions);
                newStylesAdded = true;

                vBlank = std::make_unique<juce::VBlankAttachment>(&component, [this, transitions]() {
                    component.repaint();
                });
            }
        }

        if (styles.has_value() && newStylesAdded)
        {
            uuids[id] = lookAndFeel.addStyles(
                predicate,
                *styles,
                interationState,
                precedence);
        }
    }

    void StyleSheet::findAndAddStatefulStyles(Object& object,
                                              LookAndFeel::ComponentPredicate predicate,
                                              InteractionState interationState,
                                              LookAndFeel::Precedence precedence)
    {
        for (const auto& stateName : InteractionState::propertyNames)
        {
            if (!object.hasProperty(stateName))
                continue;

            if (auto* child = dynamic_cast<Object*>(object[stateName].getDynamicObject()))
            {
                auto newState = interationState;
                newState.set(stateName);
                addStylesFrom(*child, predicate, newState, precedence);
            }
        }
    }

    void StyleSheet::findAndAddOtherStyles(Object& object)
    {
        for (const auto& [name, value] : object.getProperties())
        {
            if (Styles::propertyNames.count(name.toString()) > 0 || InteractionState::propertyNames.count(name.toString()) > 0)
                continue;

            auto child = dynamic_cast<Object*>(value.getDynamicObject());

            if (child == nullptr)
                continue;

            if (name.toString().startsWith("#"))
            {
                addStylesFrom(
                    *child,
                    [safeComponent = juce::Component::SafePointer<juce::Component>{ &component },
                     compID = name.toString().fromFirstOccurrenceOf("#", false, false)](const juce::Component& comp) {
                        return comp.getComponentID() == compID
                            && safeComponent != nullptr
                            && (safeComponent.getComponent() == &comp
                                || safeComponent->isParentOf(&comp));
                    },
                    InteractionState{},
                    painterPrecedence::componentID);
            }
            else if (name.toString().startsWith("."))
            {
                addStylesFrom(
                    *child,
                    [safeComponent = juce::Component::SafePointer<juce::Component>{ &component },
                     uiState = state,
                     className = name.toString().fromFirstOccurrenceOf(".", false, false)](const juce::Component& comp) {
                        const auto classes = fromVar<juce::StringArray>(uiState["class"]);

                        return classes.contains(className)
                            && safeComponent != nullptr
                            && (safeComponent.getComponent() == &comp
                                || safeComponent->isParentOf(&comp));
                    },
                    InteractionState{},
                    painterPrecedence::componentClass);
            }
            else
            {
                addStylesFrom(
                    *child,
                    [safeComponent = juce::Component::SafePointer<juce::Component>{ &component },
                     typeName = name.toString()](const juce::Component& comp) {
                        return ComponentTypePredicates::check(comp, typeName)
                            && safeComponent != nullptr
                            && (safeComponent.getComponent() == &comp
                                || safeComponent->isParentOf(&comp));
                    },
                    InteractionState{},
                    painterPrecedence::componentType);
            }
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
                state.setProperty("style", juce::var{}, nullptr);
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
};

static StyleSheetTests styleSheetTests;
#endif
