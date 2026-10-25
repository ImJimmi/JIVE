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

        showMainComponent();
        state.addListener(this);
        applyTheme();
    }

    PluginEditor::~PluginEditor()
    {
        state.removeListener(this);
        hideMainComponent();
    }

    void PluginEditor::replaceDecoratedItem(std::unique_ptr<GuiItem> newItem)
    {
        hideMainComponent();

        item = std::move(newItem);
        state = item->state;
        component = item->getComponent();

        showMainComponent();
        applyTheme();
    }

    void PluginEditor::showMainComponent()
    {
        // The look-and-feel is set first, because the main component's style
        // sheet is created when it's added, and only for a JIVE look-and-feel.
        if (auto mainComponent = getComponent())
        {
            mainComponent->setLookAndFeel(&jiveLookAndFeel);
            mainComponent->addComponentListener(this);
            addAndMakeVisible(*mainComponent);
            setSize(mainComponent->getWidth(), mainComponent->getHeight());
        }
    }

    void PluginEditor::hideMainComponent()
    {
        if (auto mainComponent = getComponent())
        {
            mainComponent->setLookAndFeel(nullptr);
            mainComponent->removeComponentListener(this);
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

    void PluginEditor::valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property)
    {
        if (tree == state && property == ids::theme())
            applyTheme();
    }

    void PluginEditor::applyTheme()
    {
        jiveLookAndFeel.applyTheme(fromVar<Theme>(state.getProperty(ids::theme(), toVar(Theme::steel))));

        if (auto comp = getComponent())
            comp->sendLookAndFeelChange();
    }
} // namespace jive
#endif
