#pragma once

#include "SegmentedSlider.h"
#include "XYPadItem.h"

#include <jive_layouts/jive_layouts.h>

#include <shared/Fonts.h>
#include <shared/Inspector.h>
#include <shared/Theme.h>

namespace jive_examples::widgets
{
    // Lives as long as the processor. The host owns each editor it creates,
    // so the presenter only keeps a weak reference to the current one.
    class Presenter : private juce::ComponentListener
    {
    public:
        Presenter(juce::AudioProcessor& audioProcessor,
                  juce::AudioProcessorValueTreeState& processorParameters,
                  juce::UndoManager& processorUndoManager)
            : processor{ audioProcessor }
            , parameters{ processorParameters }
            , undoManager{ processorUndoManager }
        {
            interpreter.getComponentFactory().set("XYPad", [] {
                return std::make_shared<XYPad>();
            });
            interpreter.addDecorator<XYPadItem>("XYPad");
            jive::ComponentTypePredicates::addPredicate([](const juce::Component& component, const juce::String& typeName) {
                return typeName == "XYPad" && dynamic_cast<const XYPad*>(&component) != nullptr;
            });

            interpreter.onExternalSourceLoaded = [this](const jive::SourceFile& file) {
                if (editor == nullptr)
                    return;

                if (file == mainView)
                    bindEditor();

                bindPage();
                inspector.inspect(*editor->getComponent());
            };
        }

        [[nodiscard]] juce::AudioProcessorEditor* createEditor()
        {
            auto item = interpreter.interpret(mainView, &processor);
            auto* pluginEditor = dynamic_cast<juce::AudioProcessorEditor*>(item.get());

            if (pluginEditor == nullptr)
                return new juce::GenericAudioProcessorEditor{ processor }; // NOLINT

            if (auto* lookAndFeel = dynamic_cast<jive::LookAndFeel*>(&pluginEditor->getLookAndFeel()))
                lookAndFeel->addPainter("Slider.segmented", paintSegmentedSlider);

            editor = item.release();
            pluginEditor->addComponentListener(this);
            bindEditor();
            bindPage();
            inspector.inspect(*editor->getComponent());

            return pluginEditor;
        }

    private:
        // The host deletes the editor, so everything bound to it has to let go
        // of it first. Tearing it down turns the first tab back on, which would
        // otherwise load a page into the half-deleted editor and rebind it.
        void componentBeingDeleted(juce::Component&) final
        {
            inspector.close();
            theme = nullptr;
            tabs.clear();
            toggles.clear();
            values.clear();
            events.clear();
            editor = nullptr;
        }

        void bindEditor()
        {
            tabs.clear();

            for (const auto& [tab, view] : pages)
            {
                onToggled(tabs, tab, [this, source = view](bool toggled) {
                    if (toggled)
                        setProperty("page", jive::ids::source(), source);
                });
            }

            theme = bindThemeComboBox(editor->state);
        }

        void bindPage()
        {
            toggles.clear();
            values.clear();
            events.clear();
            clicks = 0;

            bindGallery();
            bindCustom();
        }

        void bindGallery()
        {
            onClick("count-button", [this] {
                ++clicks;
                setProperty("count-label", jive::ids::text(), describeClicks());
            });

            const auto switchLight = [this](bool on) {
                setProperty("light-button", jive::ids::toggled(), on);
                setProperty("light-checkbox", jive::ids::toggled(), on);
                light(jive::findElementWithID(editor->state, lightID), on);
            };
            onToggled(toggles, "light-button", switchLight);
            onToggled(toggles, "light-checkbox", switchLight);

            for (std::size_t i = 0; i < alignments.size(); ++i)
            {
                onToggled(toggles, alignments[i].first, [this, index = static_cast<int>(i)](bool toggled) {
                    if (toggled)
                        align(index);
                });
            }
            onValueChange("alignment-combo-box", jive::ids::selected(), [this](int index) {
                if (index >= 0)
                    align(index);
            });

            for (const auto* control : levelControls)
            {
                onValueChange(control, jive::ids::value(), [this](int level) {
                    setLevel(level);
                });
            }

            onValueChange("rating-spinner", jive::ids::value(), [this](int rating) {
                const auto stars = jive::findElementWithID(editor->state, ratingID);

                for (auto i = 0; i < stars.getNumChildren(); ++i)
                    light(stars.getChild(i), i < rating);
            });
        }

        void align(int index)
        {
            const auto& [buttonID, alignment] = alignments[static_cast<std::size_t>(index)];

            setProperty(buttonID, jive::ids::toggled(), true);
            setProperty("alignment-combo-box", jive::ids::selected(), index);
            setProperty("alignment", jive::ids::alignItems(), alignment);
        }

        void setLevel(int level)
        {
            for (const auto* control : levelControls)
                setProperty(control, jive::ids::value(), level);

            setProperty("level-bar", jive::ids::value(), level / 100.0);
            setProperty("level-label", jive::ids::text(), juce::String{ level } + "%");
        }

