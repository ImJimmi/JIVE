#pragma once

#include <jive_layouts/jive_layouts.h>

#include <shared/Inspector.h>
#include <shared/Presenter.h>
#include <shared/Theme.h>

namespace jive_examples::style_sheets
{
    class Presenter : public BasePresenter
    {
    public:
        Presenter()
        {
            window = interpreter.interpret(mainView);
            interpreter.onExternalSourceLoaded = [this](const jive::SourceFile& file) {
                if (file == mainView)
                    bindWindow();

                bindPage();
                inspector.inspect(*window->getComponent());
            };
            bindWindow();
            bindPage();
            inspector.inspect(*window->getComponent());
        }

    private:
        // A control that edits a property of a rule, or a token when there's no
        // rule, in the style sheet of the element with the given ID, or of the
        // window when there's no ID.
        struct Control
        {
            juce::String id;
            juce::String sheet;
            juce::String rule;
            juce::Identifier property;
            juce::String format = "{}";
        };

        void bindWindow()
        {
            windowStyleSheet = observeStyleSheet(window->state);

            tabs.clear();

            for (const auto& [tab, view] : pages)
            {
                onToggled(tabs, tab, [this, source = view](bool toggled) {
                    if (toggled)
                        setProperty("page", "source", source);
                });
            }

            theme = bindThemeComboBox(window->state);
        }

        // Once loaded, an element's style property holds its parsed style
        // sheet, which the controls edit directly. Saving the file swaps in a
        // newly parsed object, which the controls then show.
        [[nodiscard]] std::unique_ptr<jive::Property<jive::Object::ReferenceCountedPointer>> observeStyleSheet(const juce::ValueTree& element)
        {
            auto styleSheet = std::make_unique<jive::Property<jive::Object::ReferenceCountedPointer>>(element, "style");
            styleSheet->onValueChange = [this] {
                showPage();
            };

            return styleSheet;
        }

        void bindPage()
        {
            toggles.clear();
            values.clear();
            pageStyleSheet = nullptr;

            if (const auto tokens = jive::findElementWithID(window->state, "tokens"); tokens.isValid())
                pageStyleSheet = observeStyleSheet(tokens);

            for (const auto& control : controls)
                bindControl(control);

            bindSelectors();
            bindSpecificity();

            onToggled(toggles, "enabled-checkbox", [this](bool enabled) {
                setProperty("state-button", "enabled", enabled);
            });

            showPage();
        }

        void showPage()
        {
            for (const auto& control : controls)
                showControl(control);

            showSelectors();
            showRules();
        }

        void bindControl(const Control& control)
        {
            const auto element = jive::findElementWithID(window->state, control.id);

            if (!element.isValid())
                return;

            const auto isComboBox = element.hasType("ComboBox");
            auto& value = *values.emplace_back(std::make_unique<jive::Property<int>>(element, isComboBox ? "selected" : "value"));
            value.onValueChange = [this, &value, control, element, isComboBox] {
                const auto newValue = isComboBox ? element.getChild(value.get())["text"] : juce::var{ value.get() };

                if (!newValue.isVoid())
                    setRuleProperty(control, newValue);
            };

            if (isComboBox && launchedComboBoxes.addIfNotAlreadyThere(control.id))
                setRuleProperty(control, element.getChild(0)["text"]);
        }

        void showControl(const Control& control)
        {
            auto element = jive::findElementWithID(window->state, control.id);
            const auto value = getRuleProperty(control);

            if (!element.isValid() || value.isVoid())
                return;

            if (element.hasType("ComboBox"))
            {
                element.setProperty("selected", indexOfOption(control.id, value), nullptr);
                return;
            }

            element.setProperty("value", value, nullptr);
            setProperty(control.id.replace("-slider", "-label"), "text", control.format.replace("{}", value.toString()));
        }

        // The #matches rule holds a single nested rule, which each button moves
        // to its own selector. Only the button showing the rule's current
        // selector stays toggled on.
        void bindSelectors()
        {
            for (auto button : jive::findElementsWithClass(window->state, "selector"))
            {
                onToggled(toggles, button, [this, selector = button.getChild(0)["text"].toString()](bool toggled) {
                    if (toggled)
                        moveMatchesRule(selector);

                    showSelectors();
                });
            }
        }

        void showSelectors()
        {
            for (auto button : jive::findElementsWithClass(window->state, "selector"))
                button.setProperty("toggled", button.getChild(0)["text"].toString() == findMatchSelector(), nullptr);
        }

        void moveMatchesRule(const juce::String& selector)
        {
            auto* matches = findRule({}, "#matches");
            const auto previous = findMatchSelector();

            if (matches == nullptr || previous.isEmpty() || previous == selector)
                return;

            const auto rule = matches->getProperty(previous);
            matches->removeProperty(previous);
            matches->setProperty(selector, rule);
        }

