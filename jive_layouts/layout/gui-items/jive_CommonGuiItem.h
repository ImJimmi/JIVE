#pragma once

namespace jive
{
    class CommonGuiItem
        : public GuiItemDecorator
        , private juce::ComponentListener
        , private BoxModel::Listener
    {
    public:
        explicit CommonGuiItem(std::unique_ptr<GuiItem> itemToDecorate);
        ~CommonGuiItem() override;

        BoxModel boxModel;

    protected:
        void componentMovedOrResized(juce::Component& component, bool wasMoved, bool wasResized) override;
        void componentVisibilityChanged(juce::Component& component) override;
        void componentNameChanged(juce::Component& component) override;
        void componentEnablementChanged(juce::Component& component) override;
        void componentChildrenChanged(juce::Component& component) override;
        void componentParentHierarchyChanged(juce::Component& component) override;
        void boxModelChanged(BoxModel& boxModel) override;

    private:
        Property<juce::String> name;
        Property<juce::String> title;
        Property<juce::Identifier> id;
        Property<juce::String> description;
        Property<juce::String> tooltip;
        Property<bool> enabled;
        Property<bool> accessible;
        Property<bool> visibility;
        Property<bool> alwaysOnTop;
        Property<bool> bufferedToImage;
        Property<bool> opaque;
        Property<bool> focusable;
        Property<bool> clickingGrabsFocus;
        Property<bool> focusOutline;
        Property<int> focusOrder;
        Property<float> opacity;
        Property<juce::MouseCursor::StandardCursorType> cursor;
        Property<Display> display;
        Length width;
        Length height;

        JUCE_LEAK_DETECTOR(CommonGuiItem)
    };
} // namespace jive
