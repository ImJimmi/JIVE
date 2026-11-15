#pragma once

#include <jive_layouts/layout/gui-items/jive_GuiItemDecorator.h>

namespace jive
{
    class Viewport
        : public GuiItemDecorator
        , private juce::ComponentListener
        , private juce::ValueTree::Listener
    {
    public:
        explicit Viewport(std::unique_ptr<GuiItem> itemToDecorate);
        ~Viewport() override;

        [[nodiscard]] juce::Viewport& getViewport();
        [[nodiscard]] const juce::Viewport& getViewport() const;

    private:
        void componentParentHierarchyChanged(juce::Component& changedComponent) final;
        void componentMovedOrResized(juce::Component& changedComponent, bool wasMoved, bool wasResized) final;
        void componentVisibilityChanged(juce::Component& changedComponent) final;
        void valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& id) final;

        [[nodiscard]] bool isInViewport() const;
        void takePlaceOfContent();
        void fitContent();

        juce::Viewport viewport;
        juce::Component scrolledArea;
        bool fittingContent = false;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Viewport)
    };
} // namespace jive
