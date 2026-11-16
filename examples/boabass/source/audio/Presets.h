#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace boabass
{
    // Each preset only lists the parameters that differ from their defaults.
    struct Preset
    {
        juce::String name;
        std::vector<std::pair<juce::String, float>> values;
    };

    inline const std::vector<Preset>& getPresets()
    {
        static const std::vector<Preset> presets{
            { "Init", {} },
            {
                "Viper Pluck",
                {
                    { "osc2-on", 0.0f },
                    { "filter-cutoff", 300.0f },
                    { "filter-resonance", 0.55f },
                    { "filter-env-amount", 0.7f },
                    { "filter-decay", 0.18f },
                    { "filter-sustain", 0.0f },
                    { "amp-decay", 0.4f },
                    { "amp-sustain", 0.0f },
                    { "amp-release", 0.12f },
                },
            },
            {
                "Python Sub",
                {
                    { "osc1-wave", 0.0f },
                    { "osc1-level", 0.5f },
                    { "osc2-on", 0.0f },
                    { "sub-level", 1.0f },
                    { "filter-cutoff", 1200.0f },
                    { "filter-resonance", 0.1f },
                    { "filter-env-amount", 0.1f },
                    { "filter-drive", 0.4f },
                    { "voices", 1.0f },
                    { "glide", 0.08f },
                },
            },
            {
                "Mamba Reese",
                {
                    { "osc1-wave", 2.0f },
                    { "osc1-detune", -14.0f },
                    { "osc2-wave", 2.0f },
                    { "osc2-octave", 0.0f },
                    { "osc2-detune", 14.0f },
                    { "osc2-level", 0.8f },
                    { "filter-cutoff", 900.0f },
                    { "filter-drive", 0.7f },
                    { "filter-env-amount", 0.2f },
                    { "lfo-on", 1.0f },
                    { "lfo-rate", 0.3f },
                    { "lfo-depth", 0.25f },
                    { "voices", 2.0f },
                },
            },
            {
                "Rattler Wobble",
                {
                    { "osc1-wave", 3.0f },
                    { "osc2-wave", 2.0f },
                    { "filter-cutoff", 250.0f },
                    { "filter-resonance", 0.7f },
                    { "filter-env-amount", 0.0f },
                    { "filter-drive", 0.6f },
                    { "lfo-on", 1.0f },
                    { "lfo-wave", 1.0f },
                    { "lfo-rate", 4.0f },
                    { "lfo-depth", 0.8f },
                    { "voices", 1.0f },
                    { "glide", 0.05f },
                },
            },
            {
                "Cobra Acid",
                {
                    { "osc1-wave", 2.0f },
                    { "osc2-on", 0.0f },
                    { "sub-on", 0.0f },
                    { "filter-cutoff", 180.0f },
                    { "filter-resonance", 0.85f },
                    { "filter-env-amount", 0.8f },
                    { "filter-decay", 0.25f },
                    { "filter-sustain", 0.05f },
                    { "filter-drive", 0.5f },
                    { "amp-sustain", 1.0f },
                    { "voices", 1.0f },
                    { "glide", 0.06f },
                },
            },
        };

        return presets;
    }

    inline void applyPreset(const Preset& preset, juce::AudioProcessorValueTreeState& state)
    {
        for (auto* parameter : state.processor.getParameters())
        {
            auto* ranged = dynamic_cast<juce::RangedAudioParameter*>(parameter);

            if (ranged == nullptr)
                continue;

            const auto match = std::find_if(preset.values.begin(),
                                            preset.values.end(),
                                            [ranged](const auto& value) {
                                                return value.first == ranged->getParameterID();
                                            });
            const auto value = match == preset.values.end()
                                 ? ranged->getDefaultValue()
                                 : ranged->convertTo0to1(match->second);

            ranged->beginChangeGesture();
            ranged->setValueNotifyingHost(value);
            ranged->endChangeGesture();
        }
    }
} // namespace boabass
