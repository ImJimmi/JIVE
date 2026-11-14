#pragma once

#include "Parameters.h"

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>

namespace boabass
{
    [[nodiscard]] inline float load(const std::atomic<float>& value)
    {
        return value.load(std::memory_order_relaxed);
    }

    template <typename Enum>
    [[nodiscard]] Enum loadChoice(const std::atomic<float>& value)
    {
        return static_cast<Enum>(juce::roundToInt(load(value)));
    }

    class Oscillator
    {
    public:
        void reset(float startPhase = 0.0f)
        {
            phase = startPhase;
        }

        [[nodiscard]] float next(Wave wave, float increment)
        {
            const auto value = shape(wave, increment);
            phase += increment;

            if (phase >= 1.0f)
                phase -= 1.0f;

            return value;
        }

    private:
        // Rounds off the steps in the naive saw and square so they alias less.
        [[nodiscard]] static float polyBlep(float t, float increment)
        {
            if (t < increment)
            {
                t /= increment;
                return t + t - t * t - 1.0f;
            }

            if (t > 1.0f - increment)
            {
                t = (t - 1.0f) / increment;
                return t * t + t + t + 1.0f;
            }

            return 0.0f;
        }

        [[nodiscard]] float shape(Wave wave, float increment) const
        {
            switch (wave)
            {
            case Wave::sine:
                return std::sin(juce::MathConstants<float>::twoPi * phase);
            case Wave::triangle:
                return 1.0f - 4.0f * std::abs(phase - 0.5f);
            case Wave::saw:
                return 2.0f * phase - 1.0f - polyBlep(phase, increment);
            case Wave::square:
            {
                const auto halfPhase = std::fmod(phase + 0.5f, 1.0f);
                return (phase < 0.5f ? 1.0f : -1.0f) + polyBlep(phase, increment) - polyBlep(halfPhase, increment);
            }
            }

            return 0.0f;
        }

        float phase = 0.0f;
    };

    // State shared by every voice: the LFO, which is computed once per block,
    // and the last note played, which glides start from.
    struct SharedVoiceState
    {
        std::vector<float> lfo;
        float lastFrequency = 0.0f;
    };

    class Sound : public juce::SynthesiserSound
    {
    public:
        bool appliesToNote(int) final
        {
            return true;
        }

        bool appliesToChannel(int) final
        {
            return true;
        }
    };

    class Voice : public juce::SynthesiserVoice
    {
    public:
        Voice(const ParameterValues& parameterValues, SharedVoiceState& sharedState)
            : parameters{ parameterValues }
            , shared{ sharedState }
        {
        }

        bool canPlaySound(juce::SynthesiserSound* sound) final
        {
            return dynamic_cast<Sound*>(sound) != nullptr;
        }

        void prepare(double newSampleRate, int maxBlockSize)
        {
            setCurrentPlaybackSampleRate(newSampleRate);
            ampEnvelope.setSampleRate(newSampleRate);
            filterEnvelope.setSampleRate(newSampleRate);
            filter.prepare({ newSampleRate, static_cast<juce::uint32>(maxBlockSize), 1 });
        }

        void startNote(int midiNoteNumber, float newVelocity, juce::SynthesiserSound*, int) final
        {
            targetFrequency = static_cast<float>(juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber));
            noteNumber = midiNoteNumber;
            velocity = 0.4f + 0.6f * newVelocity;

            const auto gliding = load(parameters.glide) > 0.0f && shared.lastFrequency > 0.0f;
            frequency = gliding ? shared.lastFrequency : targetFrequency;
            shared.lastFrequency = targetFrequency;

            if (!ampEnvelope.isActive())
            {
                filter.reset();
                osc1.reset();
                osc2.reset(0.25f);
                sub.reset();
            }

            updateEnvelopes();
            ampEnvelope.noteOn();
            filterEnvelope.noteOn();
        }

