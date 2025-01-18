#include "View.h"

#include <jive_layouts/jive_layouts.h>
#include <juce_audio_processors/juce_audio_processors.h>

namespace jive_example
{
    class GainPlugin : public juce::AudioProcessor
    {
    public:
        const juce::String getName() const final
        {
            return "JIVE Gain Example";
        }

        void prepareToPlay(double, int) final
        {
        }

        void releaseResources() final
        {
        }

        void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) final
        {
            buffer.applyGain(juce::Decibels::decibelsToGain<float>(gain));
        }

        double getTailLengthSeconds() const final
        {
            return 0.0;
        }

        bool acceptsMidi() const final
        {
            return false;
        }

        bool producesMidi() const final
        {
            return false;
        }

        juce::AudioProcessorEditor* createEditor() final
        {
            auto view = getGainPluginView();

            if (auto editor = viewInterpreter.interpret(view, this))
            {
                if (dynamic_cast<juce::AudioProcessorEditor*>(editor.get()))
                {
                    viewInterpreter.listenTo(*editor);

                    jive::findItemWithID(*editor, "gain-knob")->attachToParameter(apvts.getParameter("gain"), &undoManager);
                    jive::findItemWithID(*editor, "gain-label")->attachToParameter(apvts.getParameter("gain"), &undoManager);

                    return dynamic_cast<juce::AudioProcessorEditor*>(editor.release());
                }
            }

            return new juce::GenericAudioProcessorEditor{ *this };
        }

        bool hasEditor() const final
        {
            return true;
        }

        int getNumPrograms() final
        {
            return 1;
        }

        int getCurrentProgram() final
        {
            return 0;
        }

        void setCurrentProgram(int) final
        {
        }

        const juce::String getProgramName(int) final
        {
            return "";
        }

        void changeProgramName(int, const juce::String&) final
        {
        }

        void getStateInformation(juce::MemoryBlock&) final
        {
        }

        void setStateInformation(const void*, int) final
        {
        }

    private:
        jive::Interpreter viewInterpreter;

        juce::UndoManager undoManager;
        juce::AudioProcessorValueTreeState apvts{
            *this,
            &undoManager,
            "APVTS",
            []() -> juce::AudioProcessorValueTreeState::ParameterLayout {
                return {
                    std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ "gain", 1 },
                                                                "Gain",
                                                                juce::NormalisableRange<float>{ -12.0f, 12.0f },
                                                                0.0f,
                                                                juce::AudioParameterFloatAttributes{}
                                                                    .withStringFromValueFunction([](float value, int) {
                                                                        const auto maxLength = value < 0 ? 5 : 4;
                                                                        return juce::String::toDecimalStringWithSignificantFigures(value, 3)
                                                                            .paddedRight('0', maxLength)
                                                                            .substring(0, maxLength);
                                                                    })
                                                                    .withValueFromStringFunction([](const juce::String& text) {
                                                                        return text.getFloatValue();
                                                                    })
                                                                    .withCategory(juce::AudioProcessorParameter::Category::outputGain)
                                                                    .withAutomatable(true)),
                };
            }(),
        };
        juce::AudioParameterFloat& gain{
            dynamic_cast<juce::AudioParameterFloat&>(*apvts.getParameter("gain")),
        };
    };
} // namespace jive_example
