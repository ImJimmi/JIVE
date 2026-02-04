#include "jive_StyleSelector.h"

#include "jive_ComponentTypePredicates.h"

#include <jive_core/interface/jive_InteractionState.h>
#include <jive_core/values/variant-converters/jive_VariantConvertion.h>

namespace jive
{
    static auto removeLastCharacters(juce::String& str, int N)
    {
        const auto result = str.getLastCharacters(N);
        str = str.dropLastCharacters(N);
        return result;
    }

    [[nodiscard]] static std::function<bool(const juce::Component&)> buildSinglePredicate(juce::String selector,
                                                                                          juce::Component* root)
    {
        if (selector.isEmpty())
        {
            // Assume we were given an empty string by one of the recursive
            // steps below. This will be the final predicate layer so we should
            // check if the root component if one has been specified, or just
            // return true to break the recursion.
            return [rootComponent = juce::Component::SafePointer<juce::Component>{ root }](const juce::Component& component) {
                if (rootComponent != nullptr)
                    return rootComponent->isParentOf(&component) || rootComponent == &component;

                return true;
            };
        }

        for (auto i = selector.length() - 1; i >= 0; i--)
        {
            // Ensure we're not reading off the end of the string in case the
            // size of the selector changed during the previous iteration
            i = juce::jmin(i, selector.length() - 1);
            auto character = selector[i];

            if (character == '*')
            {
                removeLastCharacters(selector, 1);
                return [remainingPredicate = buildSinglePredicate(selector, root)](const juce::Component& component) {
                    return remainingPredicate(component);
                };
            }
            if (character == '#')
            {
                const auto id = removeLastCharacters(selector, selector.length() - i - 1);
                removeLastCharacters(selector, 1);

                return [id, remainingPredicate = buildSinglePredicate(selector, root)](const juce::Component& component) {
                    return component.getComponentID() == id
                        && remainingPredicate(component);
                };
            }
            if (character == '.')
            {
                const auto className = removeLastCharacters(selector, selector.length() - i - 1);
                removeLastCharacters(selector, 1);

                return [className, remainingPredicate = buildSinglePredicate(selector, root)](const juce::Component& component) {
                    return fromVar<juce::StringArray>(component.getProperties().getWithDefault("class", ""))
                               .contains(className)
                        && remainingPredicate(component);
                };
            }
            if (character == ':')
            {
                const auto stateName = removeLastCharacters(selector, selector.length() - i - 1);
                removeLastCharacters(selector, 1);

                return [stateName, remainingPredicate = buildSinglePredicate(selector, root)](const juce::Component& component) {
                    return InteractionState{}.set(stateName).appliesTo(component)
                        && remainingPredicate(component);
                };
            }
            if (character == ' ' || i == 0)
            {
                const auto typeName = i == 0
                                        ? removeLastCharacters(selector, selector.length() - i)
                                        : removeLastCharacters(selector, selector.length() - i - 1);

                if (i != 0)
                    removeLastCharacters(selector, 1);

                if (!typeName.isEmpty())
                {
                    return [typeName, remainingPredicate = buildSinglePredicate(selector, root)](const juce::Component& component) {
                        return ComponentTypePredicates::check(component, typeName)
                            && remainingPredicate(component);
                    };
                }

                // The space was at the end of the string meaning the
                // remaining selector applies to ancestor components
                selector.trim();
                character = selector.getLastCharacter();

                if (character != '>')
                {
                    // The next character after thr space isn't a '>' so the
                    // remaining selector applies to any of the component's
                    // ancestors
                    return [ancestorPredicate = buildSinglePredicate(selector, root)](const juce::Component& component) {
                        for (auto* ancestor = component.getParentComponent();
                             ancestor != nullptr;
                             ancestor = ancestor->getParentComponent())
                        {
                            if (ancestorPredicate(*ancestor))
                                return true;
                        }

                        return false;
                    };
                }

                // Let this fallthrough here so we can handle '>' characters
                // with and without spaces surrounding them
            }

            if (character == '>')
            {
                removeLastCharacters(selector, 1);
                selector = selector.trimEnd();

                return [parentPredicate = buildSinglePredicate(selector, root)](const juce::Component& component) {
                    return component.getParentComponent() != nullptr
                        && parentPredicate(*component.getParentComponent());
                };
            }
        }

        // Somehow got to the front of the string without having built a
        // predicate for it!
        jassertfalse;
        return [](const juce::Component&) {
            return false;
        };
    }