        void stopNote(float, bool allowTailOff) final
        {
            if (allowTailOff)
            {
                ampEnvelope.noteOff();
                filterEnvelope.noteOff();
                return;
            }

            clearCurrentNote();
        }

        void pitchWheelMoved(int) final
        {
        }

        void controllerMoved(int, int) final
        {
        }

        void renderNextBlock(juce::AudioBuffer<float>& output, int startSample, int numSamples) final
        {
            // A stolen voice is cleared and restarted straight away, so its
            // envelopes carry on from where they were. Any other cleared
            // voice stops dead.
            if (!isVoiceActive() || !ampEnvelope.isActive())
            {
                ampEnvelope.reset();
                filterEnvelope.reset();

                if (isVoiceActive())
                    clearCurrentNote();

                return;
            }

            updateEnvelopes();
            updateFilterMode();

            const auto sampleRate = static_cast<float>(getSampleRate());
            const auto glideCoefficient = calculateGlideCoefficient(sampleRate);
            const auto osc1Wave = loadChoice<Wave>(parameters.osc1Wave);
            const auto osc2Wave = loadChoice<Wave>(parameters.osc2Wave);
            const auto subWave = loadChoice<int>(parameters.subWave) == 0 ? Wave::sine : Wave::square;
            const auto osc1Ratio = pitchRatio(parameters.osc1Octave, parameters.osc1Detune);
            const auto osc2Ratio = pitchRatio(parameters.osc2Octave, parameters.osc2Detune);
            const auto osc1Level = load(parameters.osc1Level);
            const auto osc2Level = load(parameters.osc2On) > 0.5f ? load(parameters.osc2Level) : 0.0f;
            const auto subOn = load(parameters.subOn) > 0.5f;
            const auto subLevel = subOn ? load(parameters.subLevel) : 0.0f;
            const auto noiseLevel = subOn ? load(parameters.noiseLevel) : 0.0f;
            const auto drive = 1.0f + 9.0f * load(parameters.filterDrive);
            const auto lfoTarget = loadChoice<LfoTarget>(parameters.lfoTarget);
            const auto lfoOn = load(parameters.lfoOn) > 0.5f;

            for (auto i = 0; i < numSamples; i++)
            {
                const auto sampleIndex = static_cast<std::size_t>(startSample + i);
                const auto lfo = lfoOn && sampleIndex < shared.lfo.size() ? shared.lfo[sampleIndex] : 0.0f;
                frequency += (targetFrequency - frequency) * glideCoefficient;

                const auto pitchLfo = lfoTarget == LfoTarget::pitch ? std::exp2(lfo / 12.0f) : 1.0f;
                const auto increment = juce::jmin(0.45f, frequency * pitchLfo / sampleRate);

                auto sample = osc1.next(osc1Wave, increment * osc1Ratio) * osc1Level
                            + osc2.next(osc2Wave, increment * osc2Ratio) * osc2Level
                            + sub.next(subWave, increment * 0.5f) * subLevel
                            + (random.nextFloat() * 2.0f - 1.0f) * noiseLevel;

                const auto filterAmount = filterEnvelope.getNextSample();

                if ((i & (controlInterval - 1)) == 0)
                    updateCutoff(filterAmount, lfoTarget == LfoTarget::cutoff ? lfo : 0.0f, sampleRate);

                sample = filter.processSample(0, std::tanh(sample * drive) / std::sqrt(drive));
                sample *= ampEnvelope.getNextSample() * velocity * 0.35f;

                if (lfoTarget == LfoTarget::amp)
                    sample *= 1.0f - 0.5f * (lfo + 1.0f) * load(parameters.lfoDepth);

                for (auto channel = 0; channel < output.getNumChannels(); channel++)
                    output.addSample(channel, startSample + i, sample);
            }

            if (!ampEnvelope.isActive())
                clearCurrentNote();
        }

    private:
        static constexpr auto controlInterval = 16;

