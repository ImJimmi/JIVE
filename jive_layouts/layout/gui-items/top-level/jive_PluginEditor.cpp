#if JIVE_IS_PLUGIN_PROJECT
    #include "jive_PluginEditor.h"

namespace jive
{
    PluginEditor::PluginEditor(std::unique_ptr<GuiItem> itemToDecorate,
                               juce::AudioProcessor* pluginProcessor)
        : TopLevelGuiItem{ std::move(itemToDecorate) }
        , juce::AudioProcessorEditor{ *pluginProcessor }
    {
        jassert(pluginProcessor != nullptr);

        if (auto mainComponent = getComponent())
        {
            mainComponent->addComponentListener(this);
            addAndMakeVisible(*mainComponent);
            setSize(mainComponent->getWidth(), mainComponent->getHeight());
            mainComponent->setLookAndFeel(&jiveLookAndFeel);
        }

        state.addListener(this);

        if (state.hasProperty(ids::theme()))
            applyTheme();
    }

    PluginEditor::~PluginEditor()
    {
        state.removeListener(this);

        if (auto comp = getComponent())
        {
            comp->setLookAndFeel(nullptr);
            comp->removeComponentListener(this);
        }
    }

    void PluginEditor::replaceDecoratedItem(std::unique_ptr<GuiItem> newItem)
    {
        item = std::move(newItem);
        state = item->state;
        applyTheme();
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

    void PluginEditor::valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property)
    {
        if (tree == state && property == ids::theme())
            applyTheme();
    }

    // Editors have no theme unless they're given one.
    void PluginEditor::applyTheme()
    {
        jiveLookAndFeel.applyTheme(fromVar<Theme>(state.getProperty(ids::theme(), toVar(Theme::juce))));

        if (auto comp = getComponent())
            comp->sendLookAndFeelChange();
    }
} // namespace jive
#endif
