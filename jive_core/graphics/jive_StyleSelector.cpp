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

    [[nodiscard]] static std::function<bool(const juce::Component&)> buildPredicate(juce::String selector)
    {
        if (selector.isEmpty())
        {
            // Assume we were given an empty string by one of the recursive
            // steps below. We need to break the recursion so just return true
            // here for any previous predicates to take effect (affect?).
            return [](const juce::Component&) {
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
                return [remainingPredicate = buildPredicate(selector)](const juce::Component& component) {
                    return remainingPredicate(component);
                };
            }
            if (character == '#')
            {
                const auto id = removeLastCharacters(selector, selector.length() - i - 1);
                removeLastCharacters(selector, 1);

                return [id, remainingPredicate = buildPredicate(selector)](const juce::Component& component) {
                    return component.getComponentID() == id
                        && remainingPredicate(component);
                };
            }
            if (character == '.')
            {
                const auto className = removeLastCharacters(selector, selector.length() - i - 1);
                removeLastCharacters(selector, 1);

                return [className, remainingPredicate = buildPredicate(selector)](const juce::Component& component) {
                    return fromVar<juce::StringArray>(component.getProperties().getWithDefault("class", ""))
                               .contains(className)
                        && remainingPredicate(component);
                };
            }
            if (character == ':')
            {
                const auto stateName = removeLastCharacters(selector, selector.length() - i - 1);
                removeLastCharacters(selector, 1);

                return [stateName, remainingPredicate = buildPredicate(selector)](const juce::Component& component) {
                    return InteractionState{}.set(stateName).appliesTo(component)
                        && remainingPredicate(component);
                };
            }
            if (character == ' ' || i == 0)
            {
                const auto typeName = i == 0
                                        ? selector
                                        : removeLastCharacters(selector, selector.length() - i - 1);

                if (i != 0)
                    removeLastCharacters(selector, 1);

                if (!typeName.isEmpty())
                {
                    return [typeName, remainingPredicate = buildPredicate(selector)](const juce::Component& component) {
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
                    return [ancestorPredicate = buildPredicate(selector)](const juce::Component& component) {
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

                return [parentPredicate = buildPredicate(selector)](const juce::Component& component) {
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

    StyleSelector::StyleSelector(const juce::String& selector)
        : predicate{ buildPredicate(selector) }
    {
    }

    StyleSelector& StyleSelector::operator=(const juce::String& selector)
    {
        predicate = buildPredicate(selector);
        return *this;
    }

    bool StyleSelector::appliesTo(const juce::Component& component) const
    {
        return predicate(component);
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
#endif
