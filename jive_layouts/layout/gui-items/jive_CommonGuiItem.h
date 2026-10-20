#pragma once

#include <jive_layouts/layout/gui-items/jive_GuiItemDecorator.h>
#include <jive_layouts/utilities/jive_Display.h>

namespace jive
{
    class CommonGuiItem
        : public GuiItemDecorator
        , private juce::ComponentListener
        , private BoxModel::Listener
        , private juce::ValueTree::Listener
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
        void componentParentHierarchyChanged(juce::Component& component) override;
        void boxModelChanged(BoxModel& boxModel) override;

        void childrenChanged() override;
        void localisedStringsChanged() override;

    private:
        void valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property) final;

        void updateProperty(juce::Identifier property);
        void updateLocalisedStrings();

        PassiveProperty<juce::String> name;
        PassiveProperty<juce::String> title;
        PassiveProperty<juce::Identifier> id;
        PassiveProperty<juce::String> description;
        PassiveProperty<juce::String> tooltip;
        PassiveProperty<juce::String> lang;
        PassiveProperty<bool> enabled;
        PassiveProperty<bool> accessible;
        PassiveProperty<bool> visibility;
        PassiveProperty<bool> alwaysOnTop;
        PassiveProperty<bool> bufferedToImage;
        PassiveProperty<bool> opaque;
        PassiveProperty<bool> focusable;
        PassiveProperty<bool> clickingGrabsFocus;
        PassiveProperty<bool> focusOutline;
        PassiveProperty<int> focusOrder;
        PassiveProperty<float> opacity;
        PassiveProperty<juce::String> classes;
        PassiveProperty<juce::MouseCursor::StandardCursorType> cursor;
        PassiveProperty<Length> width;
        PassiveProperty<Length> height;

        std::unique_ptr<FileObserver> stringsFileObserver;

        // What the style sheet was last created for. The look-and-feel is only
        // compared, as JUCE asserts if one is deleted while weakly referenced.
        juce::Component::SafePointer<juce::Component> styledParent;
        const juce::LookAndFeel* styledLookAndFeel = nullptr;

        const std::vector<std::pair<juce::Identifier, std::function<void()>>> propertyUpdaters;

        JUCE_LEAK_DETECTOR(CommonGuiItem)
    };
} // namespace jive
