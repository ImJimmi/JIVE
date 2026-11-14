#pragma once

#include "components/EnvelopeView.h"
#include "components/Keyboard.h"
#include "components/Meter.h"
#include "graphics/ComboBoxPainter.h"
#include "graphics/KnobPainter.h"

#include <jive_layouts/jive_layouts.h>

#include <shared/Fonts.h>
#include <shared/Inspector.h>

namespace boabass
{
    class Presenter
        : private juce::ComponentListener
        , private juce::Timer
    {
    public:
        Presenter(juce::AudioProcessor& audioProcessor,
                  juce::AudioProcessorValueTreeState& processorParameters,
                  juce::MidiKeyboardState& keyboardState,
                  const std::atomic<float>& outputLevel)
            : processor{ audioProcessor }
            , parameters{ processorParameters }
            , level{ outputLevel }
        {
            interpreter.getComponentFactory().set("Keyboard", [&keyboardState] {
                return std::make_shared<Keyboard>(keyboardState);
            });
            interpreter.getComponentFactory().set("Envelope", [] {
                return std::make_shared<EnvelopeView>();
            });
            interpreter.addDecorator<EnvelopeViewItem>("Envelope");
            interpreter.getComponentFactory().set("Meter", [] {
                return std::make_shared<Meter>();
            });
            interpreter.addDecorator<MeterItem>("Meter");

            interpreter.autoAttachParameters(parameters);
            interpreter.onLoad = [this](jive::GuiItem&) {
                if (view == nullptr)
                    return;

                bind();
                inspector.inspect(*view->getComponent());
            };
        }

        ~Presenter() override
        {
            stopTimer();
        }

        [[nodiscard]] juce::AudioProcessorEditor* createEditor()
        {
            auto item = interpreter.interpret(editorView, &processor);

            if (dynamic_cast<juce::AudioProcessorEditor*>(item.get()) == nullptr)
                return new juce::GenericAudioProcessorEditor{ processor };

            return dynamic_cast<juce::AudioProcessorEditor*>(show(item.release()));
        }

        [[nodiscard]] std::unique_ptr<jive::GuiItem> createWindow()
        {
            auto item = interpreter.interpret(windowView);
            show(item.get());
            return item;
        }

    private:
        juce::Component* show(jive::GuiItem* item)
        {
            auto* component = dynamic_cast<juce::Component*>(item);

            if (auto* lookAndFeel = dynamic_cast<jive::LookAndFeel*>(&component->getLookAndFeel()))
            {
                lookAndFeel->addPainter("Knob", paintKnob);
                lookAndFeel->addPainter("ComboBox", paintComboBox);
            }

            view = item;
            component->addComponentListener(this);
            bind();
            inspector.inspect(*view->getComponent());
            startTimerHz(30);

            return component;
        }

        void componentBeingDeleted(juce::Component&) final
        {
            stopTimer();
            inspector.close();
            unbind();
            view = nullptr;
        }

        void unbind()
        {
            attachments.clear();
            events.clear();
            preset = nullptr;
            language = nullptr;
            meter = {};
        }

        void bind()
        {
            unbind();

            bindChoices();
            bindSwitches();
            bindEnvelopes();
            bindPresets();
            bindLanguage();

            meter = jive::findElementWithID(view->state, "meter");
        }

        void bindChoices()
        {
            for (auto group : jive::findElementsWithClass(view->state, "choices"))
            {
                auto* parameter = parameters.getParameter(group["parameter"].toString());

                if (parameter == nullptr)
                    continue;

                // Toggling a button also clicks it, which mustn't be taken
                // for the user picking that choice.
                auto& attachment = attach(*parameter, [this, group](float index) mutable {
                    const juce::ScopedValueSetter<bool> showing{ showingChoice, true };

                    for (auto i = 0; i < group.getNumChildren(); i++)
                        group.getChild(i).setProperty("toggled", i == juce::roundToInt(index), nullptr);
                });

                for (auto i = 0; i < group.getNumChildren(); i++)
                {
                    events.push_back(std::make_unique<jive::Event>(group.getChild(i), "on-click", [this, &attachment, i] {
                        if (!showingChoice)
                            attachment.setValueAsCompleteGesture(static_cast<float>(i));
                    }));
                }
            }
        }

        void bindSwitches()
        {
            const auto sections = jive::findAll(view->state, [](const juce::ValueTree& tree) {
                return tree.hasProperty("switched-by");
            });

            for (auto section : sections)
            {
                if (auto* parameter = parameters.getParameter(section["switched-by"].toString()))
                {
                    attach(*parameter, [section](float on) mutable {
                        auto classes = juce::StringArray::fromTokens(section["class"].toString(), false);
                        classes.removeString("off");

                        if (on < 0.5f)
                            classes.add("off");

                        section.setProperty("class", classes.joinIntoString(" "), nullptr);
                    });
                }
            }
        }

        void bindEnvelopes()
        {
            const auto envelopes = jive::findAll(view->state, [](const juce::ValueTree& tree) {
                return tree.hasType("Envelope");
            });

            for (auto envelope : envelopes)
            {
                const auto prefix = envelope["prefix"].toString();

                for (const auto* stage : { "attack", "decay", "sustain", "release" })
                {
                    if (auto* parameter = parameters.getParameter(prefix + "-" + stage))
                    {
                        attach(*parameter, [envelope, stage](float value) mutable {
                            envelope.setProperty(stage, value, nullptr);
                        });
                    }
                }
            }
        }

        void bindPresets()
        {
            auto comboBox = jive::findElementWithID(view->state, "preset");

            if (!comboBox.isValid())
                return;

            comboBox.removeAllChildren(nullptr);

            for (auto i = 0; i < processor.getNumPrograms(); i++)
                comboBox.appendChild(juce::ValueTree{ "Option", { { "text", processor.getProgramName(i) } } }, nullptr);

            comboBox.setProperty("selected", processor.getCurrentProgram(), nullptr);

            preset = std::make_unique<jive::Property<int>>(comboBox, "selected");
            preset->onValueChange = [this] {
                if (preset->get() >= 0 && preset->get() != processor.getCurrentProgram())
                    processor.setCurrentProgram(preset->get());
            };

            const auto step = [this, comboBox](int direction) {
                return [this, comboBox, direction]() mutable {
                    const auto numPresets = processor.getNumPrograms();
                    comboBox.setProperty("selected", (processor.getCurrentProgram() + direction + numPresets) % numPresets, nullptr);
                };
            };
            onClick("previous-preset", step(-1));
            onClick("next-preset", step(1));
        }

        void bindLanguage()
        {
            auto comboBox = jive::findElementWithID(view->state, "language");

            if (!comboBox.isValid())
                return;

            language = std::make_unique<jive::Property<int>>(comboBox, "selected");
            language->onValueChange = [this, comboBox] {
                const auto option = comboBox.getChild(language->get());

                if (option.isValid())
                    view->state.setProperty(jive::ids::lang(), option[jive::ids::lang()], nullptr);
            };
            language->onValueChange();
        }

        void onClick(const juce::String& buttonID, std::function<void()> callback)
        {
            const auto button = jive::findElementWithID(view->state, buttonID);

            if (button.isValid())
                events.push_back(std::make_unique<jive::Event>(button, "on-click", std::move(callback)));
        }

        juce::ParameterAttachment& attach(juce::RangedAudioParameter& parameter, std::function<void(float)> callback)
        {
            auto& attachment = *attachments.emplace_back(std::make_unique<juce::ParameterAttachment>(parameter,
                                                                                                     std::move(callback),
                                                                                                     parameters.undoManager));
            attachment.sendInitialUpdate();
            return attachment;
        }

        void timerCallback() final
        {
            showLevel();
            followProgramChangesFromHost();
        }

        void followProgramChangesFromHost()
        {
            if (preset != nullptr && preset->get() != processor.getCurrentProgram())
                *preset = processor.getCurrentProgram();
        }

        void showLevel()
        {
            if (!meter.isValid())
                return;

            const auto decibels = juce::Decibels::gainToDecibels(level.load(std::memory_order_relaxed), -60.0f);
            const auto target = juce::jmap(decibels, -60.0f, 0.0f, 0.0f, 1.0f);
            static constexpr auto meterFallPerFrame = 0.03f;
            displayedLevel = juce::jmax(target, displayedLevel - meterFallPerFrame);
            meter.setProperty("value", displayedLevel, nullptr);
        }

        // Declared before anything that gets interpreted.
        const std::array<juce::Typeface::Ptr, 6> fonts = jive_examples::registerFonts();

        const jive::SourceFile editorView{ "public/views/Editor.xml" };
        const jive::SourceFile windowView{ "public/views/Window.xml" };

        juce::AudioProcessor& processor;
        juce::AudioProcessorValueTreeState& parameters;
        const std::atomic<float>& level;

        jive::Interpreter interpreter;
        juce::WeakReference<jive::GuiItem> view;
        std::vector<std::unique_ptr<juce::ParameterAttachment>> attachments;
        std::vector<std::unique_ptr<jive::Event>> events;
        std::unique_ptr<jive::Property<int>> preset;
        std::unique_ptr<jive::Property<int>> language;
        juce::ValueTree meter;
        float displayedLevel = 0.0f;
        bool showingChoice = false;
        jive_examples::Inspector inspector;
    };
} // namespace boabass
