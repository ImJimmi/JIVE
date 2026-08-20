#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace jive_demo
{
    class PluginProcessor : public juce::AudioProcessor
    {
    public:
        PluginProcessor() = default;

        const juce::String getName() const final
        {
            return "JIVE Demo";
        }

        void prepareToPlay(double, int) final {}
        void releaseResources() final {}
        void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) final {}

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
            jassert(JIVE_IS_PLUGIN_PROJECT);

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

        void setCurrentProgram(int) final {}

        const juce::String getProgramName(int) final
        {
            return "";
        }

        void changeProgramName(int, const juce::String&) final {}
        void getStateInformation(juce::MemoryBlock&) final {}
        void setStateInformation(const void*, int) final {}

    private:
    };
} // namespace jive_demo