    [[nodiscard]] static std::function<bool(const juce::Component&)> buildPredicate(juce::String selector, juce::Component* root)
    {
        // Handle comma-separated selectors (multiple selectors)
        const auto selectors = juce::StringArray::fromTokens(selector, ",", "");

        if (selectors.size() > 1)
        {
            std::vector<std::function<bool(const juce::Component&)>> predicates;
            predicates.reserve(static_cast<size_t>(selectors.size()));

            for (const auto& singleSelector : selectors)
                predicates.push_back(buildSinglePredicate(singleSelector.trim(), root));

            return [predicates](const juce::Component& component) {
                for (const auto& predicate : predicates)
                {
                    if (predicate(component))
                        return true;
                }
                return false;
            };
        }

        return buildSinglePredicate(selector, root);
    }

    [[nodiscard]] static auto calculatePrecedence(const juce::String& selector)
    {
        // [0] = IDs, [1] = classes/attributes/pseudo-classes, [2] = elements/pseudo-elements
        std::array<std::uint16_t, 3> precedence{ 0, 0, 0 };

        static constexpr auto isIdentStart = [](auto character) {
            return juce::CharacterFunctions::isLetter(character)
                || character == '-'
                || character == '_';
        };
        static constexpr auto isIdentChar = [](auto character) {
            return isIdentStart(character) || juce::CharacterFunctions::isDigit(character);
        };

        const auto skipIdentifier = [&selector](int& i) {
            while (i < selector.length() && isIdentChar(selector[i]))
                i++;
        };

        const auto skipUntilAfter = [&selector](int& i, juce::juce_wchar terminator) {
            while (i < selector.length() && selector[i] != terminator)
                i++;
            if (i < selector.length())
                i++; // consume terminator
        };

        for (int i = 0; i < selector.length();)
        {
            auto character = selector[i];

            // Ignore whitespace and combinators
            if (juce::CharacterFunctions::isWhitespace(character)
                || character == '>'
                || character == '+'
                || character == '~'
                || character == ',')
            {
                i++;
                continue;
            }

            // Universal selector: zero specificity
            if (character == '*')
            {
                i++;
                continue;
            }

            // ID selector
            if (character == '#')
            {
                precedence[0]++;
                i++;
                skipIdentifier(i);
                continue;
            }

            // Class selector
            if (character == '.')
            {
                precedence[1]++;
                i++;
                skipIdentifier(i);
                continue;
            }

            // Attribute selector
            if (character == '[')
            {
                precedence[1]++;
                i++;
                skipUntilAfter(i, ']');
                continue;
            }

            // Pseudo-class / pseudo-element
            if (character == ':')
            {
                i++;

                if (i < selector.length() && selector[i] == ':')
                {
                    // Pseudo-element
                    precedence[2]++;
                    i++;
                }
                else
                {
                    // Pseudo-class
                    precedence[1]++;
                }

                skipIdentifier(i);

                // Handle functional pseudos like :not(...)
                if (i < selector.length() && selector[i] == '(')
                    skipUntilAfter(i, ')');

                continue;
            }

            // Element selector (identifier start)
            if (isIdentStart(character))
            {
                precedence[2]++;
                skipIdentifier(i);
                continue;
            }

            // Anything else: just advance
            i++;
        }

        return precedence;
    }

    StyleSelector::StyleSelector(const juce::String& selector, juce::Component* root)
        : predicate{ buildPredicate(selector, root) }
        , precedence{ calculatePrecedence(selector) }
    {
    }

    StyleSelector::StyleSelector(const char* selector, juce::Component* root)
        : StyleSelector{ juce::String{ selector }, root }
    {
    }

    bool StyleSelector::appliesTo(const juce::Component& component) const
    {
        return predicate(component);
    }

    std::array<std::uint16_t, 3> StyleSelector::getPrecedence() const
    {
        return precedence;
    }

    bool StyleSelector::operator<(const StyleSelector& other) const
    {
        if (isSystem && !other.isSystem)
            return false;
        if (other.isSystem && !isSystem)
            return true;

        return precedence > other.precedence;
    }

    bool StyleSelector::operator>(const StyleSelector& other) const
    {
        return other < *this;
    }

    bool StyleSelector::operator==(const StyleSelector& other) const
    {
        return precedence == other.precedence && isSystem == other.isSystem;
    }

