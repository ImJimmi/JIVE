#include "PluginProcessor.h"

namespace boabass
{
    // Plays the synth through the default audio device, from any MIDI input
    // as well as the on-screen keyboard.
    class Application : public juce::JUCEApplication
    {
    public:
        const juce::String getApplicationName() final
        {
            return JUCE_APPLICATION_NAME;
        }

        const juce::String getApplicationVersion() final
        {
            return JUCE_APPLICATION_VERSION;
        }

        void initialise(const juce::String&) final
        {
            processor = std::make_unique<Processor>();
            deviceManager.initialiseWithDefaultDevices(0, 2);

            for (const auto& input : juce::MidiInput::getAvailableDevices())
                deviceManager.setMidiInputDeviceEnabled(input.identifier, true);

            deviceManager.addMidiInputDeviceCallback({}, &player);
            deviceManager.addAudioCallback(&player);
            player.setProcessor(processor.get());

            window = processor->createWindow();

            if (auto* documentWindow = dynamic_cast<jive::DocumentWindow*>(window.get()))
                documentWindow->onCloseButtonPressed = [] {
                    quit();
                };
        }

        void shutdown() final
        {
            window = nullptr;
            player.setProcessor(nullptr);
            deviceManager.removeAudioCallback(&player);
            deviceManager.removeMidiInputDeviceCallback({}, &player);
            processor = nullptr;
        }

    private:
        juce::AudioDeviceManager deviceManager;
        juce::AudioProcessorPlayer player;
        std::unique_ptr<Processor> processor;
        std::unique_ptr<jive::GuiItem> window;
    };
} // namespace boabass

START_JUCE_APPLICATION(boabass::Application)
