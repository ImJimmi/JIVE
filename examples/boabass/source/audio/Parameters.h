#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace boabass
{
    enum class Wave
    {
        sine,
        triangle,
        saw,
        square,
    };

    enum class FilterMode
    {
        lowPass,
        bandPass,
        highPass,
    };

    enum class LfoTarget
    {
        cutoff,
        pitch,
        amp,
    };

    namespace parameters
    {
        inline const juce::StringArray waves{ "Sine", "Triangle", "Saw", "Square" };
        inline const juce::StringArray subWaves{ "Sine", "Square" };
        inline const juce::StringArray filterModes{ "Low-pass", "Band-pass", "High-pass" };
        inline const juce::StringArray lfoTargets{ "Cutoff", "Pitch", "Amp" };

        [[nodiscard]] inline juce::NormalisableRange<float> skewed(float start, float end, float centre)
        {
            juce::NormalisableRange<float> range{ start, end };
            range.setSkewForCentre(centre);
            return range;
        }

        // Widgets show parameters as text and read typed text back, so each
        // unit converts both ways.
        struct Unit
        {
            juce::String (*toText)(float value, int maximumLength);
            float (*fromText)(const juce::String& text);
        };

        inline const Unit seconds{
            [](float value, int) {
                if (value < 1.0f)
                    return juce::String{ juce::roundToInt(value * 1000.0f) } + " ms";

                return juce::String{ value, 2 } + " s";
            },
            [](const juce::String& text) {
                const auto value = text.getFloatValue();
                return text.containsIgnoreCase("ms") ? value / 1000.0f : value;
            },
        };

        inline const Unit hertz{
            [](float value, int) {
                if (value < 1000.0f)
                    return juce::String{ value, value < 10.0f ? 2 : 0 } + " Hz";

                return juce::String{ value / 1000.0f, 2 } + " kHz";
            },
            [](const juce::String& text) {
                const auto value = text.getFloatValue();
                return text.containsIgnoreCase("k") ? value * 1000.0f : value;
            },
        };

        inline const Unit percent{
            [](float value, int) {
                return juce::String{ juce::roundToInt(value * 100.0f) } + "%";
            },
            [](const juce::String& text) {
                return text.getFloatValue() / 100.0f;
            },
        };

        inline const Unit signedPercent{
            [](float value, int) {
                const auto rounded = juce::roundToInt(value * 100.0f);
                return (rounded > 0 ? "+" : "") + juce::String{ rounded } + "%";
            },
            percent.fromText,
        };

        inline const Unit cents{
            [](float value, int) {
                const auto rounded = juce::roundToInt(value);
                return (rounded > 0 ? "+" : "") + juce::String{ rounded } + " ct";
            },
            [](const juce::String& text) {
                return text.getFloatValue();
            },
        };

        inline const Unit decibels{
            [](float value, int) {
                return juce::String{ value, 1 } + " dB";
            },
            cents.fromText,
        };

        [[nodiscard]] inline juce::String formatOctave(int octave, int)
        {
            return (octave > 0 ? "+" : "") + juce::String{ octave } + " oct";
        }

        [[nodiscard]] inline std::unique_ptr<juce::AudioParameterFloat> makeFloat(const juce::String& id,
                                                                                  const juce::String& name,
                                                                                  juce::NormalisableRange<float> range,
                                                                                  float defaultValue,
                                                                                  const Unit& unit)
        {
            return std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ id, 1 },
                                                               name,
                                                               range,
                                                               defaultValue,
                                                               juce::AudioParameterFloatAttributes{}
                                                                   .withStringFromValueFunction(unit.toText)
                                                                   .withValueFromStringFunction(unit.fromText));
        }

        [[nodiscard]] inline std::unique_ptr<juce::AudioParameterInt> makeOctave(const juce::String& id, const juce::String& name, int defaultValue)
        {
            return std::make_unique<juce::AudioParameterInt>(juce::ParameterID{ id, 1 },
                                                             name,
                                                             -2,
                                                             1,
                                                             defaultValue,
                                                             juce::AudioParameterIntAttributes{}
                                                                 .withStringFromValueFunction(formatOctave));
        }

        [[nodiscard]] inline std::unique_ptr<juce::AudioParameterChoice> makeChoice(const juce::String& id,
                                                                                    const juce::String& name,
                                                                                    const juce::StringArray& choices,
                                                                                    int defaultIndex)
        {
            return std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{ id, 1 }, name, choices, defaultIndex);
        }

        [[nodiscard]] inline std::unique_ptr<juce::AudioParameterBool> makeSwitch(const juce::String& id, const juce::String& name, bool defaultValue)
        {
            return std::make_unique<juce::AudioParameterBool>(juce::ParameterID{ id, 1 }, name, defaultValue);
        }

        [[nodiscard]] inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
        {
            const auto envelopeTime = skewed(0.001f, 5.0f, 0.3f);

            juce::AudioProcessorValueTreeState::ParameterLayout layout;

            layout.add(makeChoice("osc1-wave", "Osc 1 Wave", waves, 2),
                       makeOctave("osc1-octave", "Osc 1 Octave", 0),
                       makeFloat("osc1-detune", "Osc 1 Detune", { -50.0f, 50.0f }, 0.0f, cents),
                       makeFloat("osc1-level", "Osc 1 Level", { 0.0f, 1.0f }, 0.8f, percent));

            layout.add(makeSwitch("osc2-on", "Osc 2 On", true),
                       makeChoice("osc2-wave", "Osc 2 Wave", waves, 3),
                       makeOctave("osc2-octave", "Osc 2 Octave", -1),
                       makeFloat("osc2-detune", "Osc 2 Detune", { -50.0f, 50.0f }, 7.0f, cents),
                       makeFloat("osc2-level", "Osc 2 Level", { 0.0f, 1.0f }, 0.5f, percent));

            layout.add(makeSwitch("sub-on", "Sub On", true),
                       makeChoice("sub-wave", "Sub Wave", subWaves, 0),
                       makeFloat("sub-level", "Sub Level", { 0.0f, 1.0f }, 0.6f, percent),
                       makeFloat("noise-level", "Noise Level", { 0.0f, 1.0f }, 0.0f, percent));

            layout.add(makeChoice("filter-mode", "Filter Mode", filterModes, 0),
                       makeFloat("filter-cutoff", "Cutoff", skewed(20.0f, 18000.0f, 800.0f), 650.0f, hertz),
                       makeFloat("filter-resonance", "Resonance", { 0.0f, 1.0f }, 0.35f, percent),
                       makeFloat("filter-drive", "Drive", { 0.0f, 1.0f }, 0.25f, percent),
                       makeFloat("filter-env-amount", "Env Amount", { -1.0f, 1.0f }, 0.45f, signedPercent),
                       makeFloat("filter-key-track", "Key Track", { 0.0f, 1.0f }, 0.5f, percent));

            layout.add(makeFloat("filter-attack", "Filter Attack", envelopeTime, 0.002f, seconds),
                       makeFloat("filter-decay", "Filter Decay", envelopeTime, 0.35f, seconds),
                       makeFloat("filter-sustain", "Filter Sustain", { 0.0f, 1.0f }, 0.2f, percent),
                       makeFloat("filter-release", "Filter Release", envelopeTime, 0.25f, seconds));

            layout.add(makeFloat("amp-attack", "Amp Attack", envelopeTime, 0.003f, seconds),
                       makeFloat("amp-decay", "Amp Decay", envelopeTime, 0.6f, seconds),
                       makeFloat("amp-sustain", "Amp Sustain", { 0.0f, 1.0f }, 0.8f, percent),
                       makeFloat("amp-release", "Amp Release", envelopeTime, 0.15f, seconds));

            layout.add(makeSwitch("lfo-on", "LFO On", false),
                       makeChoice("lfo-wave", "LFO Wave", waves, 0),
                       makeChoice("lfo-target", "LFO Target", lfoTargets, 0),
                       makeFloat("lfo-rate", "LFO Rate", skewed(0.05f, 20.0f, 2.0f), 2.0f, hertz),
                       makeFloat("lfo-depth", "LFO Depth", { 0.0f, 1.0f }, 0.4f, percent));

            layout.add(std::make_unique<juce::AudioParameterInt>(juce::ParameterID{ "voices", 1 }, "Voices", 1, 8, 4),
                       makeFloat("glide", "Glide", skewed(0.0f, 1.0f, 0.15f), 0.0f, seconds),
                       makeFloat("volume", "Volume", { -36.0f, 6.0f, 0.1f }, -6.0f, decibels));

            return layout;
        }
    } // namespace parameters

    // The audio thread's view of the parameters.
    struct ParameterValues
    {
        explicit ParameterValues(const juce::AudioProcessorValueTreeState& state)
            : osc1Wave{ *state.getRawParameterValue("osc1-wave") }
            , osc1Octave{ *state.getRawParameterValue("osc1-octave") }
            , osc1Detune{ *state.getRawParameterValue("osc1-detune") }
            , osc1Level{ *state.getRawParameterValue("osc1-level") }
            , osc2On{ *state.getRawParameterValue("osc2-on") }
            , osc2Wave{ *state.getRawParameterValue("osc2-wave") }
            , osc2Octave{ *state.getRawParameterValue("osc2-octave") }
            , osc2Detune{ *state.getRawParameterValue("osc2-detune") }
            , osc2Level{ *state.getRawParameterValue("osc2-level") }
            , subOn{ *state.getRawParameterValue("sub-on") }
            , subWave{ *state.getRawParameterValue("sub-wave") }
            , subLevel{ *state.getRawParameterValue("sub-level") }
            , noiseLevel{ *state.getRawParameterValue("noise-level") }
            , filterMode{ *state.getRawParameterValue("filter-mode") }
            , filterCutoff{ *state.getRawParameterValue("filter-cutoff") }
            , filterResonance{ *state.getRawParameterValue("filter-resonance") }
            , filterDrive{ *state.getRawParameterValue("filter-drive") }
            , filterEnvAmount{ *state.getRawParameterValue("filter-env-amount") }
            , filterKeyTrack{ *state.getRawParameterValue("filter-key-track") }
            , filterAttack{ *state.getRawParameterValue("filter-attack") }
            , filterDecay{ *state.getRawParameterValue("filter-decay") }
            , filterSustain{ *state.getRawParameterValue("filter-sustain") }
            , filterRelease{ *state.getRawParameterValue("filter-release") }
            , ampAttack{ *state.getRawParameterValue("amp-attack") }
            , ampDecay{ *state.getRawParameterValue("amp-decay") }
            , ampSustain{ *state.getRawParameterValue("amp-sustain") }
            , ampRelease{ *state.getRawParameterValue("amp-release") }
            , lfoOn{ *state.getRawParameterValue("lfo-on") }
            , lfoWave{ *state.getRawParameterValue("lfo-wave") }
            , lfoTarget{ *state.getRawParameterValue("lfo-target") }
            , lfoRate{ *state.getRawParameterValue("lfo-rate") }
            , lfoDepth{ *state.getRawParameterValue("lfo-depth") }
            , voices{ *state.getRawParameterValue("voices") }
            , glide{ *state.getRawParameterValue("glide") }
            , volume{ *state.getRawParameterValue("volume") }
        {
        }

        const std::atomic<float>& osc1Wave;
        const std::atomic<float>& osc1Octave;
        const std::atomic<float>& osc1Detune;
        const std::atomic<float>& osc1Level;
        const std::atomic<float>& osc2On;
        const std::atomic<float>& osc2Wave;
        const std::atomic<float>& osc2Octave;
        const std::atomic<float>& osc2Detune;
        const std::atomic<float>& osc2Level;
        const std::atomic<float>& subOn;
        const std::atomic<float>& subWave;
        const std::atomic<float>& subLevel;
        const std::atomic<float>& noiseLevel;
        const std::atomic<float>& filterMode;
        const std::atomic<float>& filterCutoff;
        const std::atomic<float>& filterResonance;
        const std::atomic<float>& filterDrive;
        const std::atomic<float>& filterEnvAmount;
        const std::atomic<float>& filterKeyTrack;
        const std::atomic<float>& filterAttack;
        const std::atomic<float>& filterDecay;
        const std::atomic<float>& filterSustain;
        const std::atomic<float>& filterRelease;
        const std::atomic<float>& ampAttack;
        const std::atomic<float>& ampDecay;
        const std::atomic<float>& ampSustain;
        const std::atomic<float>& ampRelease;
        const std::atomic<float>& lfoOn;
        const std::atomic<float>& lfoWave;
        const std::atomic<float>& lfoTarget;
        const std::atomic<float>& lfoRate;
        const std::atomic<float>& lfoDepth;
        const std::atomic<float>& voices;
        const std::atomic<float>& glide;
        const std::atomic<float>& volume;
    };
} // namespace boabass