    StyleSelector StyleSelector::makeInlineStyleSelector(const juce::Component& component)
    {
        StyleSelector selector{ "*" };
        selector.predicate = [safeComponent = juce::Component::SafePointer<juce::Component>{ const_cast<juce::Component*>(&component) }](const juce::Component& other) {
            return safeComponent != nullptr && safeComponent == &other;
        };

        return selector;
    }

    StyleSelector StyleSelector::makeSystemStyleSelector(const juce::String& pattern)
    {
        StyleSelector selector{ pattern };
        selector.isSystem = true;

        return selector;
    }
} // namespace jive

#if JIVE_UNIT_TESTS
    #include <jive_core/testing/jive_UnitTest.h>

static jive::UnitTest singleSelectorsTest{
    "jive",
    "jive::StyleSelector",
    "Single selectors",
    [](auto& test) {
        juce::Component component;
        component.setComponentID("my-comp");
        component.getProperties().set("class", "foo bar");

        {
            const jive::StyleSelector selector{ "#my-comp" };
            test.expect(selector.appliesTo(component));
        }

        {
            const jive::StyleSelector selector{ ".foo" };
            test.expect(selector.appliesTo(component));
        }

        {
            const jive::StyleSelector selector{ ".bar" };
            test.expect(selector.appliesTo(component));
        }

        {
            const jive::StyleSelector selector{ "#not-mine" };
            test.expect(!selector.appliesTo(component));
        }

        {
            const jive::StyleSelector selector{ ".fizz" };
            test.expect(!selector.appliesTo(component));
        }
    },
};

static jive::UnitTest directChildSelectorTest{
    "jive",
    "jive::StyleSelector",
    "Direct child selector",
    [](auto& test) {
        juce::Component topLevel;
        topLevel.setComponentID("top-level");
        juce::Component container;
        container.setComponentID("container");
        topLevel.addAndMakeVisible(container);
        juce::Component child;
        child.setComponentID("child");
        container.addAndMakeVisible(child);

        {
            const jive::StyleSelector selector{ "#top-level > #container" };
            test.expect(!selector.appliesTo(topLevel));
            test.expect(selector.appliesTo(container));
            test.expect(!selector.appliesTo(child));
        }
        {
            const jive::StyleSelector selector{ "#top-level > #child" };
            test.expect(!selector.appliesTo(topLevel));
            test.expect(!selector.appliesTo(container));
            test.expect(!selector.appliesTo(child));
        }
        {
            const jive::StyleSelector selector{ "#container > #child" };
            test.expect(!selector.appliesTo(topLevel));
            test.expect(!selector.appliesTo(container));
            test.expect(selector.appliesTo(child));
        }
        {
            const jive::StyleSelector selector{ "#top-level > #container > #child" };
            test.expect(!selector.appliesTo(topLevel));
            test.expect(!selector.appliesTo(container));
            test.expect(selector.appliesTo(child));
        }
    },
};

static jive::UnitTest indirectChildSelectorTest{
    "jive",
    "jive::StyleSelector",
    "Indirect child selector",
    [](auto& test) {
        juce::Component topLevel;
        topLevel.setComponentID("top-level");
        juce::Component container;
        container.setComponentID("container");
        topLevel.addAndMakeVisible(container);
        juce::Component child;
        child.setComponentID("child");
        container.addAndMakeVisible(child);

        {
            const jive::StyleSelector selector{ "#top-level #container" };
            test.expect(!selector.appliesTo(topLevel));
            test.expect(selector.appliesTo(container));
            test.expect(!selector.appliesTo(child));
        }
        {
            const jive::StyleSelector selector{ "#top-level #child" };
            test.expect(!selector.appliesTo(topLevel));
            test.expect(!selector.appliesTo(container));
            test.expect(selector.appliesTo(child));
        }
        {
            const jive::StyleSelector selector{ "#container #child" };
            test.expect(!selector.appliesTo(topLevel));
            test.expect(!selector.appliesTo(container));
            test.expect(selector.appliesTo(child));
        }
        {
            const jive::StyleSelector selector{ "#top-level #container #child" };
            test.expect(!selector.appliesTo(topLevel));
            test.expect(!selector.appliesTo(container));
            test.expect(selector.appliesTo(child));
        }
    },
};

