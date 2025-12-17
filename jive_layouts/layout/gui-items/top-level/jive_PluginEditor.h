#pragma once

#include <jive_layouts/layout/gui-items/jive_GuiItemDecorator.h>

#include <juce_audio_processors/juce_audio_processors.h>

namespace jive
{
    class PluginEditor final
        : public jive::GuiItemDecorator
        , public juce::AudioProcessorEditor
        , private juce::ComponentListener
    {
    public:
        PluginEditor(std::unique_ptr<GuiItem> itemToDecorate,
                     juce::AudioProcessor* pluginProcessor = nullptr);
        ~PluginEditor() final;

        void resized() final;

    private:
        void componentMovedOrResized(juce::Component&, bool, bool) final;

        LookAndFeel jiveLookAndFeel;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginEditor)
    };
} // namespace jive
