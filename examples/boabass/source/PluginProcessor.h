#pragma once

#include "Presenter.h"
#include "audio/Presets.h"
#include "audio/Voice.h"

namespace boabass
{
    class Processor : public juce::AudioProcessor
    {
    public:
        Processor()
            : juce::AudioProcessor{
                BusesProperties{}.withOutput("Output", juce::AudioChannelSet::stereo(), true),
            }
        {
            for (auto i = 0; i < maxVoices; i++)
                synth.addVoice(new Voice{ values, sharedVoiceState });

            synth.addSound(new Sound);
        }

        const juce::String getName() const final
        {
            return "BoaBass";
        }

        bool isBusesLayoutSupported(const BusesLayout& layouts) const final
        {
            const auto output = layouts.getMainOutputChannelSet();
            return output == juce::AudioChannelSet::mono() || output == juce::AudioChannelSet::stereo();
        }

        void prepareToPlay(double sampleRate, int maxBlockSize) final
        {
            synth.setCurrentPlaybackSampleRate(sampleRate);

            for (auto i = 0; i < synth.getNumVoices(); i++)
            {
                if (auto* voice = dynamic_cast<Voice*>(synth.getVoice(i)))
                    voice->prepare(sampleRate, maxBlockSize);
            }

            sharedVoiceState.lfo.resize(static_cast<std::size_t>(maxBlockSize));
            gain.reset(sampleRate, 0.05);
            gain.setCurrentAndTargetValue(juce::Decibels::decibelsToGain(load(values.volume)));
            keyboardState.reset();
        }

        void releaseResources() final
        {
        }

        void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi) final
        {
            const juce::ScopedNoDenormals noDenormals;
            const auto numSamples = buffer.getNumSamples();

            buffer.clear();
            keyboardState.processNextMidiBuffer(midi, 0, numSamples, true);
            synth.setVoiceLimit(juce::roundToInt(load(values.voices)));
            renderLfo(numSamples);
            synth.renderNextBlock(buffer, midi, 0, numSamples);

            gain.setTargetValue(juce::Decibels::decibelsToGain(load(values.volume)));
            gain.applyGain(buffer, numSamples);

            level.store(buffer.getMagnitude(0, numSamples), std::memory_order_relaxed);
        }

        double getTailLengthSeconds() const final
        {
            return 0.0;
        }

        bool acceptsMidi() const final
        {
            return true;
        }

        bool producesMidi() const final
        {
            return false;
        }

        juce::AudioProcessorEditor* createEditor() final
        {
            return getPresenter().createEditor();
        }

        [[nodiscard]] std::unique_ptr<jive::GuiItem> createWindow()
        {
            return getPresenter().createWindow();
        }

        bool hasEditor() const final
        {
            return true;
        }

        int getNumPrograms() final
        {
            return static_cast<int>(getPresets().size());
        }

        int getCurrentProgram() final
        {
            return currentProgram;
        }

        void setCurrentProgram(int index) final
        {
            if (!juce::isPositiveAndBelow(index, getNumPrograms()))
                return;

            currentProgram = index;
            applyPreset(getPresets()[static_cast<std::size_t>(index)], parameters);
        }

        const juce::String getProgramName(int index) final
        {
            if (!juce::isPositiveAndBelow(index, getNumPrograms()))
                return {};

            return getPresets()[static_cast<std::size_t>(index)].name;
        }

        void changeProgramName(int, const juce::String&) final
        {
        }

        void getStateInformation(juce::MemoryBlock& data) final
        {
            auto state = parameters.copyState();
            state.setProperty("program", currentProgram, nullptr);

            if (const auto xml = state.createXml())
                copyXmlToBinary(*xml, data);
        }

        void setStateInformation(const void* data, int size) final
        {
            if (const auto xml = getXmlFromBinary(data, size);
                xml != nullptr && xml->hasTagName(parameters.state.getType()))
            {
                const auto state = juce::ValueTree::fromXml(*xml);
                currentProgram = state.getProperty("program", 0);
                parameters.replaceState(state);

                // The value tree state skips any parameter whose stored value
                // matches its own, but a switch at 0.8 matches a stored "on".
                for (const auto& child : state)
                {
                    if (auto* parameter = parameters.getParameter(child["id"].toString()))
                        parameter->setValueNotifyingHost(parameter->convertTo0to1(child["value"]));
                }
            }
        }

    private:
        static constexpr auto maxVoices = 8;

        Presenter& getPresenter()
        {
            if (presenter == nullptr)
                presenter = std::make_unique<Presenter>(*this, parameters, keyboardState, level);

            return *presenter;
        }

        void renderLfo(int numSamples)
        {
            const auto wave = loadChoice<Wave>(values.lfoWave);
            const auto increment = load(values.lfoRate) / static_cast<float>(getSampleRate());

            for (std::size_t i = 0; i < static_cast<std::size_t>(numSamples) && i < sharedVoiceState.lfo.size(); i++)
                sharedVoiceState.lfo[i] = lfo.next(wave, increment);
        }

        juce::UndoManager undoManager;
        juce::AudioProcessorValueTreeState parameters{ *this, &undoManager, "Parameters", boabass::parameters::createLayout() };
        const ParameterValues values{ parameters };

        juce::MidiKeyboardState keyboardState;
        SharedVoiceState sharedVoiceState;
        Synthesiser synth;
        Oscillator lfo;
        juce::SmoothedValue<float, juce::ValueSmoothingTypes::Multiplicative> gain{ 1.0f };
        std::atomic<float> level{ 0.0f };
        int currentProgram = 0;

        std::unique_ptr<Presenter> presenter;
    };
} // namespace boabass
