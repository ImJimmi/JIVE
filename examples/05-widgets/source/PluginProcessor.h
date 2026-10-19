#pragma once

#include "Presenter.h"

namespace jive_examples::widgets
{
    class Processor : public juce::AudioProcessor
    {
    public:
        Processor()
            : juce::AudioProcessor{
                BusesProperties{}
                    .withInput("Input", juce::AudioChannelSet::stereo(), true)
                    .withOutput("Output", juce::AudioChannelSet::stereo(), true),
            }
        {
        }

        const juce::String getName() const final
        {
            return JucePlugin_Name;
        }

        bool isBusesLayoutSupported(const BusesLayout& layouts) const final
        {
            const auto output = layouts.getMainOutputChannelSet();

            return output == layouts.getMainInputChannelSet()
                && (output == juce::AudioChannelSet::mono() || output == juce::AudioChannelSet::stereo());
        }

        void prepareToPlay(double, int) final
        {
            previousGains = calculateGains(getTotalNumOutputChannels());
        }

        void releaseResources() final
        {
        }

        void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) final
        {
            const juce::ScopedNoDenormals noDenormals;
            const auto numChannels = juce::jmin(buffer.getNumChannels(), 2);

            if (numChannels == 2)
                routeChannels(buffer);

            const auto gains = calculateGains(numChannels);

            for (auto channel = 0; channel < numChannels; channel++)
            {
                const auto index = static_cast<std::size_t>(channel);
                buffer.applyGainRamp(channel, 0, buffer.getNumSamples(), previousGains[index], gains[index]);
                previousGains[index] = gains[index];
            }
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
            if (presenter == nullptr)
                presenter = std::make_unique<Presenter>(*this, parameters, undoManager);

            return presenter->createEditor();
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
            return {};
        }

        void changeProgramName(int, const juce::String&) final
        {
        }

        void getStateInformation(juce::MemoryBlock& data) final
        {
            if (const auto xml = parameters.copyState().createXml())
                copyXmlToBinary(*xml, data);
        }

        void setStateInformation(const void* data, int size) final
        {
            if (const auto xml = getXmlFromBinary(data, size);
                xml != nullptr && xml->hasTagName(parameters.state.getType()))
            {
                parameters.replaceState(juce::ValueTree::fromXml(*xml));
            }
        }

    private:
        enum class Channels
        {
            stereo,
            swapped,
            mono,
        };

        [[nodiscard]] static juce::AudioProcessorValueTreeState::ParameterLayout createParameters()
        {
            return {
                std::make_unique<juce::AudioParameterFloat>(
                    juce::ParameterID{ "gain", 1 },
                    "Gain",
                    juce::NormalisableRange<float>{ -24.0f, 12.0f, 0.1f },
                    0.0f,
                    juce::AudioParameterFloatAttributes{}
                        .withLabel("dB")
                        .withStringFromValueFunction([](float value, int) {
                            return juce::String{ value, 1 } + " dB";
                        })
                        .withValueFromStringFunction([](const juce::String& text) {
                            return text.getFloatValue();
                        })),
                std::make_unique<juce::AudioParameterFloat>(
                    juce::ParameterID{ "pan", 1 },
                    "Pan",
                    juce::NormalisableRange<float>{ -1.0f, 1.0f, 0.01f },
                    0.0f,
                    juce::AudioParameterFloatAttributes{}
                        .withStringFromValueFunction([](float value, int) {
                            const auto percentage = juce::roundToInt(std::abs(value) * 100.0f);

                            if (percentage == 0)
                                return juce::String{ "Centre" };

                            return juce::String{ value < 0.0f ? "L " : "R " } + juce::String{ percentage };
                        })
                        .withValueFromStringFunction([](const juce::String& text) {
                            const auto value = text.retainCharacters("0123456789.").getFloatValue() / 100.0f;
                            return text.startsWithIgnoreCase("L") ? -value : value;
                        })),
                std::make_unique<juce::AudioParameterChoice>(
                    juce::ParameterID{ "channels", 1 },
                    "Channels",
                    juce::StringArray{ "Stereo", "Swapped", "Mono" },
                    0),
                std::make_unique<juce::AudioParameterBool>(
                    juce::ParameterID{ "mute", 1 },
                    "Mute",
                    false),
            };
        }

        void routeChannels(juce::AudioBuffer<float>& buffer) const
        {
            const auto numSamples = buffer.getNumSamples();

            switch (static_cast<Channels>(channels.getIndex()))
            {
            case Channels::stereo:
                break;
            case Channels::swapped:
                std::swap_ranges(buffer.getWritePointer(0),
                                 buffer.getWritePointer(0) + numSamples,
                                 buffer.getWritePointer(1));
                break;
            case Channels::mono:
                buffer.addFrom(0, 0, buffer, 1, 0, numSamples);
                buffer.applyGain(0, 0, numSamples, 0.5f);
                buffer.copyFrom(1, 0, buffer, 0, 0, numSamples);
                break;
            }
        }

        // Pan only turns one side down, so the centre stays at full level.
        [[nodiscard]] std::array<float, 2> calculateGains(int numChannels) const
        {
            const auto level = mute.get() ? 0.0f : juce::Decibels::decibelsToGain(gain.get());

            if (numChannels < 2)
                return { level, level };

            return {
                level * juce::jmin(1.0f, 1.0f - pan.get()),
                level * juce::jmin(1.0f, 1.0f + pan.get()),
            };
        }

        juce::UndoManager undoManager;
        juce::AudioProcessorValueTreeState parameters{ *this, &undoManager, "Parameters", createParameters() };
        juce::AudioParameterFloat& gain{ dynamic_cast<juce::AudioParameterFloat&>(*parameters.getParameter("gain")) };
        juce::AudioParameterFloat& pan{ dynamic_cast<juce::AudioParameterFloat&>(*parameters.getParameter("pan")) };
        juce::AudioParameterChoice& channels{ dynamic_cast<juce::AudioParameterChoice&>(*parameters.getParameter("channels")) };
        juce::AudioParameterBool& mute{ dynamic_cast<juce::AudioParameterBool&>(*parameters.getParameter("mute")) };
        std::array<float, 2> previousGains{ 1.0f, 1.0f };

        std::unique_ptr<Presenter> presenter;
    };
} // namespace jive_examples::widgets