        [[nodiscard]] static float pitchRatio(const std::atomic<float>& octave, const std::atomic<float>& cents)
        {
            return std::exp2(load(octave) + load(cents) / 1200.0f);
        }

        [[nodiscard]] float calculateGlideCoefficient(float sampleRate) const
        {
            const auto glideTime = load(parameters.glide);

            if (glideTime <= 0.0f)
                return 1.0f;

            return 1.0f - std::exp(-1.0f / (glideTime * 0.3f * sampleRate));
        }

        void updateEnvelopes()
        {
            ampEnvelope.setParameters({
                load(parameters.ampAttack),
                load(parameters.ampDecay),
                load(parameters.ampSustain),
                load(parameters.ampRelease),
            });
            filterEnvelope.setParameters({
                load(parameters.filterAttack),
                load(parameters.filterDecay),
                load(parameters.filterSustain),
                load(parameters.filterRelease),
            });
        }

        void updateFilterMode()
        {
            using Type = juce::dsp::StateVariableTPTFilterType;

            switch (loadChoice<FilterMode>(parameters.filterMode))
            {
            case FilterMode::lowPass:
                filter.setType(Type::lowpass);
                break;
            case FilterMode::bandPass:
                filter.setType(Type::bandpass);
                break;
            case FilterMode::highPass:
                filter.setType(Type::highpass);
                break;
            }

            filter.setResonance(0.5f + 9.5f * std::pow(load(parameters.filterResonance), 2.0f));
        }

        void updateCutoff(float envelope, float lfo, float sampleRate)
        {
            static constexpr auto envelopeRangeInOctaves = 5.0f;
            static constexpr auto lfoRangeInOctaves = 3.0f;
            static constexpr auto middleC = 60;

            const auto octavesFromMiddleC = static_cast<float>(noteNumber - middleC) / 12.0f;
            const auto octaves = envelopeRangeInOctaves * envelope * load(parameters.filterEnvAmount)
                               + load(parameters.filterKeyTrack) * octavesFromMiddleC
                               + lfoRangeInOctaves * lfo * load(parameters.lfoDepth);
            const auto cutoff = load(parameters.filterCutoff) * std::exp2(octaves);

            filter.setCutoffFrequency(juce::jlimit(20.0f, sampleRate * 0.45f, cutoff));
        }

        const ParameterValues& parameters;
        SharedVoiceState& shared;

        Oscillator osc1;
        Oscillator osc2;
        Oscillator sub;
        juce::Random random;
        juce::ADSR ampEnvelope;
        juce::ADSR filterEnvelope;
        juce::dsp::StateVariableTPTFilter<float> filter;

        float frequency = 0.0f;
        float targetFrequency = 0.0f;
        float velocity = 1.0f;
        int noteNumber = 60;
    };

    // Limits how many voices play at once, stealing the oldest one when a new
    // note needs it.
    class Synthesiser : public juce::Synthesiser
    {
    public:
        void setVoiceLimit(int newLimit)
        {
            voiceLimit = juce::jlimit(1, getNumVoices(), newLimit);

            for (auto i = voiceLimit; i < getNumVoices(); i++)
            {
                if (auto* voice = getVoice(i); voice->isVoiceActive())
                    voice->stopNote(0.0f, true);
            }
        }

    protected:
        juce::SynthesiserVoice* findFreeVoice(juce::SynthesiserSound* sound, int channel, int note, bool steal) const final
        {
            juce::SynthesiserVoice* oldest = nullptr;

            for (auto i = 0; i < voiceLimit; i++)
            {
                auto* voice = getVoice(i);

                if (!voice->isVoiceActive() && voice->canPlaySound(sound))
                    return voice;

                if (oldest == nullptr || voice->wasStartedBefore(*oldest))
                    oldest = voice;
            }

            juce::ignoreUnused(channel, note);
            return steal ? oldest : nullptr;
        }

    private:
        int voiceLimit = 1;
    };
} // namespace boabass
