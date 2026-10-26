#pragma once

#include <jive_layouts/jive_layouts.h>

#include <shared/Inspector.h>
#include <shared/Presenter.h>
#include <shared/Theme.h>

namespace jive_examples::layouts
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
        void bindWindow()
        {
            windowState = window->state;
            theme = bindThemeComboBox(window->state);

            tabs.clear();

            for (const auto& [tab, view] : pages)
            {
                onToggledOn(tabs, tab, [this, source = view] {
                    setProperty("page", "template", source);
                });
            }
        }

        void bindPage()
        {
            toggles.clear();
            values.clear();

            bindFlexControls();
            bindGridControls();
            bindAbsoluteControls();
        }

        void bindFlexControls()
        {
            bindRadioButtons({
                                 { "row-button", "row" },
                                 { "column-button", "column" },
                                 { "row-reverse-button", "row-reverse" },
                                 { "column-reverse-button", "column-reverse" },
                             },
                             "flex-direction-label",
                             "flex-preview",
                             "flex-direction");
            bindComboBox("justify-content-combo-box", "flex-preview", "justify-content");
            bindComboBox("align-items-combo-box", "flex-preview", "align-items");
            bindComboBox("flex-wrap-combo-box", "flex-preview", "flex-wrap");
            bindComboBox("align-content-combo-box", "flex-preview", "align-content");
            bindSlider("flex-gap-slider", "flex-gap-label", "flex-preview", "gap", "{}px");
            bindSlider("flex-width-slider", "flex-width-label", "flex-preview", "width", "{}%");

            const auto flexItem = selectedItem("flex-item-combo-box", "flex-preview");
            bindSlider("flex-grow-slider", "flex-grow-label", flexItem, "flex-grow");
            bindSlider("flex-shrink-slider", "flex-shrink-label", flexItem, "flex-shrink");
            bindSlider("flex-basis-slider", "flex-basis-label", flexItem, "flex-basis", "{}px");
            bindSlider("order-slider", "order-label", flexItem, "order");
            bindComboBox("align-self-combo-box", flexItem, "align-self");
            onValueChange(values, "flex-item-combo-box", "selected", [this, flexItem](int) {
                showFlexItem(flexItem());
            });
        }

        void showFlexItem(const juce::ValueTree& item)
        {
            setProperty("flex-grow-slider", "value", item.getProperty("flex-grow", 0));
            setProperty("flex-shrink-slider", "value", item.getProperty("flex-shrink", 1));
            setProperty("flex-basis-slider", "value", item.getProperty("flex-basis").toString().getIntValue());
            setProperty("order-slider", "value", item.getProperty("order", 0));
            setProperty("align-self-combo-box", "selected", indexOfOption("align-self-combo-box", item.getProperty("align-self", "auto")));
        }

        void bindGridControls()
        {
            bindComboBox("columns-combo-box", "grid-preview", "grid-template-columns");
            bindComboBox("rows-combo-box", "grid-preview", "grid-template-rows");
            bindSlider("grid-gap-slider", "grid-gap-label", "grid-preview", "gap", "{}px");
            bindComboBox("justify-items-combo-box", "grid-preview", "justify-items");
            bindComboBox("grid-align-items-combo-box", "grid-preview", "align-items");
            bindComboBox("grid-justify-content-combo-box", "grid-preview", "justify-content");
            bindComboBox("grid-align-content-combo-box", "grid-preview", "align-content");
            bindComboBox("auto-flow-combo-box", "grid-preview", "grid-auto-flow");

            const auto gridItem = selectedItem("grid-item-combo-box", "grid-preview");
            bindSlider("column-span-slider", "column-span-label", gridItem, "grid-column", "auto / span {}");
            bindSlider("row-span-slider", "row-span-label", gridItem, "grid-row", "auto / span {}");
            onValueChange(values, "grid-item-combo-box", "selected", [this, gridItem](int) {
                showGridItem(gridItem());
            });
        }

        void showGridItem(const juce::ValueTree& item)
        {
            setProperty("column-span-slider", "value", spanOf(item["grid-column"]));
            setProperty("row-span-slider", "value", spanOf(item["grid-row"]));
        }

        [[nodiscard]] static int spanOf(const juce::var& placement)
        {
            return juce::jmax(1, placement.toString().fromLastOccurrenceOf("span", false, false).getIntValue());
        }

        struct Length
        {
            juce::Identifier property;
            bool horizontal;

            [[nodiscard]] juce::String id(const juce::String& suffix) const
            {
                return property.toString() + suffix;
            }
        };

        void bindAbsoluteControls()
        {
            const auto absoluteItem = selectedItem("absolute-item-combo-box", "absolute-preview");

            // Shown before anything else is bound so the controls' initial
            // values don't overwrite the box's own.
            onValueChange(values, "absolute-item-combo-box", "selected", [this, absoluteItem](int) {
                showAbsoluteItem(absoluteItem());
            });
            bindComboBox("anchor-combo-box", absoluteItem, "anchor");

            for (const auto& length : lengths)
            {
                onValueChange(values, length.id("-slider"), "value", [this, length, absoluteItem](int value) {
                    auto item = absoluteItem();
                    item.setProperty(length.property, withUnit(value, isPercentage(item[length.property])), nullptr);
                    setProperty(length.id("-label"), "text", item[length.property]);
                });
                onToggledOn(toggles, length.id("-px-button"), [this, length, absoluteItem] {
                    convertUnit(length, absoluteItem(), false);
                });
                onToggledOn(toggles, length.id("-percent-button"), [this, length, absoluteItem] {
                    convertUnit(length, absoluteItem(), true);
                });
            }
        }

        void showAbsoluteItem(const juce::ValueTree& item)
        {
            setProperty("anchor-combo-box", "selected", indexOfOption("anchor-combo-box", item.getProperty("anchor", "top-left")));

            for (const auto& length : lengths)
            {
                const auto percentage = isPercentage(item[length.property]);
                setProperty(length.id(percentage ? "-percent-button" : "-px-button"), "toggled", true);
                showLength(length, item);
            }
        }

        void showLength(const Length& length, const juce::ValueTree& item)
        {
            const auto value = item[length.property];
            setProperty(length.id("-slider"), "max", isPercentage(value) ? 100 : 500);
            setProperty(length.id("-slider"), "value", juce::roundToInt(value.toString().getFloatValue()));
            setProperty(length.id("-label"), "text", value);
        }

        // Converts to the other unit using the preview's current size, so the
        // box stays where it is until the preview is resized.
        void convertUnit(const Length& length, juce::ValueTree item, bool toPercentage)
        {
            const auto& property = length.property;

            if (isPercentage(item[property]) != toPercentage)
            {
                const auto bounds = jive::boxModel(*jive::findItemWithID(*window, "absolute-preview")).getContentBounds();
                const auto parentSize = length.horizontal ? bounds.getWidth() : bounds.getHeight();
                const auto value = item[property].toString().getFloatValue();
                const auto converted = toPercentage ? value / parentSize * 100.0f : value * parentSize / 100.0f;
                item.setProperty(property, withUnit(juce::roundToInt(converted), toPercentage), nullptr);
            }

            showLength(length, item);
        }

        [[nodiscard]] static bool isPercentage(const juce::var& length)
        {
            return length.toString().trim().endsWithChar('%');
        }

        [[nodiscard]] static juce::String withUnit(int value, bool percentage)
        {
            return juce::String{ value } + (percentage ? "%" : "px");
        }

        void bindRadioButtons(const std::vector<std::pair<juce::String, juce::String>>& buttons,
                              const juce::String& labelID,
                              const juce::String& targetID,
                              const juce::Identifier& property)
        {
            for (const auto& [button, value] : buttons)
            {
                onToggledOn(toggles, button, [this, labelID, targetID, property, text = value] {
                    setProperty(targetID, property, text);
                    setProperty(labelID, "text", text);
                });
            }
        }

        void bindComboBox(const juce::String& comboBoxID,
                          const juce::String& targetID,
                          const juce::Identifier& property)
        {
            bindComboBox(comboBoxID, elementWithID(targetID), property);
        }

        void bindComboBox(const juce::String& comboBoxID,
                          std::function<juce::ValueTree()> target,
                          const juce::Identifier& property)
        {
            onValueChange(values, comboBoxID, "selected", [this, comboBoxID, target, property](int selected) {
                const auto option = jive::findElementWithID(window->state, comboBoxID).getChild(selected);
                target().setProperty(property, option["text"], nullptr);
            });
        }

        void bindSlider(const juce::String& sliderID,
                        const juce::String& labelID,
                        const juce::String& targetID,
                        const juce::Identifier& property,
                        const juce::String& format = "{}")
        {
            bindSlider(sliderID, labelID, elementWithID(targetID), property, format);
        }

        void bindSlider(const juce::String& sliderID,
                        const juce::String& labelID,
                        std::function<juce::ValueTree()> target,
                        const juce::Identifier& property,
                        const juce::String& format = "{}")
        {
            onValueChange(values, sliderID, "value", [this, labelID, target, property, format](int value) {
                const auto text = format.replace("{}", juce::String{ value });
                target().setProperty(property, text, nullptr);
                setProperty(labelID, "text", text);
            });
        }

        void onToggledOn(std::vector<std::unique_ptr<jive::Property<bool>>>& bindings,
                         const juce::String& buttonID,
                         std::function<void()> callback)
        {
            onToggled(bindings, buttonID, [callback](bool toggled) {
                if (toggled)
                    callback();
            });
        }

        [[nodiscard]] std::function<juce::ValueTree()> elementWithID(const juce::String& id) const
        {
            return [this, id] {
                return jive::findElementWithID(window->state, id);
            };
        }

        [[nodiscard]] std::function<juce::ValueTree()> selectedItem(const juce::String& comboBoxID,
                                                                    const juce::String& previewID) const
        {
            return [this, comboBoxID, previewID] {
                const auto selected = jive::findElementWithID(window->state, comboBoxID)["selected"];
                return jive::findElementWithID(window->state, previewID).getChild(static_cast<int>(selected));
            };
        }

        const jive::SourceFile mainView{ "public/views/Main.xml" };
        const std::array<std::pair<juce::String, juce::String>, 3> pages{ {
            { "flex-tab", "public/views/Flex.xml" },
            { "grid-tab", "public/views/Grid.xml" },
            { "absolute-tab", "public/views/Absolute.xml" },
        } };
        const std::array<Length, 4> lengths{ {
            { "x", true },
            { "y", false },
            { "width", true },
            { "height", false },
        } };

        // Switching pages leaves the window's tree in place, along with the
        // tab binding that switched them. Reloading Main.xml replaces it.
        juce::ValueTree windowState;
        std::unique_ptr<jive::Property<int>> theme;
        std::vector<std::unique_ptr<jive::Property<bool>>> tabs;
        std::vector<std::unique_ptr<jive::Property<bool>>> toggles;
        std::vector<std::unique_ptr<jive::Property<int>>> values;
        Inspector inspector;
    };
} // namespace jive_examples::layouts