        static void light(juce::ValueTree icon, bool on)
        {
            icon.setProperty(jive::ids::classes(), on ? "icon on" : "icon", nullptr);
        }

        [[nodiscard]] juce::String describeClicks() const
        {
            return "Clicked " + juce::String{ clicks } + (clicks == 1 ? " time" : " times");
        }

        void bindCustom()
        {
            attachParameters(*editor);
            onChange("pad", [this] {
                const auto pad = jive::findElementWithID(editor->state, padID);
                const auto format = [&pad](juce::Identifier axis) {
                    return juce::String{ static_cast<double>(pad[axis]), 2 };
                };

                setProperty("pad-label",
                            jive::ids::text(),
                            "x " + format(xValueID) + ", y " + format(yValueID));
            });
        }

        // Every element with a parameter attribute is attached to that
        // parameter, including Text elements, which show its value.
        void attachParameters(jive::GuiItem& item)
        {
            if (const auto paramID = item.state[parameterID].toString();
                paramID.isNotEmpty())
            {
                item.attachToParameter(parameters.getParameter(paramID),
                                       &undoManager);
            }

            for (auto* child : item.getChildren())
                attachParameters(*child);
        }

        void onClick(const juce::String& buttonID, std::function<void()> callback)
        {
            onEvent(buttonID, jive::ids::onClick(), std::move(callback));
        }

        void onChange(const juce::String& controlID, std::function<void()> callback)
        {
            onEvent(controlID, jive::ids::onChange(), std::move(callback));
        }

        void onEvent(const juce::String& elementID, juce::Identifier event, std::function<void()> callback)
        {
            const auto element = jive::findElementWithID(editor->state, elementID);

            if (element.isValid())
                events.push_back(std::make_unique<jive::Event>(element, event, std::move(callback)));
        }

        void onToggled(std::vector<std::unique_ptr<jive::Property<bool>>>& bindings,
                       const juce::String& buttonID,
                       const std::function<void(bool)>& callback)
        {
            const auto button = jive::findElementWithID(editor->state, buttonID);

            if (!button.isValid())
                return;

            auto& toggled = *bindings.emplace_back(std::make_unique<jive::Property<bool>>(button, jive::ids::toggled()));
            toggled.onValueChange = [&toggled, callback] {
                callback(toggled.get());
            };

            callback(toggled.get());
        }

        void onValueChange(const juce::String& controlID,
                           juce::Identifier property,
                           const std::function<void(int)>& callback)
        {
            const auto control = jive::findElementWithID(editor->state, controlID);

            if (!control.isValid())
                return;

            auto& value = *values.emplace_back(std::make_unique<jive::Property<int>>(control, property));
            value.onValueChange = [&value, callback] {
                callback(value.get());
            };

            callback(value.get());
        }

        void setProperty(const juce::String& id, juce::Identifier property, const juce::var& value)
        {
            jive::findElementWithID(editor->state, id).setProperty(property, value, nullptr);
        }

        const juce::Identifier lightID{ "light" };
        const juce::Identifier ratingID{ "rating" };
        const juce::Identifier padID{ "pad" };
        const juce::Identifier padLabelID{ "pad-label" };
        const juce::Identifier xValueID{ "x-value" };
        const juce::Identifier yValueID{ "y-value" };
        const juce::Identifier parameterID{ "parameter" };

        // Declared before anything that gets interpreted.
        const std::array<juce::Typeface::Ptr, 3> fonts = registerFonts();

        const jive::SourceFile mainView{ "public/views/Main.xml" };
        const std::array<std::pair<juce::String, juce::String>, 2> pages{
            {
                { "gallery-tab", "public/views/Gallery.xml" },
                { "custom-tab", "public/views/Custom.xml" },
            },
        };
        const std::array<std::pair<juce::String, juce::String>, 3> alignments{
            {
                { "left-button", "flex-start" },
                { "centre-button", "center" },
                { "right-button", "flex-end" },
            },
        };
        const std::array<const char*, 3> levelControls{
            "level-slider",
            "level-fader",
            "level-knob",
        };

        juce::AudioProcessor& processor;
        juce::AudioProcessorValueTreeState& parameters;
        juce::UndoManager& undoManager;

        jive::Interpreter interpreter;
        juce::WeakReference<jive::GuiItem> editor;
        std::unique_ptr<jive::Property<int>> theme;
        std::vector<std::unique_ptr<jive::Property<bool>>> tabs;
        std::vector<std::unique_ptr<jive::Property<bool>>> toggles;
        std::vector<std::unique_ptr<jive::Property<int>>> values;
        std::vector<std::unique_ptr<jive::Event>> events;
        int clicks = 0;
        Inspector inspector;
    };
} // namespace jive_examples::widgets
