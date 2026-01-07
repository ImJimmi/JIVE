#pragma once

#include "jive_TopLevelGuiItem.h"

#include <juce_audio_processors/juce_audio_processors.h>

namespace jive
{
    class PluginEditor final
        : public jive::TopLevelGuiItem
        , public juce::AudioProcessorEditor
        , private juce::ComponentListener
    {
    public:
        PluginEditor(std::unique_ptr<GuiItem> itemToDecorate,
                     juce::AudioProcessor* pluginProcessor = nullptr);
        ~PluginEditor() override;

        void replaceDecoratedItem(std::unique_ptr<GuiItem> newItem) override;
        void resized() override;

    private:
        void componentMovedOrResized(juce::Component&, bool, bool) final;

        LookAndFeel jiveLookAndFeel;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginEditor)
    };
} // namespace jive