static jive::UnitTest stateSelectors{
    "jive",
    "jive::StyleSelector",
    "State selectors",
    [](auto& test) {
        juce::Component component;

        {
            const jive::StyleSelector selector{ ":hover" };
            test.expect(!selector.appliesTo(component));
            component.getProperties().set("jive::hover", true);
            test.expect(selector.appliesTo(component));
            component.getProperties().remove("jive::hover");
        }
        {
            const jive::StyleSelector selector{ ":active" };
            test.expect(!selector.appliesTo(component));
            component.getProperties().set("jive::active", true);
            test.expect(selector.appliesTo(component));
            component.getProperties().remove("jive::active");
        }
        {
            const jive::StyleSelector selector{ ":hover:focus" };
            test.expect(!selector.appliesTo(component));
            component.getProperties().set("jive::hover", true);
            component.getProperties().set("jive::focus", true);
            test.expect(selector.appliesTo(component));
            component.getProperties().remove("jive::hover");
            component.getProperties().remove("jive::focus");
        }
    },
};

static jive::UnitTest complexSelectors{
    "jive",
    "jive::StyleSelector",
    "Complex selectors",
    [](auto& test) {
        juce::Component topLevel;
        juce::Component left;
        juce::Component right;
        juce::TextButton button1;
        juce::TextButton button2;

        topLevel.setComponentID("top-level");
        topLevel.setSize(100, 100);
        topLevel.addAndMakeVisible(left);
        topLevel.addAndMakeVisible(right);
        left.setBounds(0, 0, 50, 100);
        right.setBounds(50, 0, 50, 100);

        left.setComponentID("left");
        left.addAndMakeVisible(button1);
        button1.setBounds(10, 10, 30, 15);
        button1.getProperties().set("class", "btn");

        right.setComponentID("right");
        right.addAndMakeVisible(button2);
        button2.setBounds(10, 10, 30, 15);
        button2.getProperties().set("class", "btn");

        {
            const jive::StyleSelector selector{ "#top-level > *" };
            test.expect(!selector.appliesTo(topLevel));
            test.expect(selector.appliesTo(left));
            test.expect(selector.appliesTo(right));
            test.expect(!selector.appliesTo(button1));
            test.expect(!selector.appliesTo(button2));
        }
        {
            const jive::StyleSelector selector{ "#top-level *" };
            test.expect(!selector.appliesTo(topLevel));
            test.expect(selector.appliesTo(left));
            test.expect(selector.appliesTo(right));
            test.expect(selector.appliesTo(button1));
            test.expect(selector.appliesTo(button2));
        }
        {
            const jive::StyleSelector selector{ "#top-level > .btn" };
            test.expect(!selector.appliesTo(topLevel));
            test.expect(!selector.appliesTo(left));
            test.expect(!selector.appliesTo(right));
            test.expect(!selector.appliesTo(button1));
            test.expect(!selector.appliesTo(button2));
        }
        {
            const jive::StyleSelector selector{ "#top-level > * > .btn" };
            test.expect(!selector.appliesTo(topLevel));
            test.expect(!selector.appliesTo(left));
            test.expect(!selector.appliesTo(right));
            test.expect(selector.appliesTo(button1));
            test.expect(selector.appliesTo(button2));
        }
        {
            const jive::StyleSelector selector{ "#left .btn" };
            test.expect(!selector.appliesTo(topLevel));
            test.expect(!selector.appliesTo(left));
            test.expect(!selector.appliesTo(right));
            test.expect(selector.appliesTo(button1));
            test.expect(!selector.appliesTo(button2));
        }
        {
            const jive::StyleSelector selector{ "#right > .btn" };
            test.expect(!selector.appliesTo(topLevel));
            test.expect(!selector.appliesTo(left));
            test.expect(!selector.appliesTo(right));
            test.expect(!selector.appliesTo(button1));
            test.expect(selector.appliesTo(button2));
        }
    },
};

