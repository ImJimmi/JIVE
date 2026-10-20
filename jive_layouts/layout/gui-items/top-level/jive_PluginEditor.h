#pragma once

#include "jive_TopLevelGuiItem.h"

#include <juce_audio_processors/juce_audio_processors.h>

namespace jive
{
    class PluginEditor final
        : public jive::TopLevelGuiItem
        , public juce::AudioProcessorEditor
        , private juce::ComponentListener
        , private juce::ValueTree::Listener
    {
    public:
        explicit PluginEditor(std::unique_ptr<GuiItem> itemToDecorate,
                              juce::AudioProcessor* pluginProcessor = nullptr);
        ~PluginEditor() override;

        void replaceDecoratedItem(std::unique_ptr<GuiItem> newItem) override;
        void resized() override;

    private:
        void componentMovedOrResized(juce::Component&, bool, bool) final;
        void valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property) final;
        void showMainComponent();
        void hideMainComponent();
        void applyTheme();

        // Attached to the editor itself, which outlives every main component a
        // reload swaps in, so that transitions keep being repainted.
        LookAndFeel jiveLookAndFeel{ *this };

        // The host shows the editor's window, so this paints the editor when
        // whichever window it's in is about to be shown.
        PaintBeforeShowing paintBeforeShowing{ *this };

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginEditor)
    };
} // namespace jive
