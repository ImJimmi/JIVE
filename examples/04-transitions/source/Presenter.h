#pragma once

#include <jive_layouts/jive_layouts.h>

#include <shared/Inspector.h>
#include <shared/Presenter.h>
#include <shared/Theme.h>

namespace jive_examples::transitions
{
    class Presenter : public BasePresenter
    {
    public:
        Presenter()
        {
            window = interpreter.interpret(mainView);
            interpreter.onLoad = [this](jive::GuiItem& root) {
                if (root.state != windowState)
                    bindWindow();

                bindPage();
                inspector.inspect(*window->getComponent());
            };
            interpreter.onLoad(*window);
        }

    private:
        // An attribute that Animate switches between two values on the element
        // with the given ID. The element's own transition does the animating.
        struct Animation
        {
            juce::Identifier property;
            juce::String targetID;
            juce::var from;
            juce::var to;
        };

        // The numbers a style property's start and end sliders choose between,
        // and how each one is written in the style sheet.
        struct StyleRange
        {
            juce::String property;
            int min;
            int max;
            juce::String format;
            int start;
            int end;

            [[nodiscard]] juce::String toValue(int number) const
            {
                return format.replace("{}", juce::String{ number });
            }

            [[nodiscard]] std::optional<int> fromValue(const juce::var& value) const
            {
                const auto prefix = format.upToFirstOccurrenceOf("{}", false, false);

                if (value.isVoid() || !value.toString().startsWith(prefix))
                    return std::nullopt;

                return value.toString().substring(prefix.length()).getIntValue();
            }
        };

        void bindWindow()
        {
            windowState = window->state;
            theme = bindThemeComboBox(window->state);

            tabs.clear();

            for (const auto& [tab, view] : pages)
            {
                onToggled(tabs, tab, [this, source = view](bool toggled) {
                    if (toggled)
                        setProperty("page", "source", source);
                });
            }
        }

        void bindPage()
        {
            toggles.clear();
            values.clear();
            cardStyleSheet = nullptr;

            bindTiming();
            bindLayout();
            bindStyles();
        }

        // Each box uses the transition written next to it, with the duration
        // swapped for the slider's.
        void bindTiming()
        {
            onValueChange(values, "duration-slider", "value", [this](int duration) {
                const auto lanes = jive::findElementsWithClass(window->state, "lane");
                const auto runners = jive::findElementsWithClass(window->state, "runner");

                for (std::size_t i = 0; i < std::min(lanes.size(), runners.size()); i++)
                {
                    auto parts = juce::StringArray::fromTokens(lanes[i]["text"].toString(), " ", "");

                    if (parts.size() < 2)
                        continue;

                    parts.set(1, juce::String{ duration } + "ms");
                    juce::ValueTree{ lanes[i] }.setProperty("text", parts.joinIntoString(" "), nullptr);
                    juce::ValueTree{ runners[i] }.setProperty("transition", parts.joinIntoString(" "), nullptr);
                }

                setProperty("duration-label", "text", juce::String{ duration } + "ms");
            });
            onToggled(toggles, "move-button", [this](bool toggled) {
                for (auto runner : jive::findElementsWithClass(window->state, "runner"))
                    runner.setProperty("x", toggled ? "95%" : "5%", nullptr);
            });
        }

        void bindLayout()
        {
            onValueChange(values, "property-combo-box", "selected", [this](int) {
                setProperty("animate-button", "toggled", false);

                for (const auto& animation : animations)
                    setProperty(animation.targetID, animation.property, animation.from);

                showAnimation();
            });
            onToggled(toggles, "animate-button", [this](bool toggled) {
                if (const auto* animation = findSelectedAnimation())
                    setProperty(animation->targetID, animation->property, toggled ? animation->to : animation->from);

                showAnimation();
            });
        }

        void showAnimation()
        {
            if (const auto* animation = findSelectedAnimation())
            {
                const auto value = jive::findElementWithID(window->state, animation->targetID)[animation->property];
                setProperty("value-label", "text", value);
            }
        }

        [[nodiscard]] const Animation* findSelectedAnimation() const
        {
            const auto comboBox = jive::findElementWithID(window->state, "property-combo-box");
            const auto option = comboBox.getChild(comboBox["selected"])["text"].toString();

            for (const auto& animation : animations)
            {
                if (animation.property.toString() == option)
                    return &animation;
            }

            return nullptr;
        }

        // Saving Card.json swaps in a newly parsed style sheet, which the
        // controls then show.
        void bindStyles()
        {
            const auto preview = jive::findElementWithID(window->state, "card-preview");

            if (!preview.isValid())
                return;

            cardStyleSheet = std::make_unique<jive::Property<jive::Object::ReferenceCountedPointer>>(preview, "style");
            cardStyleSheet->onValueChange = [this] {
                showStyles();
            };

            onValueChange(values, "style-property-combo-box", "selected", [this](int selected) {
                const auto option = jive::findElementWithID(window->state, "style-property-combo-box").getChild(selected);
                transitionStyle(option["text"].toString());
            });
            onValueChange(values, "start-slider", "value", [this](int value) {
                setStyle(findRule(findCardStyleSheet(), "#card"), value);
            });
            onValueChange(values, "end-slider", "value", [this](int value) {
                setStyle(findRule(findRule(findCardStyleSheet(), "#card"), ":hover"), value);
            });

            showStyles();
        }