        [[nodiscard]] juce::String findMatchSelector() const
        {
            if (const auto* matches = findRule({}, "#matches"))
            {
                for (const auto& [selector, rule] : matches->getProperties())
                {
                    if (rule.getDynamicObject() != nullptr)
                        return selector.toString();
                }
            }

            return {};
        }

        void bindSpecificity()
        {
            onToggled(toggles, "class-checkbox", [this](bool) {
                showSpecificity();
            });
            onToggled(toggles, "id-checkbox", [this](bool) {
                showSpecificity();
            });
        }

        void showSpecificity()
        {
            auto button = jive::findElementWithID(window->state, "specificity").getChild(0);
            const auto hasClass = isToggled("class-checkbox");
            const auto hasID = isToggled("id-checkbox");

            button.setProperty("class", hasClass ? "primary" : "", nullptr);
            button.setProperty("id", hasID ? "save" : "", nullptr);
            button.getChild(0).setProperty("text",
                                           juce::String{ "Button" }
                                               + (hasID ? "#save" : "")
                                               + (hasClass ? ".primary" : ""),
                                           nullptr);
        }

        // Shows the rules named by a readout's `rule` attribute, or its whole
        // style sheet when it doesn't have one.
        void showRules()
        {
            for (auto readout : jive::findElementsWithClass(window->state, "rule"))
            {
                auto* styleSheet = findStyleSheet(readout["sheet"].toString());

                if (styleSheet == nullptr)
                    continue;

                const auto selectors = juce::StringArray::fromTokens(readout["rule"].toString(), ",", "");
                juce::StringArray lines;

                for (const auto& selector : selectors)
                {
                    lines.add(juce::JSON::toString(selector.trim())
                              + ": "
                              + juce::JSON::toString(styleSheet->getProperty(selector.trim())));
                }

                if (selectors.isEmpty())
                    lines.add(juce::JSON::toString(juce::var{ styleSheet }));

                readout.setProperty("text", lines.joinIntoString(",\n").replace("\r\n", "\n"), nullptr);
            }
        }

        [[nodiscard]] jive::Object* findStyleSheet(const juce::String& elementID) const
        {
            const auto element = elementID.isEmpty()
                                   ? window->state
                                   : jive::findElementWithID(window->state, elementID);

            return dynamic_cast<jive::Object*>(element["style"].getDynamicObject());
        }

        [[nodiscard]] jive::Object* findRule(const juce::String& sheet, const juce::String& selector) const
        {
            auto* styleSheet = findStyleSheet(sheet);

            if (styleSheet == nullptr || selector.isEmpty())
                return styleSheet;

            return dynamic_cast<jive::Object*>(styleSheet->getProperty(selector).getDynamicObject());
        }

        [[nodiscard]] juce::var getRuleProperty(const Control& control) const
        {
            if (const auto* rule = findRule(control.sheet, control.rule))
                return rule->getProperty(control.property);

            return {};
        }

        void setRuleProperty(const Control& control, const juce::var& value)
        {
            if (auto* rule = findRule(control.sheet, control.rule))
                rule->setProperty(control.property, value);
        }

        [[nodiscard]] bool isToggled(const juce::String& buttonID) const
        {
            return jive::findElementWithID(window->state, buttonID)["toggled"];
        }

        const jive::SourceFile mainView{ "public/views/Main.xml" };
        const std::array<std::pair<juce::String, juce::String>, 5> pages{ {
            { "properties-tab", "public/views/Properties.xml" },
            { "selectors-tab", "public/views/Selectors.xml" },
            { "specificity-tab", "public/views/Specificity.xml" },
            { "states-tab", "public/views/States.xml" },
            { "tokens-tab", "public/views/Tokens.xml" },
        } };
        const std::array<Control, 9> controls{ {
            { "background-combo-box", {}, "#card", "background" },
            { "border-combo-box", {}, "#card", "border" },
            { "border-width-slider", {}, "#card", "border-width", "{}px" },
            { "border-radius-slider", {}, "#card", "border-radius", "{}px" },
            { "shadow-combo-box", {}, "#card", "shadow" },
            { "foreground-combo-box", {}, "#card", "foreground" },
            { "fill-combo-box", {}, "#card", "fill" },
            { "accent-combo-box", "tokens", {}, "$accent" },
            { "radius-slider", "tokens", {}, "$radius" },
        } };

        std::unique_ptr<jive::Property<jive::Object::ReferenceCountedPointer>> windowStyleSheet;
        std::unique_ptr<jive::Property<jive::Object::ReferenceCountedPointer>> pageStyleSheet;
        std::unique_ptr<jive::Property<int>> theme;
        std::vector<std::unique_ptr<jive::Property<bool>>> tabs;
        std::vector<std::unique_ptr<jive::Property<bool>>> toggles;
        std::vector<std::unique_ptr<jive::Property<int>>> values;
        juce::StringArray launchedComboBoxes;
        Inspector inspector;
    };
} // namespace jive_examples::style_sheets