static jive::UnitTest selectorPrecedence{
    "jive",
    "jive::StyleSelector",
    "Selector precedence",
    [](auto& test) {
        test.expect(jive::StyleSelector{ "*" } > jive::StyleSelector{ "Component" });
        test.expect(jive::StyleSelector{ "*" } > jive::StyleSelector{ ".foo" });
        test.expect(jive::StyleSelector{ "*" } > jive::StyleSelector{ "#foo" });
        test.expect(jive::StyleSelector{ "*" } > jive::StyleSelector{ "Component::before" });

        test.expect(jive::StyleSelector{ "Component" } > jive::StyleSelector{ "Component.foo" });
        test.expect(jive::StyleSelector{ "Component" } > jive::StyleSelector{ ".foo" });
        test.expect(jive::StyleSelector{ "Component" } > jive::StyleSelector{ "#foo" });

        test.expect(jive::StyleSelector{ "div span" } > jive::StyleSelector{ ".foo" });
        test.expect(jive::StyleSelector{ "ul li a" } > jive::StyleSelector{ ".nav-link" });
        test.expect(jive::StyleSelector{ "p em strong" } > jive::StyleSelector{ ".highlight" });

        test.expect(jive::StyleSelector{ ".foo" } > jive::StyleSelector{ ".foo.bar" });
        test.expect(jive::StyleSelector{ ".foo" } > jive::StyleSelector{ "#foo" });
        test.expect(jive::StyleSelector{ ".foo" } > jive::StyleSelector{ ".foo::before" });

        test.expect(jive::StyleSelector{ "[type=text]" } > jive::StyleSelector{ ".foo.bar" });
        test.expect(jive::StyleSelector{ "[data-x]" } > jive::StyleSelector{ "#foo" });

        test.expect(jive::StyleSelector{ "div::before" } > jive::StyleSelector{ "div:hover" });
        test.expect(jive::StyleSelector{ "div::before" } > jive::StyleSelector{ ".foo" });
        test.expect(jive::StyleSelector{ ".foo::before" } > jive::StyleSelector{ ".foo:hover" });

        test.expect(jive::StyleSelector{ "div.foo.bar span" } > jive::StyleSelector{ "#app div.foo.bar span" });
        test.expect(jive::StyleSelector{ "#app div.foo span::before" } > jive::StyleSelector{ "#app #content div.foo span:hover::before" });
    },
};

static jive::UnitTest multipleSelectors{
    "jive",
    "jive::StyleSelector",
    "Multiple selectors (comma-separated)",
    [](auto& test) {
        juce::Component component1;
        component1.setComponentID("first");
        component1.getProperties().set("class", "foo");

        juce::Component component2;
        component2.setComponentID("second");
        component2.getProperties().set("class", "bar");

        juce::Component component3;
        component3.setComponentID("third");
        component3.getProperties().set("class", "baz");

        // Basic comma-separated class selectors
        {
            const jive::StyleSelector selector{ ".foo, .bar" };
            test.expect(selector.appliesTo(component1));
            test.expect(selector.appliesTo(component2));
            test.expect(!selector.appliesTo(component3));
        }

        // Comma-separated ID selectors
        {
            const jive::StyleSelector selector{ "#first, #second" };
            test.expect(selector.appliesTo(component1));
            test.expect(selector.appliesTo(component2));
            test.expect(!selector.appliesTo(component3));
        }

        // Mixed ID and class selectors
        {
            const jive::StyleSelector selector{ "#first, .bar" };
            test.expect(selector.appliesTo(component1));
            test.expect(selector.appliesTo(component2));
            test.expect(!selector.appliesTo(component3));
        }

        // Three comma-separated selectors
        {
            const jive::StyleSelector selector{ ".foo, .bar, .baz" };
            test.expect(selector.appliesTo(component1));
            test.expect(selector.appliesTo(component2));
            test.expect(selector.appliesTo(component3));
        }

        // Comma-separated with complex selectors
        {
            juce::Component parent;
            parent.setComponentID("parent");
            parent.addAndMakeVisible(component1);
            parent.addAndMakeVisible(component2);

            const jive::StyleSelector selector{ "#parent > .foo, #parent > .bar" };
            test.expect(selector.appliesTo(component1));
            test.expect(selector.appliesTo(component2));
            test.expect(!selector.appliesTo(component3));
        }

        // Comma-separated with no spaces around comma
        {
            const jive::StyleSelector selector{ ".foo,.bar" };
            test.expect(selector.appliesTo(component1));
            test.expect(selector.appliesTo(component2));
            test.expect(!selector.appliesTo(component3));
        }

        // Comma-separated with extra spaces
        {
            const jive::StyleSelector selector{ ".foo ,  .bar" };
            test.expect(selector.appliesTo(component1));
            test.expect(selector.appliesTo(component2));
            test.expect(!selector.appliesTo(component3));
        }
    },
};
#endif
