#if JIVE_IS_PLUGIN_PROJECT
    #include "jive_PluginEditor.h"

namespace jive
{
    PluginEditor::PluginEditor(std::unique_ptr<GuiItem> itemToDecorate,
                               juce::AudioProcessor* pluginProcessor)
        : GuiItemDecorator{ std::move(itemToDecorate) }
        , juce::AudioProcessorEditor{ *pluginProcessor }
    {
        jassert(pluginProcessor != nullptr);

        if (auto mainComponent = getComponent())
        {
            mainComponent->addComponentListener(this);
            addAndMakeVisible(*mainComponent);
            setSize(mainComponent->getWidth(), mainComponent->getHeight());
            mainComponent->setLookAndFeel(&lookAndFeel);
        }
    }

    PluginEditor::~PluginEditor()
    {
        if (auto comp = getComponent())
        {
            comp->setLookAndFeel(nullptr);
            comp->removeComponentListener(this);
        }
    }

    void PluginEditor::resized()
    {
        if (auto comp = getComponent())
            comp->setBounds(getLocalBounds());
    }

    void PluginEditor::componentMovedOrResized(juce::Component& componentThatMovedOrResized,
                                               bool,
                                               bool wasResized)
    {
        jassertquiet(&componentThatMovedOrResized == getComponent().get());

        if (wasResized)
            setSize(getComponent()->getWidth(), getComponent()->getHeight());
    }
} // namespace jive
#endif