        void showStyles()
        {
            auto* card = findRule(findCardStyleSheet(), "#card");
            const auto* range = findTransitionedStyle();

            if (card == nullptr || range == nullptr)
                return;

            setProperty("style-property-combo-box", "selected", indexOfOption("style-property-combo-box", range->property));
            showStyle("start", *range, card);
            showStyle("end", *range, findRule(card, ":hover"));
            setProperty("card-sheet", "text", juce::JSON::toString(juce::var{ findCardStyleSheet() }).replace("\r\n", "\n"));
        }

        void showStyle(const juce::String& control, const StyleRange& range, const jive::Object* rule)
        {
            const auto value = rule != nullptr ? rule->getProperty(range.property) : juce::var{};

            setProperty(control + "-slider", "min", range.min);
            setProperty(control + "-slider", "max", range.max);

            if (const auto number = range.fromValue(value))
                setProperty(control + "-slider", "value", *number);

            setProperty(control + "-label", "text", value);
        }

        // Moves the transition, and the :hover state, to another property. The
        // transition keeps its timing.
        void transitionStyle(const juce::String& property)
        {
            auto* styleSheet = findCardStyleSheet();
            auto* card = findRule(styleSheet, "#card");
            const auto* range = findStyleRange(property);

            if (card == nullptr || range == nullptr || range == findTransitionedStyle())
                return;

            const auto timing = styleSheet->getProperty("transition").toString().fromFirstOccurrenceOf(" ", false, false);

            styleSheet->setProperty("transition", range->property + " " + (timing.isEmpty() ? defaultTiming : timing));
            card->setProperty(range->property, range->toValue(range->start));
            card->setProperty(":hover", new jive::Object{ { range->property, range->toValue(range->end) } });
        }

        void setStyle(jive::Object* rule, int value)
        {
            if (const auto* range = findTransitionedStyle(); range != nullptr && rule != nullptr)
                rule->setProperty(range->property, range->toValue(value));
        }

        [[nodiscard]] const StyleRange* findTransitionedStyle() const
        {
            if (const auto* styleSheet = findCardStyleSheet())
                return findStyleRange(styleSheet->getProperty("transition").toString().upToFirstOccurrenceOf(" ", false, false));

            return nullptr;
        }

        [[nodiscard]] const StyleRange* findStyleRange(const juce::String& property) const
        {
            for (const auto& range : styleRanges)
            {
                if (range.property == property)
                    return &range;
            }

            return nullptr;
        }

        [[nodiscard]] jive::Object* findCardStyleSheet() const
        {
            const auto preview = jive::findElementWithID(window->state, "card-preview");
            return dynamic_cast<jive::Object*>(preview["style"].getDynamicObject());
        }

        [[nodiscard]] static jive::Object* findRule(jive::Object* parent, const juce::String& selector)
        {
            if (parent == nullptr)
                return nullptr;

            return dynamic_cast<jive::Object*>(parent->getProperty(selector).getDynamicObject());
        }

        const jive::SourceFile mainView{ "public/views/Main.xml" };
        const std::array<std::pair<juce::String, juce::String>, 3> pages{ {
            { "timing-tab", "public/views/Timing.xml" },
            { "layout-tab", "public/views/Layout.xml" },
            { "styles-tab", "public/views/Styles.xml" },
        } };
        const std::array<Animation, 8> animations{ {
            { "flex-grow", "flex-box", 1, 4 },
            { "flex-basis", "flex-box", 60, 240 },
            { "gap", "flex-sample", 10, 40 },
            { "padding", "flex-sample", 10, 25 },
            { "border-width", "flex-box", 0, 12 },
            { "grid-template-columns", "grid-sample", "1fr 1fr 1fr", "1fr 4fr 1fr" },
            { "width", "absolute-box", 60, 300 },
            { "height", "absolute-box", 40, 90 },
        } };
        const std::array<StyleRange, 8> styleRanges{ {
            { "background", 0, 360, "hsl({}, 70%, 60%)", 200, 280 },
            { "border", 0, 360, "hsl({}, 90%, 70%)", 50, 160 },
            { "border-width", 0, 12, "{}px", 2, 10 },
            { "border-radius", 0, 55, "{}px", 16, 55 },
            { "shadow", 0, 40, "0 0 {}px hotpink", 0, 30 },
            { "foreground", 0, 100, "hsl(0, 0%, {}%)", 10, 100 },
            { "fill", 0, 360, "hsl({}, 90%, 55%)", 220, 45 },
            { "opacity", 0, 100, "{}%", 100, 30 },
        } };
        const juce::String defaultTiming{ "600ms ease-in-out" };

        // Switching pages leaves the window's tree in place, along with the
        // tab binding that switched them. Reloading Main.xml replaces it.
        juce::ValueTree windowState;
        std::unique_ptr<jive::Property<int>> theme;
        std::unique_ptr<jive::Property<jive::Object::ReferenceCountedPointer>> cardStyleSheet;
        std::vector<std::unique_ptr<jive::Property<bool>>> tabs;
        std::vector<std::unique_ptr<jive::Property<bool>>> toggles;
        std::vector<std::unique_ptr<jive::Property<int>>> values;
        Inspector inspector;
    };
} // namespace jive_examples::transitions
